use std::thread;
use std::time::Duration;

use crate::thread_utils::create_and_start_thread;

fn dummy_function(a: i32, b: i32, should_sleep: bool) {
    println!("dummyFunction({}, {})", a, b);
    println!("dummyFunction output={}", a + b);

    if should_sleep {
        println!("dummyFunction sleeping...");
        thread::sleep(Duration::from_secs(2));
    }

    println!("dummyFunction done.");
}

pub fn run_thread_creation_demo() -> i32 {
    let t1 = match create_and_start_thread("dummyFunction1", move || {
        dummy_function(12, 21, false);
    }) {
        Ok(handle) => handle,
        Err(err) => {
            eprintln!("{err}");
            return 1;
        }
    };

    let t2 = match create_and_start_thread("dummyFunction2", move || {
        dummy_function(15, 51, true);
    }) {
        Ok(handle) => handle,
        Err(err) => {
            eprintln!("{err}");
            let _ = t1.join();
            return 1;
        }
    };

    println!("main waiting for threads to be done.");
    if t1.join().is_err() {
        eprintln!("dummyFunction1 panicked");
        let _ = t2.join();
        return 1;
    }

    if t2.join().is_err() {
        eprintln!("dummyFunction2 panicked");
        return 1;
    }

    println!("main exiting.");
    0
}
