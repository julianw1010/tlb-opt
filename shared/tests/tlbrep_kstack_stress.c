#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BATCH 16

static void *nop(void *arg)
{
	return NULL;
}

int main(int argc, char **argv)
{
	long iters = 20000;
	pthread_t threads[BATCH];
	long i, j;

	if (argc > 1)
		iters = atol(argv[1]);

	for (i = 0; i < iters; i++) {
		for (j = 0; j < BATCH; j++) {
			if (pthread_create(&threads[j], NULL, nop, NULL)) {
				perror("pthread_create");
				return 1;
			}
		}
		for (j = 0; j < BATCH; j++)
			pthread_join(threads[j], NULL);
	}

	printf("done: %ld batches of %d threads\n", iters, BATCH);
	return 0;
}
