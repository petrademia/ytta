use std::sync::mpsc;
use std::thread::{self, JoinHandle};

pub fn create_and_start_thread<F>(name: &str, f: F) -> Result<JoinHandle<()>, String>
where
    F: FnOnce() + Send + 'static,
{
    let (tx, rx) = mpsc::channel();
    let handle = thread::Builder::new()
        .name(name.to_string())
        .spawn(move || {
            let _ = tx.send(());
            f();
        })
        .map_err(|e| format!("failed to spawn {name}: {e}"))?;

    rx.recv()
        .map_err(|e| format!("failed to receive startup signal from {name}: {e}"))?;

    Ok(handle)
}
