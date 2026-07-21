use std::env;
use std::fs::File;
use std::io::Write;
use std::process;

use ytta_rust::v0::{load_ticks, run_pipeline, LatencyProbe};

fn usage(argv0: &str) {
    eprintln!("Usage: {argv0} --fixture <ticks.ndjson> --out <events.ndjson>");
}

fn main() {
    let mut fixture = None;
    let mut out_path = None;
    let mut args = env::args().skip(1);
    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--fixture" => fixture = args.next(),
            "--out" => out_path = args.next(),
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
    let events = run_pipeline(&ticks, &mut probe);

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
        "{{\"type\":\"latency\",\"count\":{},\"p50_ns\":{},\"p99_ns\":{}}}",
        s.count, s.p50_ns, s.p99_ns
    );
}
