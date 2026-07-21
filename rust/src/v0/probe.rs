#[derive(Default)]
pub struct LatencyProbe {
    e2e: Vec<i64>,
    ingest: Vec<i64>,
    decide: Vec<i64>,
    execute: Vec<i64>,
    drops: u64,
}

#[derive(Debug, Clone, Copy, Default)]
pub struct LatencySummary {
    pub count: u64,
    pub p50_ns: i64,
    pub p99_ns: i64,
    pub ingest_p50_ns: i64,
    pub ingest_p99_ns: i64,
    pub decide_p50_ns: i64,
    pub decide_p99_ns: i64,
    pub execute_p50_ns: i64,
    pub execute_p99_ns: i64,
    pub drops: u64,
}

impl LatencyProbe {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn add_drop(&mut self) {
        self.drops += 1;
    }

    pub fn record_ns(&mut self, delta: i64) {
        self.e2e.push(delta);
    }

    pub fn record_stages(&mut self, e2e_ns: i64, ingest_ns: i64, decide_ns: i64, execute_ns: i64) {
        self.e2e.push(e2e_ns);
        self.ingest.push(ingest_ns);
        self.decide.push(decide_ns);
        self.execute.push(execute_ns);
    }

    pub fn summarize(&self) -> LatencySummary {
        let (p50, p99) = percentiles(&self.e2e);
        let (ingest_p50, ingest_p99) = percentiles(&self.ingest);
        let (decide_p50, decide_p99) = percentiles(&self.decide);
        let (execute_p50, execute_p99) = percentiles(&self.execute);
        LatencySummary {
            count: self.e2e.len() as u64,
            p50_ns: p50,
            p99_ns: p99,
            ingest_p50_ns: ingest_p50,
            ingest_p99_ns: ingest_p99,
            decide_p50_ns: decide_p50,
            decide_p99_ns: decide_p99,
            execute_p50_ns: execute_p50,
            execute_p99_ns: execute_p99,
            drops: self.drops,
        }
    }
}

fn percentiles(samples: &[i64]) -> (i64, i64) {
    if samples.is_empty() {
        return (0, 0);
    }
    let mut sorted = samples.to_vec();
    sorted.sort_unstable();
    let idx = |pct: f64| -> usize {
        if sorted.len() == 1 {
            0
        } else {
            (pct * (sorted.len() - 1) as f64) as usize
        }
    };
    (sorted[idx(0.50)], sorted[idx(0.99)])
}
