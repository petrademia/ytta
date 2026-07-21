use std::fs::File;
use std::io::{BufRead, BufReader};
use std::path::Path;

use super::types::{Side, Tick};

fn json_string_field(line: &str, key: &str) -> Option<String> {
    let needle = format!("\"{key}\":\"");
    let start = line.find(&needle)? + needle.len();
    let end = line[start..].find('"')? + start;
    Some(line[start..end].to_string())
}

fn json_int_field(line: &str, key: &str) -> Option<i64> {
    let needle = format!("\"{key}\":");
    let start = line.find(&needle)? + needle.len();
    let rest = line[start..].trim_start();
    let end = rest
        .find(|c: char| !(c.is_ascii_digit() || c == '-' || c == '+'))
        .unwrap_or(rest.len());
    rest[..end].parse().ok()
}

pub fn parse_tick_line(line: &str) -> Option<Result<Tick, String>> {
    let line = line.trim();
    if line.is_empty() || line.starts_with('#') {
        return None;
    }
    let ty = json_string_field(line, "type")?;
    if ty != "tick" {
        return None;
    }
    Some((|| {
        let ts_ns = json_int_field(line, "ts_ns").ok_or("missing ts_ns")?;
        let price = json_int_field(line, "price").ok_or("missing price")?;
        let qty = json_int_field(line, "qty").ok_or("missing qty")?;
        let seq = json_int_field(line, "seq").ok_or("missing seq")? as u64;
        let instrument = json_string_field(line, "instrument").ok_or("missing instrument")?;
        let side_s = json_string_field(line, "side").ok_or("missing side")?;
        let side = match side_s.as_str() {
            "BUY" => Side::Buy,
            "SELL" => Side::Sell,
            other => return Err(format!("invalid side {other}")),
        };
        Ok(Tick {
            ts_ns,
            instrument,
            side,
            price,
            qty,
            seq,
        })
    })())
}

pub fn load_ticks(path: impl AsRef<Path>) -> Result<Vec<Tick>, String> {
    let file = File::open(path.as_ref()).map_err(|e| e.to_string())?;
    let reader = BufReader::new(file);
    let mut ticks = Vec::new();
    for (i, line) in reader.lines().enumerate() {
        let line = line.map_err(|e| e.to_string())?;
        if let Some(parsed) = parse_tick_line(&line) {
            ticks.push(parsed.map_err(|e| format!("line {}: {e}", i + 1))?);
        }
    }
    Ok(ticks)
}
