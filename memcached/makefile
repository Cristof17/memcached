build: profiler
	gcc profiler.c -c -o profiler.o -lmemcached
profiler.o: profiler.c
	gcc profiler.c -c -o profiler.o
profiler: profiler.o
	gcc profiler.o -lmemcached -o profiler
clean:
	rm *.o
	rm profiler

.PHONY: clean build
