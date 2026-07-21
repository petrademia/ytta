use std::fs;
use std::path::PathBuf;

use ytta_rust::v0::{load_ticks, run_pipeline_mode, LatencyProbe, PipelineMode};

fn repo_root() -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("..")
}

#[test]
fn golden_parity_sync_and_queued() {
    let root = repo_root();
    let fixture = root.join("shared/fixtures/v0/ticks.ndjson");
    let golden_path = root.join("shared/fixtures/v0/golden.ndjson");
    let ticks = load_ticks(&fixture).expect("load ticks");
    let want = fs::read_to_string(&golden_path).expect("read golden");

    for mode in [PipelineMode::Sync, PipelineMode::Queued] {
        let mut probe = LatencyProbe::new();
        let events = run_pipeline_mode(&ticks, &mut probe, mode);
        let mut got = String::new();
        for e in &events {
            got.push_str(&e.line);
            got.push('\n');
        }
        assert_eq!(got, want, "golden drift mode={mode:?}");
        assert_eq!(probe.summarize().drops, 0);
    }
}

#[test]
fn burst_queued_no_drops() {
    let root = repo_root();
    let fixture = root.join("shared/fixtures/v1/ticks_burst.ndjson");
    let ticks = load_ticks(&fixture).expect("load burst");
    let mut probe = LatencyProbe::new();
    let events = run_pipeline_mode(&ticks, &mut probe, PipelineMode::Queued);
    let s = probe.summarize();
    assert_eq!(s.drops, 0);
    assert_eq!(s.count as usize, ticks.len());
    let actions = events
        .iter()
        .filter(|e| e.line.contains("\"type\":\"action\""))
        .count();
    assert_eq!(actions, ticks.len());
}
