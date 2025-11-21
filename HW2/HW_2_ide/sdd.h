#ifndef SDD_H
#define SDD_H

#include <pthread.h>


typedef struct SortedDispatcherDatabase SortedDispatcherDatabase;
typedef struct TaskNode TaskNode;

typedef struct ThreadArguments {
    SortedDispatcherDatabase* q;
    int id;
} ThreadArguments;

typedef struct Task {
    char* task_id;
    int task_duration;
	double cache_warmed_up;
	SortedDispatcherDatabase* owner;
} Task;


struct SortedDispatcherDatabase {
    TaskNode* head;
    TaskNode* tail;
    int size;
    pthread_mutex_t lock;
};



void submitTask(SortedDispatcherDatabase* q, Task* _task);//add a new job to the sdd
Task* fetchTask(SortedDispatcherDatabase* q);//get a job from sdd (owner t)
Task* fetchTaskFromOthers(SortedDispatcherDatabase* q);//get the longest job from sdd (not by the owner)
void print_queue(SortedDispatcherDatabase* q, int core_id);//show the queue sorted
void init_queue(SortedDispatcherDatabase* q); // initializing
int queue_size(SortedDispatcherDatabase* q); // to see how many jobs left in the queue




void executeJob(Task* task, SortedDispatcherDatabase* my_queue, int my_id );
void* processJobs(void* arg);
void initSharedVariables();
#endif