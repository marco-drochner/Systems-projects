//#define _XOPEN_SOURCE
// Marco Drochner marcodrochner

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "cachelab.h"
#define MAX_LINE_LENGTH 64
/* Function declaration */
void usage(void);
void goThroughItAll(void);

// representing e.g. "S 7ff0005c8,8"
struct transaction {
    char op;
    unsigned int address;
    unsigned int size;
};
struct transaction * nextTransaction();


int verbose = 0; // Whether to print our all the extra info for debugging
int s, E, b; // # sets, # lines per set, # of block bits
FILE* traceFP = NULL;

int main(int argc, char **argv)
{
    s = 0; E = 0; b = 0;
    // Note just to be sure of academic honesty:
    //   I asked chatGPT to give me an example of how getopt works given an example of a command like mine
    //   (did not copy paste)
    for (char arg; ((arg = getopt(argc, argv, "hvs:E:b:t:")) != -1);) {
        switch (arg) {
            case 'h':             /* print help message */
                usage();
                return 0;
            case 'v':             /* emit additional diagnostic info */
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                traceFP = fopen(optarg, "r");
                if (traceFP == NULL) {
                    printf("\n\nFile opening error");
                    return -1;
                }
                break;
            default:
                usage();
        } 
    }
    if (!(s&&E&&b) || traceFP == NULL) {
        printf("\n?????Missing commands");
        return 1;
    }
    goThroughItAll();
    // printSummary(0, 0, 0);???
    return 0;
}

/**
 * returns NULL if at end of trace file.
 */
struct transaction * nextTransaction() {
    /*
     * Note just to be sure of academic honesty:
     *  Asked ChatGPT "write a c function that parses a text file named "yi.trace" into a struct,
     *  which I did not directly use (it wanted to put everything into an array of structs at once),
     *  it did, however, inspire my use of fgets and sscanf, in which ChatGPT still didn't take into
     *  account the \n included in fgets lines.
    */
    char line[MAX_LINE_LENGTH];

    do {
        if (fgets(line, MAX_LINE_LENGTH, traceFP) == NULL)
            return NULL;
    }
    while (line[0] == 'I');

    struct transaction *ret = malloc(sizeof(struct transaction));
    sscanf(line, " %c %d,%d\n", &(ret->op), &(ret->address), &(ret->size));
    return ret;
}



void usage(void) {
    printf("\n-h: Optional help flag that prints usage info");
    printf("\n-v: Optional verbose flag that displays trace info");
    printf("\n-s <s>: Number of set index bits (𝑆 = 2^s is the number of sets)");
    printf("\n-E <E>: Associativity (number of lines per set)");
    printf("\n-b <b>: Number of block bits (𝐵 = 2^b is the block size)");
    printf("\n-t <tracefile>: Name of the valgrind trace to replay\n");
}

void closeFileAndExit(int code)
{
    fclose(traceFP);
    exit(code);
}


struct cache_line {
    int valid;
    int tag;
};

struct cache_lines_queue {
    int spotToEvict;
    int nextSpotInQueue;
    int *linesArr;
};

struct cache_lines_set {
    struct cache_line *cache_lines;
    struct cache_lines_queue *cache_lines_queue;
};

/*
 * Assumes un-enqueued spots in array are -1
 */
void loadOrModifyInSet (struct cache_lines_queue *queue, int index)
{
    /* find if already in set */
            int oldestI = queue->spotToEvict;
            // current most recent element, right before the spot for enqueing
            int newestI = (queue->nextSpotInQueue -1 + E)%E;
            // If the queue wraps around array, show that it's still after,
            // and use mod to get actual index in array.
            if (newestI < oldestI) newestI += E;
            // assert(1 + newestI - oldestI == E);
            int found = -1;
            // Check each in the range oldest to newest, including newest
            for (int i = oldestI; i <= newestI; i++)
                if (queue->linesArr[i%E] == index) {
                    found = i;
                    break;
                }
    /* if already there, move it to the most recent position */
    if (found != -1) { // then move it to newest in queue
        for (int i = found; i != newestI; i = (i+1)%E) {
            // similar to bubble sort, shift everything after it leftwards 
            // as if you removed the found cache line
            queue->linesArr[i] = queue->linesArr[(i+1)%E];
        }
        // now put it in the most recent spot
        queue->linesArr[newestI] = index;
    /* otherwise, this is a new save */
    } else {
        // if set's full, we evict, so the oldest is now an index later
        if (oldestI == queue->nextSpotInQueue) {
            oldestI = (oldestI + 1)%E;
            queue->spotToEvict = oldestI;
        }
        queue->linesArr[queue->nextSpotInQueue] = index;
        queue->nextSpotInQueue = (queue->nextSpotInQueue + 1)%E;
    }
}

void goThroughItAll()
{
    struct cache_lines_set * all_cache_lines_sets[s];
    for (int i = 0; i < s; i++)
    {
        struct cache_lines_set *cache_lines_set = malloc(sizeof(cache_lines_set));
        cache_lines_set->cache_lines = malloc(sizeof(struct cache_line * E));

        struct cache_lines_queue *its_queue
            = malloc(sizeof(struct cache_lines_queue));
        cache_lines_queue->spotToEvict = 0;
        cache_lines_queue->nextSpotInQueue = 0;
        cache_lines_queue->linesArr = malloc(sizeof(int) * E);
        for (int j = 0; j < E; j++)
        {
            // so that it doesn't "find" a cache line in the queue 
            // as recently used purely by chance
            cache_lines_queue->linesArr[j] = -1;
        }

        cache_lines_set->cache_lines_queue = its_queue;

        all_cache_lines_sets[i] = cache_lines_set;
    }

}