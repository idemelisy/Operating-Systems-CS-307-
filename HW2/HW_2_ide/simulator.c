#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include "sdd.h"
#include "constants.h"

extern int stop_threads;
extern SortedDispatcherDatabase** processor_queues;

#define LOW_WATERMARK 3      // below this we start stealing work
#define HIGH_WATERMARK 8     // above this we try to donate work
#define IDLE_SLEEP_USEC 5000 // microseconds to sleep when idle

// Returns the heaviest queue (excluding self) that exceeds the high watermark.
static SortedDispatcherDatabase* pick_heavy_queue(int self_id) {
    SortedDispatcherDatabase* chosen = NULL;
    int heaviest = 0;
    for (int i = 0; i < NUM_CORES; i++) {
        if (i == self_id) continue;
        int size = queue_size(processor_queues[i]);
        if (size > HIGH_WATERMARK && size > heaviest) {// the heaviest one among the ones that exceed high_watermark.
            heaviest = size;
            chosen = processor_queues[i];
        }
    }
    return chosen;
}

// Returns the lightest queue (excluding self) that can accept migrated work.
static SortedDispatcherDatabase* pick_light_queue(int self_id) {
    SortedDispatcherDatabase* chosen = NULL;
    int lightest = HIGH_WATERMARK + 1;
    for (int i = 0; i < NUM_CORES; i++) {
        if (i == self_id) continue;
        int size = queue_size(processor_queues[i]);
        if (size < LOW_WATERMARK && size < lightest) {// the lightest one among the ones that are less than low_watermark.
            lightest = size;
            chosen = processor_queues[i];
        }
    }
    return chosen;
}

// If the current queue is overloaded, move the most expensive job to a lighter core.
static void donate_if_overloaded(SortedDispatcherDatabase* my_queue, int my_id) {
    int current_size = queue_size(my_queue);
    if (current_size <= HIGH_WATERMARK) return; // if the queue is not overloaded, return

    SortedDispatcherDatabase* receiver = pick_light_queue(my_id);
    if (!receiver) return; // if no receiver found, return

    Task* donated = fetchTaskFromOthers(my_queue);
    if (!donated) return; // if no donated task found, return

    submitTask(receiver, donated); // submit the donated task to the receiver queue
}

// Thread function for each core simulator thread
void* processJobs(void* arg) {
    // initalize local variables
    ThreadArguments* my_arg = (ThreadArguments*) arg;
    SortedDispatcherDatabase* my_queue = my_arg -> q;
    int my_id = my_arg -> id;
    free(my_arg);

    
    while (!stop_threads) {
        Task* task = fetchTask(my_queue);

        if (!task) {
            int my_size = queue_size(my_queue);
            if (my_size < LOW_WATERMARK) {
                SortedDispatcherDatabase* target = pick_heavy_queue(my_id);
                if (target) {
                    task = fetchTaskFromOthers(target);
                }
            }
        }

        if (!task) {
            usleep(IDLE_SLEEP_USEC);
            continue;
        }

        executeJob(task, my_queue, my_id);

        if (task->task_duration > 0) {
            submitTask(my_queue, task);
        } else {
            free(task->task_id);
            free(task);
        }

        donate_if_overloaded(my_queue, my_id);
    }

    return NULL;
}


void initSharedVariables() {
    for (int i = 0; i < NUM_CORES; i++) {
        init_queue(processor_queues[i]);
    }
}