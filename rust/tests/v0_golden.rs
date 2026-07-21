use std::fs;
use std::path::PathBuf;

use ytta_rust::v0::{load_ticks, run_pipeline, LatencyProbe};

fn repo_root() -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("..")
}

#[test]
fn golden_parity() {
    let root = repo_root();
    let fixture = root.join("shared/fixtures/v0/ticks.ndjson");
    let golden_path = root.join("shared/fixtures/v0/golden.ndjson");

    let ticks = load_ticks(&fixture).expect("load ticks");
    let mut probe = LatencyProbe::new();
    let events = run_pipeline(&ticks, &mut probe);

    let mut got = String::new();
    for e in &events {
        got.push_str(&e.line);
        got.push('\n');
    }

    let want = fs::read_to_string(&golden_path).expect("read golden");
    assert_eq!(got, want, "golden drift");
}
