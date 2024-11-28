//#define _XOPEN_SOURCE
// Marco Drochner marcodrochner

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "cachelab.h"
#include <string.h>

#define MAX_LINE_LENGTH 64
#define NEW 1
#define NO_UNUSED_LINES_LEFT -1
/* Function declaration */
void usage(void);
void goThroughItAll(int *hitCount, int *missCount, int*evictionCount);

// int hexStringToInt(char*string);
// representing e.g. "S 7ff0005c8,8"
struct transaction {
    char op;
    unsigned int address;
};
int nextTransaction();


struct transaction transaction;
int verbose = 0; // Whether to print our all the extra info for debugging
int S, s, E, b; // # set bits, # lines per set, # of block bits
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
                S = 1<<s;
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
                    printf("%s: No such file or directory\n", optarg);
                    return -1;
                }
                break;
            default:
                usage();
        } 
    }
    if (!(s&&E&&b&&S) || traceFP == NULL) {
        printf("\n?????Missing commands");
        return 1;
    }
    int hits = 0, misses = 0, evictions = 0;
    goThroughItAll(&hits, &misses, &evictions);
    printSummary(hits, misses, evictions);
    return 0;
}

/**
 * returns 0 if at end of trace file.
 */
int nextTransaction() {
    /*
     * Just in case for academic honesty: I asked ChatGPT how to read lines into a struct,
     * but the code needed reworking (it misunderstood a bit), but I was inspired to use fgets and sscanf.
     * But later of course Dr. Pohly just showed that we can use fscanf :facepalm: so I *peronally don't think
     * this is a case of credit reduction
     */
    char line[MAX_LINE_LENGTH];

    do {
        if (fgets(line, MAX_LINE_LENGTH, traceFP) == NULL)
            return 0; // Indicates no more lines
    }
    while (line[0] == 'I');

    int uselessParameter;
    int howMany = sscanf(line, " %c %x,%d\n", &(transaction.op), &(transaction.address), &uselessParameter);
    if (howMany != 3) {
        printf("\n\nError scanning file\n\n");
        return 0;
    }

    return 1; // Indicates that there is a next line, which is in "transaction"
}



void usage(void) {
    printf(
        "Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
        "Options:\n"
        "  -h         Print this help message.\n"
        "  -v         Optional verbose flag.\n"
        "  -s <num>   Number of set index bits.\n"
        "  -E <num>   Number of lines per set.\n"
        "  -b <num>   Number of block offset bits.\n"
        "  -t <file>  Trace file.\n"
        "\n"
        "Examples:\n"
        "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
        "  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n"    );
}

// int hexStringToInt(char*string)
// {
//     int len = strlen(string);
//     int ret = 0;
//     char charString[2];
//     charString[1] = '\0';

//     int i = len-1, placeMult = 1;
//     while (i >= 0)
//     {
//         charString[0] = string[i];
//         int value = atoi(charString);
//         // if letter, get val from hex meaning of ascii
//         if (!value && string[i] != '0')
//             value = string[i] - 87;
//         ret += value * placeMult;


//         i--;
//         placeMult *= 16;
//     }
//     return ret;
// }

void closeFileAndExit(int code)
{
    fclose(traceFP);
    exit(code);
}


struct cache_line {
    unsigned int valid;
    unsigned int tag;
};

struct cache_lines_queue {
    int indexOfIndexToEvict;
    int nextSpotToEnqueue;
    int *array;
};

/*
 * Assumes un-enqueued spots in array are -1
 */
void hitAccess (struct cache_lines_queue *queue, int lineNum)
{
    // Find in queue
    int index;
    for (int i = 0; i < E; i++) {
        if (lineNum == queue->array[i]) {
            index = i;
            break;
        }
    }
    int newestI = (queue->nextSpotToEnqueue -1 + E)%E;
            
    // move to most recently used
    for (int i = index; i != newestI; i = (i+1)%E) {
        // similar to bubble sort, shift everything after it leftwards 
        // as if you removed the found cache line
        queue->array[i] = queue->array[(i+1)%E];
    }
    // now put it in the most recent spot
    queue->array[newestI] = lineNum;
}

int missAccess (struct cache_lines_queue *queue, int lineToReplace)
{
    // If nothing free, must evict
    if (lineToReplace == NO_UNUSED_LINES_LEFT) {
        lineToReplace = queue->array[queue->indexOfIndexToEvict];
        assert(queue->indexOfIndexToEvict == queue->nextSpotToEnqueue);
        queue->indexOfIndexToEvict 
            = (queue->indexOfIndexToEvict + 1)%E;
    }
    queue->array[queue->nextSpotToEnqueue] = lineToReplace;
    queue->nextSpotToEnqueue = (queue->nextSpotToEnqueue + 1)%E;
    return lineToReplace;
}

struct cache_lines_queue * initArrayOfQueues()
{
    struct cache_lines_queue *queues = malloc(sizeof(struct cache_lines_queue) * S);
    for (int i = 0; i < S; i++) {
        queues[i].array = malloc(sizeof(int) * E);
        /*so that it doesn't "find" a cache line in the queue 
          as recently used purely by chance */
        for (int j = 0; j < E; j++)
            queues[i].array[j] = -1;
    }
    return queues;
}

void goThroughItAll(int *hitCount, int *missCount, int *evictionCount)
{
    struct cache_lines_queue *queues = initArrayOfQueues();
    struct cache_line cacheLines[S*E];
    for (int i = 0; i < S*E; ++i)
        cacheLines[i].valid = 0;

    while (nextTransaction()) {
        if (transaction.op == 'M')
            *hitCount +=1;
        unsigned int setNum = (transaction.address>>b) % (1<<s);
        unsigned int tag = transaction.address>>(b+s);
        int found = 0;
        int unusedLineToPutIn = NO_UNUSED_LINES_LEFT;
        for (int i = 0; i < E; i++) {
            if (cacheLines[setNum*E + i].valid) {
                // hit?
                if (tag == cacheLines[setNum*E + i].tag) {
                    *hitCount += 1;
                    found = 1;
                    hitAccess(&queues[setNum], i);
                    break;
                }

            } else {
                unusedLineToPutIn = i;
            }
        }
        // miss?
        if (!found) {
            *missCount += 1;
            if (unusedLineToPutIn == NO_UNUSED_LINES_LEFT)
                *evictionCount += 1;
            // update queue
            int replacedLine // I wish this '=' style was more popular...
            = missAccess(&queues[setNum], unusedLineToPutIn);

            cacheLines[setNum*E + replacedLine].valid = 1;
            cacheLines[setNum*E + replacedLine].tag = tag;
        }
    }
    // nextTransaction() rets 0 when at end of file.
    

}