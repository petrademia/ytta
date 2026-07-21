#[derive(Default)]
pub struct LatencyProbe {
    samples: Vec<i64>,
}

#[derive(Debug, Clone, Copy)]
pub struct LatencySummary {
    pub count: u64,
    pub p50_ns: i64,
    pub p99_ns: i64,
}

impl LatencyProbe {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn record_ns(&mut self, delta: i64) {
        self.samples.push(delta);
    }

    pub fn summarize(&self) -> LatencySummary {
        let count = self.samples.len() as u64;
        if self.samples.is_empty() {
            return LatencySummary {
                count: 0,
                p50_ns: 0,
                p99_ns: 0,
            };
        }
        let mut sorted = self.samples.clone();
        sorted.sort_unstable();
        let idx = |pct: f64| -> usize {
            if sorted.len() == 1 {
                0
            } else {
                (pct * (sorted.len() - 1) as f64) as usize
            }
        };
        LatencySummary {
            count,
            p50_ns: sorted[idx(0.50)],
            p99_ns: sorted[idx(0.99)],
        }
    }
}
