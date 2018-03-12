#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

/*
 *  Program that exercises posix mutex code.  It takes as arguments a number of threads (default is 1) and
 *  a number of iterations (increments of a counter, default is -1, i.e., run forever).  Each thread
 *  loops, acquires a mutex and checks to see if the counter has reached the number of iterations.  If
 *  the counter is less than the number of iterations, it is incremented and the lock is released.  If the counter
 *  has reached the number of iterations, the thread unlocks the lock and pthread_exits.
 *
 *  To build:
 *	gcc -m64 -O -lpthread lock.c -o lock
 */

pthread_mutex_t thelock = PTHREAD_MUTEX_INITIALIZER;
long counter = 0;

#define ITERATIONS  -1  /* default number of increments of counter, -1 means run forever */
#define NTHREADS 1	/* default number of worker threads */

long iterations = ITERATIONS;
int nthreads = NTHREADS;
int debug = 0;

extern  char *optarg;
extern  int optind, opterr, optopt;

char usage[] = "Usage: %s [-t nthreads] [-i iterations] [-d]\n";

void *
increment(void *arg)
{
	int i;

	for (i = 0; ; i++) {
		pthread_mutex_lock(&thelock);
		if (iterations != -1) {
			if (counter >= iterations) {
				pthread_mutex_unlock(&thelock);
				if (debug)
					printf("Thread %d: looped %d times\n", (int) pthread_self(), i);
				pthread_exit(0);
			} else {
				counter++;
			}
		} else {
			counter++;
		}
		pthread_mutex_unlock(&thelock);
	}
	/*NOTREACHED*/
}

int
main(int argc, char *argv[])
{
	pthread_t *tid;
	int c;
	int i;
	int errflg = 0;
	
       
	if (argc > 1) {
		while ((c = getopt(argc, argv, "t:i:d")) != -1) {
			switch(c) {
			case 'd':
				debug = 1;
				break;
			case 't':
				nthreads = strtol(optarg, NULL, 0);
				if (errno != 0 || nthreads <= 0) {
					fprintf(stderr, "%s: nthread: %s must be > 0\n", argv[0], optarg);
					exit(1);
				}
				break;
			case 'i':
				iterations = strtol(optarg, NULL, 0);
				if (errno != 0 || iterations < -1) {
					fprintf(stderr, "%s: iterations: %s must be >= -1\n", argv[0], optarg);
					exit(1);
				}
				break;
			case '?':
				errflg++;
				break;
			}
		}
	}
	
	if ((optind != argc) || errflg || errno) {
		fprintf(stderr, usage, argv[0]);
		exit(1);
	}

	tid = malloc(nthreads * sizeof(pthread_t *));
	if (tid == NULL) {
		perror("cannot allocate space for threads\n");
		exit(1);
	}

	for (i = 0; i < nthreads; i++)
		pthread_create(&tid[i], NULL, increment, NULL);

	for (i = 0; i < nthreads; i++)
		pthread_join(tid[i], NULL);


	exit(0);
}
