package v0

import (
	"fmt"
	"sync/atomic"
)

// SpscQueue is a bounded single-producer/single-consumer ring.
// Capacity must be a power of two.
type SpscQueue[T any] struct {
	buf   []T
	mask  uint64
	write atomic.Uint64
	read  atomic.Uint64
}

func NewSpscQueue[T any](capacity int) (*SpscQueue[T], error) {
	if capacity < 2 || capacity&(capacity-1) != 0 {
		return nil, fmt.Errorf("spsc capacity must be power of two >= 2, got %d", capacity)
	}
	return &SpscQueue[T]{
		buf:  make([]T, capacity),
		mask: uint64(capacity - 1),
	}, nil
}

func (q *SpscQueue[T]) Capacity() int { return len(q.buf) }

func (q *SpscQueue[T]) TryPush(v T) bool {
	w := q.write.Load()
	r := q.read.Load()
	if w-r >= uint64(len(q.buf)) {
		return false
	}
	q.buf[w&q.mask] = v
	q.write.Store(w + 1)
	return true
}

func (q *SpscQueue[T]) TryPop() (T, bool) {
	var zero T
	r := q.read.Load()
	w := q.write.Load()
	if r == w {
		return zero, false
	}
	v := q.buf[r&q.mask]
	q.read.Store(r + 1)
	return v, true
}
