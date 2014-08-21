#include <stdio.h>
#include <time.h> 

#define SIZE 2000
#define TEST_SIZE 100

int largeArray[SIZE][SIZE];

void setup();
long runTest(void (*funPointer)(void));
void loopOverSecondIndex();
void loopOverFirstIndex();
void printStats(char* testRun, long* results);

int main() {
    long firstIndexResults[TEST_SIZE];
    long secondIndexResults[TEST_SIZE];

    for(int i = 0; i < TEST_SIZE; i++) {
        firstIndexResults[i] = runTest(loopOverFirstIndex);
    }
    printStats("First Index [i+1][j]", firstIndexResults);
    
    for(int i = 0; i < TEST_SIZE; i++) {
        secondIndexResults[i] = runTest(loopOverSecondIndex);
    }
    printStats("Second Index [i][j+1]", secondIndexResults);
}

void printStats(char* testRun, long* results) {
    long long averageTime = 0;
    long min = results[0];
    long max = results[0];
    for(int i = 0; i < TEST_SIZE; i++) {
        averageTime += results[i];
        if( results[i] < min ) {
            min = results[i];
        }
        if( results[i] > max ) {
            max = results[i];
        }
    }

    printf("%s\n", testRun);
    printf("---- Stats for run ----\n");
    printf("Max Runtime:        %06ld\n", max);
    printf("Average Runtime:    %06ld\n", (long)(averageTime / TEST_SIZE));
    printf("Min Runtime:        %06ld\n\n", min);
}

void loopOverSecondIndex() {
    for(int i = 1; i < SIZE; i++){
        for(int j = 1; j < SIZE; j++) {
            largeArray[i][j] += largeArray[i][j-1];
        }
    }
}

void loopOverFirstIndex() {
    for(int j = 1; j < SIZE; j++) {
        for(int i = 1; i < SIZE; i++){
            largeArray[i][j] += largeArray[i][j-1];
        }
    }
}

long runTest( void (*funPointer)(void) ) {
    setup();
    clock_t startTime, endTime;
    startTime = clock();

    (*funPointer)();
    
    endTime = clock();
    //return (endTime - startTime)/(double)CLOCKS_PER_SEC;
    return (endTime - startTime);
}

void setup() {
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            largeArray[i][j] = 1;
        }
    }
}
