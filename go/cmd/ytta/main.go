package main

import (
	"fmt"
	"time"

	"ytta/go/internal/threadutils"
)

func dummyFunction(a int, b int, shouldSleep bool) {
	fmt.Printf("dummyFunction(%d, %d)\n", a, b)
	fmt.Printf("dummyFunction output=%d\n", a+b)

	if shouldSleep {
		fmt.Println("dummyFunction sleeping...")
		time.Sleep(2 * time.Second)
	}

	fmt.Println("dummyFunction done.")
}

func main() {
	t1, err := threadutils.CreateAndStart("dummyFunction1", func() {
		dummyFunction(12, 21, false)
	})
	if err != nil {
		fmt.Println(err)
		return
	}

	t2, err := threadutils.CreateAndStart("dummyFunction2", func() {
		dummyFunction(15, 51, true)
	})
	if err != nil {
		fmt.Println(err)
		t1.Wait()
		return
	}

	fmt.Println("main waiting for threads to be done.")
	t1.Wait()
	t2.Wait()
	fmt.Println("main exiting.")
}
