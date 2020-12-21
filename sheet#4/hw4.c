/*Musab Mehadi
mmehadi@jacobs-university.de*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>

int N = 10000;
int P = 100;
char coins[] = {'O','O','O','O','O','O','O','O','O','O','X','X','X','X','X','X','X','X','X','X'};
static double timer(int n, void* (*q)(void *));
char flipper(char j);
void create_join_threads(unsigned int n, void* (*proc)(void *));
static void *case1(void *b);
static void *case2(void *b);
static void *case3(void *b);

int main(int argc, char* argv[])
{
    int i;
    double t1;
	double t2;
	double t3;
    /* Getting the command line options */
    while((i = getopt(argc, argv, "p:n:")) != -1)
    {
        if(i == 'p')
        {
            int persons_no;
            persons_no = atoi(optarg);
            if(persons_no <= 0)
            {
                perror("Error, invalid number of people.\n");
                exit(1);
            }
            P = persons_no;
        }
        else if(i == 'n')
        {
            int flips_no;
            flips_no = atoi(optarg);
            if(flips_no <= 0)
            {
                perror("Error, invalid number of flips.\n");
                exit(1);
            }
            N = flips_no;
        }
        else
        {
            perror("Error, invalid command.\n");
            exit(1);
        }
    }
    
	// Using method 1 
    printf("coins: %s (start - global lock)\n", coins);
    t1 = timer(P,case1);
    printf("coins: %s (end - global lock)\n", coins);
    printf("%d threads x %d flips: %.3lf ms\n\n",P,N,t1); 
    // Using method 2
    printf("coins: %s (start - iteration lock)\n", coins);
    t2 = timer(P,case2);
    printf("coins: %s (end - table lock)\n", coins);
    printf("%d threads x %d flips: %.3lf ms\n\n",P, N, t2);
	// Using method 3 
    printf("coins: %s (start - coin lock)\n", coins);
    t3 = timer(P, case3);
    printf("coins: %s (end - coin lock)\n", coins);
    printf("%d threads x %d flips: %.3lf ms\n\n",P, N, t3);
	return 0;
}

// coin flipping function
char flipper(char j)
{
    if(j == 'X')
    {
        return 'O';
    }
    else
    {
        return 'X';
    }
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static const char *program = "pthread";
void create_join_threads(unsigned int n, void* (*proc)(void *))
{
    int i;
	int k;
	int rc;
    pthread_t *thread;
	thread = calloc(n, sizeof(pthread_t));
    if (!thread) 
    {
        fprintf(stderr, "%s: %s: %s\n", program, __func__, strerror(errno));
        exit(1);
    }
	for ( i = 0; i < n; i++) 
    {
        rc = pthread_create(&thread[i], NULL, proc, NULL);
        if (rc) 
        {
            fprintf(stderr, "%s: %s: unable to create thread %d: %s\n",
                    program, __func__, i, strerror(rc));
        }
    }
	for (k = 0; k < n; k++) 
    {
        if (thread[k]) 
        {
            (void) pthread_join(thread[k], NULL);
	    }
    }
	(void) free(thread);
}

// Method 1
static void *case1(void *b)
{
	int i;
	int j;
    pthread_mutex_lock(&mutex);
    for( i = 0; i < N; i++)
    {
        for(j = 0; j < 20; j++)
        {
            coins[j] = flipper(coins[j]);
        }
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

//Method 2
static void *case2(void *b)
{
	int i;
	int j;
    for(i = 0; i < N; i++)
    {
        pthread_mutex_lock(&mutex);
        for(j = 0; j < 20; j++)
        {
            coins[j] = flipper(coins[j]);
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// Method 3
static void *case3(void *b)
{
	int i;
	int j;
    for(i = 0; i < N; i++)
    {
        for(j = 0; j < 20; j++)
        {
            pthread_mutex_lock(&mutex);
            coins[j] = flipper(coins[j]);
            pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}
  // Timer
    static double timer(int n, void* (*q)(void *))
    {
    	clock_t T1 = clock();
    	clock_t T2 = clock();
    	create_join_threads(n,q);
    	return ((double) T2 - (double) T1) / CLOCKS_PER_SEC * 1000;
	}
  
  
  
  
