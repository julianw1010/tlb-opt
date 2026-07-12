#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/mman.h>

static atomic_int stop;
static volatile char *page;

static void pin_to_cpu(int cpu)
{
	cpu_set_t set;

	CPU_ZERO(&set);
	CPU_SET(cpu, &set);
	if (pthread_setaffinity_np(pthread_self(), sizeof(set), &set)) {
		perror("pthread_setaffinity_np");
		exit(1);
	}
}

static void *spinner(void *arg)
{
	long cpu = (long)arg;
	volatile char sink;

	pin_to_cpu(cpu);
	while (!atomic_load_explicit(&stop, memory_order_relaxed))
		sink = page[0];
	(void)sink;
	return NULL;
}

int main(int argc, char **argv)
{
	long iters = 200000;
	long ncpus = sysconf(_SC_NPROCESSORS_ONLN);
	long pagesz = sysconf(_SC_PAGESIZE);
	pthread_t *threads;
	long cpu, i;

	if (argc > 1)
		iters = atol(argv[1]);

	page = mmap(NULL, pagesz, PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (page == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	page[0] = 1;

	threads = calloc(ncpus, sizeof(*threads));
	if (!threads) {
		perror("calloc");
		return 1;
	}

	pin_to_cpu(0);
	for (cpu = 1; cpu < ncpus; cpu++) {
		if (pthread_create(&threads[cpu], NULL, spinner,
				   (void *)cpu)) {
			perror("pthread_create");
			return 1;
		}
	}

	sleep(1);

	for (i = 0; i < iters; i++) {
		if (mprotect((void *)page, pagesz, PROT_READ)) {
			perror("mprotect");
			return 1;
		}
		if (mprotect((void *)page, pagesz,
			     PROT_READ | PROT_WRITE)) {
			perror("mprotect");
			return 1;
		}
	}

	atomic_store(&stop, 1);
	for (cpu = 1; cpu < ncpus; cpu++)
		pthread_join(threads[cpu], NULL);

	printf("done: %ld protection-flip iterations on %ld cpus\n",
	       iters, ncpus);
	return 0;
}
