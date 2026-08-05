.PHONY: all help cpp cpp-test test-m0 test-m1 test-m2 test-m3 test-m4 test-m5 test-m6 test

all: cpp

help:
	@echo "Targets: cpp cpp-test test-m0 test-m1 test-m2 test-m3 test-m4 test-m5 test-m6 test"

cpp:
	cmake -S cpp -B cpp/build
	cmake --build cpp/build

test-m0: cpp
	./cpp/build/ytta_m0_smoke

test-m1: cpp
	./cpp/build/ytta_m1_tests

test-m2: cpp
	./cpp/build/ytta_m2_tests

test-m3: cpp
	./cpp/build/ytta_m3_tests

test-m4: cpp
	./cpp/build/ytta_m4_tests
	./cpp/tests/run_m4_golden.sh

test-m5: cpp
	./cpp/build/ytta_m5_tests

test-m6: cpp
	./cpp/build/ytta_m6_tests

cpp-test: test-m0
	@echo "Note: test-m1..m6 are expected to FAIL until you implement them."
	-./cpp/build/ytta_m1_tests; \
	-./cpp/build/ytta_m2_tests; \
	-./cpp/build/ytta_m3_tests; \
	-./cpp/build/ytta_m4_tests; \
	-./cpp/build/ytta_m5_tests; \
	-./cpp/build/ytta_m6_tests; \
	true

test: test-m0
