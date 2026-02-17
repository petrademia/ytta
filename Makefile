.PHONY: all help cpp cpp-run cpp-mem-pool-run cpp-bench cpp-test rust rust-run rust-test go go-run go-test java java-run java-test test

all: cpp rust go java

help:
	@echo "Targets: cpp cpp-run cpp-mem-pool-run cpp-bench cpp-test rust rust-run rust-test go go-run go-test java java-run java-test test"

cpp:
	cmake -S cpp -B cpp/build
	cmake --build cpp/build

cpp-run: cpp
	./cpp/build/ytta_thread_demo

cpp-mem-pool-run: cpp
	./cpp/build/ytta_mem_pool_demo

cpp-bench:
	@N=$${N:-10}; BIN=$${BIN:-./cpp/build/ytta_thread_demo}; OUT=$$(mktemp); \
	i=1; \
	while [ $$i -le $$N ]; do \
		$$BIN >> $$OUT; \
		i=$$((i + 1)); \
	done; \
	awk -F= '/^t1 launch ns=/{a+=$$2;c++} /^t2 launch ns=/{b+=$$2} /^t1 execution ns=/{d+=$$2} /^t2 execution ns=/{e+=$$2} /^total wall ns=/{f+=$$2} /^t1 launch cycles=/{g+=$$2} /^t2 launch cycles=/{h+=$$2} /^total cycles=/{j+=$$2} END {if (c==0) {print "No benchmark lines found"; exit 1} printf "runs=%d\navg t1 launch ns=%.2f\navg t2 launch ns=%.2f\navg t1 execution ns=%.2f\navg t2 execution ns=%.2f\navg total wall ns=%.2f\navg t1 launch cycles=%.2f\navg t2 launch cycles=%.2f\navg total cycles=%.2f\n", c, a/c, b/c, d/c, e/c, f/c, g/c, h/c, j/c}' $$OUT; \
	rm -f $$OUT

cpp-test:
	@echo "No C++ test runner configured yet"

rust:
	cargo build --manifest-path rust/Cargo.toml

rust-run:
	cargo run --manifest-path rust/Cargo.toml

rust-test:
	cargo test --manifest-path rust/Cargo.toml

go:
	cd go && go build ./...

go-run:
	cd go && go run ./cmd/ytta

go-test:
	cd go && go test ./...

java:
	mvn -q -f java/pom.xml package

java-run: java
	java -cp java/target/ytta-java-0.1.0.jar com.ytta.App

java-test:
	mvn -q -f java/pom.xml test

test: rust-test go-test java-test
