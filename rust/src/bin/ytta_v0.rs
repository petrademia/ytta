use std::env;
use std::fs::File;
use std::io::Write;
use std::process;

use ytta_rust::v0::{load_ticks, run_pipeline_mode, LatencyProbe, PipelineMode};

fn usage(argv0: &str) {
    eprintln!("Usage: {argv0} --fixture <ticks.ndjson> --out <events.ndjson> [--mode=sync|queued]");
}

fn parse_mode(s: &str) -> Result<PipelineMode, String> {
    match s.to_ascii_lowercase().as_str() {
        "sync" => Ok(PipelineMode::Sync),
        "queued" => Ok(PipelineMode::Queued),
        other => Err(format!("invalid --mode {other} (use sync|queued)")),
    }
}

fn main() {
    let mut fixture = None;
    let mut out_path = None;
    let mut mode = PipelineMode::Sync;
    let mut args = env::args().skip(1);
    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--fixture" => fixture = args.next(),
            "--out" => out_path = args.next(),
            "--mode" => {
                let Some(v) = args.next() else {
                    usage("ytta_v0");
                    process::exit(1);
                };
                match parse_mode(&v) {
                    Ok(m) => mode = m,
                    Err(e) => {
                        eprintln!("error: {e}");
                        process::exit(1);
                    }
                }
            }
            a if a.starts_with("--mode=") => match parse_mode(&a[7..]) {
                Ok(m) => mode = m,
                Err(e) => {
                    eprintln!("error: {e}");
                    process::exit(1);
                }
            },
            "--help" | "-h" => {
                usage("ytta_v0");
                process::exit(0);
            }
            _ => {
                usage("ytta_v0");
                process::exit(1);
            }
        }
    }
    let (Some(fixture), Some(out_path)) = (fixture, out_path) else {
        usage("ytta_v0");
        process::exit(1);
    };

    let ticks = match load_ticks(&fixture) {
        Ok(t) => t,
        Err(e) => {
            eprintln!("error: {e}");
            process::exit(1);
        }
    };

    let mut probe = LatencyProbe::new();
    let events = run_pipeline_mode(&ticks, &mut probe, mode);

    let mut file = match File::create(&out_path) {
        Ok(f) => f,
        Err(e) => {
            eprintln!("error: {e}");
            process::exit(1);
        }
    };
    for e in &events {
        if writeln!(file, "{}", e.line).is_err() {
            eprintln!("error: write failed");
            process::exit(1);
        }
    }

    let s = probe.summarize();
    eprintln!(
        "{{\"type\":\"latency\",\"count\":{},\"p50_ns\":{},\"p99_ns\":{},\"ingest_p50_ns\":{},\"ingest_p99_ns\":{},\"decide_p50_ns\":{},\"decide_p99_ns\":{},\"execute_p50_ns\":{},\"execute_p99_ns\":{},\"drops\":{}}}",
        s.count,
        s.p50_ns,
        s.p99_ns,
        s.ingest_p50_ns,
        s.ingest_p99_ns,
        s.decide_p50_ns,
        s.decide_p99_ns,
        s.execute_p50_ns,
        s.execute_p99_ns,
        s.drops
    );

    if s.drops != 0 {
        eprintln!("error: queue drops={}", s.drops);
        process::exit(1);
    }
}
