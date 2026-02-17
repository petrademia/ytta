package threadutils

import (
	"fmt"
)

type Task struct {
	done chan struct{}
}

func (t Task) Wait() {
	<-t.done
}

func CreateAndStart(name string, fn func()) (Task, error) {
	if name == "" {
		return Task{}, fmt.Errorf("thread name cannot be empty")
	}

	started := make(chan struct{})
	done := make(chan struct{})

	go func() {
		close(started)
		defer close(done)
		fn()
	}()

	<-started
	return Task{done: done}, nil
}
