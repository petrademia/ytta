package v0

// ObjectPool is a tiny free-list pool for reusable values.
type ObjectPool[T any] struct {
	free []T
	new  func() T
}

func NewObjectPool[T any](factory func() T, prewarm int) *ObjectPool[T] {
	p := &ObjectPool[T]{new: factory}
	for i := 0; i < prewarm; i++ {
		p.free = append(p.free, factory())
	}
	return p
}

func (p *ObjectPool[T]) Acquire() T {
	n := len(p.free)
	if n == 0 {
		return p.new()
	}
	v := p.free[n-1]
	p.free = p.free[:n-1]
	return v
}

func (p *ObjectPool[T]) Release(v T) {
	p.free = append(p.free, v)
}

func (p *ObjectPool[T]) FreeCount() int { return len(p.free) }
