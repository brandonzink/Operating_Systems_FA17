#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "util.h"
#include "queue.h"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"
#define TEST_SIZE 10
#define NUM_THREADS 10

// Thread Data Structures
struct dynamicArray{
    FILE *array[5];
    int size;
};

struct RequestData{
    long threadNumber;
    struct dynamicArray inputFiles;
    queue* q;
    int filesServiced;
};
struct ResolveData{
    FILE* outputFile;
    queue* q;
};

/*
	Contains all of the code for a requester thread and it's actions with
	reading from files and writing to the queue. Doesn't return anything. Input is
	used for creating a uniquely named struct. Also writes to the serviced.txt file.
*/
void* RequestThread(void* threadarg);

/*
	Contains all of the code for a resolver thread and it's actions with
	writing to files and reading from the queue. Doesn't return anything. Input is
	used for creating a uniquely named struct.
*/
void* ResolveThread(void* threadarg);

/*
	Used for calculating the current time for runtime. Called at the beginning and the
	end of the code, and the time taken is calculated then. 
*/
long long current_timestamp();

/*
	main allocates all of the threads taken from the user inputs in argv[], contains a
	few error checks on inputs, and cleans up the data after it's finished. This is the main
	function that runs.
*/
int main(int argc, char* argv[]);

