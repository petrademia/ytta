package v0

import "testing"

func TestSpscFIFOAndBounds(t *testing.T) {
	q, err := NewSpscQueue[int](4)
	if err != nil {
		t.Fatal(err)
	}
	if !q.TryPush(1) || !q.TryPush(2) || !q.TryPush(3) || !q.TryPush(4) {
		t.Fatal("push 1-4")
	}
	if q.TryPush(5) {
		t.Fatal("expected full")
	}
	for want := 1; want <= 4; want++ {
		got, ok := q.TryPop()
		if !ok || got != want {
			t.Fatalf("fifo want %d got %d ok=%v", want, got, ok)
		}
	}
	if _, ok := q.TryPop(); ok {
		t.Fatal("expected empty")
	}
}

func TestSpscReuseAfterPop(t *testing.T) {
	q, err := NewSpscQueue[string](2)
	if err != nil {
		t.Fatal(err)
	}
	if !q.TryPush("a") || !q.TryPush("b") {
		t.Fatal("push")
	}
	if q.TryPush("c") {
		t.Fatal("full")
	}
	v, ok := q.TryPop()
	if !ok || v != "a" {
		t.Fatal("pop a")
	}
	if !q.TryPush("c") {
		t.Fatal("reuse slot")
	}
	v, _ = q.TryPop()
	if v != "b" {
		t.Fatalf("want b got %s", v)
	}
	v, _ = q.TryPop()
	if v != "c" {
		t.Fatalf("want c got %s", v)
	}
}

func TestSpscBadCapacity(t *testing.T) {
	if _, err := NewSpscQueue[int](3); err == nil {
		t.Fatal("expected error for non-power-of-two")
	}
}

func TestObjectPoolReuse(t *testing.T) {
	type node struct{ n int }
	created := 0
	pool := NewObjectPool(func() *node {
		created++
		return &node{}
	}, 0)
	a := pool.Acquire()
	a.n = 7
	b := pool.Acquire()
	b.n = 8
	if a == b {
		t.Fatal("distinct")
	}
	pool.Release(a)
	c := pool.Acquire()
	if c != a {
		t.Fatal("expected reuse")
	}
	c.n = 9
	pool.Release(b)
	pool.Release(c)
	if pool.FreeCount() != 2 {
		t.Fatalf("free=%d", pool.FreeCount())
	}
	before := created
	for i := 0; i < 1000; i++ {
		p := pool.Acquire()
		p.n = i
		pool.Release(p)
	}
	if created != before {
		t.Fatalf("pool grew on reuse loop: %d -> %d", before, created)
	}
}
