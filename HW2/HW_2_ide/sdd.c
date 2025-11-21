#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "sdd.h"

typedef struct TaskNode {
    Task* task;
    struct TaskNode* prev;
    struct TaskNode* next;
} TaskNode;

// Allocate a new TaskNode wrapper for the incoming task.
static TaskNode* create_node(Task* task) {
    TaskNode* node = malloc(sizeof(TaskNode));
    if (!node) return NULL;
    node->task = task;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

// Removes a node from the doubly linked list and returns its task payload.
static Task* detach_node(SortedDispatcherDatabase* q, TaskNode* node) {
    if (!node) return NULL;
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        q->head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        q->tail = node->prev;
    }

    q->size--;
    Task* task = node->task;
    free(node);
    return task;
}

// Initialize a queue before it is used by simulator threads.
void init_queue(SortedDispatcherDatabase* q) {
    if (!q) return;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    pthread_mutex_init(&q->lock, NULL);
}

// Helper that compares two tasks by remaining duration.
static int compare_tasks(const Task* a, const Task* b) {//if a is shorter return 1
    if (!a && !b) return 0;
    if (!a) return 1;
    if (!b) return -1;
    if (a->task_duration < b->task_duration) return -1;
    if (a->task_duration > b->task_duration) return 1;
    return 0;
}

// Inserts a task so that the list stays sorted in ascending remaining time.
void submitTask(SortedDispatcherDatabase* q, Task* _task) {
    if (!q || !_task) return;//if the queue is empty or the task is null, return

    TaskNode* node = create_node(_task);
    if (!node) return;//if the node is null, return

    pthread_mutex_lock(&q->lock);//Critical section to protect

    TaskNode* current = q->head;
    while (current && compare_tasks(current->task, _task) <= 0) {//move further if current is not null and it is larger than the next task
        current = current->next;
    }

    if (!current) {//if current null we came to the tail it  was the  longest so far
        // insert at tail
        node->prev = q->tail;
        if (q->tail) {
            q->tail->next = node;
        } else {
            q->head = node;
        }
        q->tail = node;
    } else {//if current is inside the queue somewhere
        node->next = current;
        node->prev = current->prev;
        current->prev = node;
        if (node->prev) {
            node->prev->next = node;
        } else {
            q->head = node;
        }
    }

    q->size++;// we've added a new node
    pthread_mutex_unlock(&q->lock);//critical section ended.
}

// Pops the shortest job (head) for the owner thread.
Task* fetchTask(SortedDispatcherDatabase* q) {
    if (!q) return NULL; // if queue is empty no job left
    pthread_mutex_lock(&q->lock); //critical section to protect
    Task* task = detach_node(q, q->head); // get the SHORTEST job to completion.
    pthread_mutex_unlock(&q->lock); //critical section ended.
    return task;
}

// Pops the longest job (tail) for a stealing thread.
Task* fetchTaskFromOthers(SortedDispatcherDatabase* q) {
    if (!q) return NULL; // if queue is empty no job left
    pthread_mutex_lock(&q->lock); //critical section to protect
    Task* task = detach_node(q, q->tail); // get the LONGEST job to completion because this steals from other cores.
    pthread_mutex_unlock(&q->lock); //critical section ended.
    return task;
}

// Snapshot the current queue length in a thread-safe fashion.
int queue_size(SortedDispatcherDatabase* q) {
    if (!q) return 0;
    pthread_mutex_lock(&q->lock);
    int current = q->size;
    pthread_mutex_unlock(&q->lock);
    return current;
}

// Debug helper that prints the queue from head to tail, as required by the spec.
void print_queue(SortedDispatcherDatabase* q, int core_id) {
    if (!q) {
        printf("Core %d queue [size=0]: (empty)\n", core_id);
        return;
    }

    pthread_mutex_lock(&q->lock);
    printf("Core %d queue [size=%d]: ", core_id, q->size);
    if (q->size == 0) {
        printf("(empty)\n");
        pthread_mutex_unlock(&q->lock);
        return;
    }

    TaskNode* current = q->head;
    while (current) {
        Task* task = current->task;
        printf("%s(%d)", task->task_id, task->task_duration);
        if (current->next) {
            printf(" -> ");
        }
        current = current->next;
    }
    printf("\n");
    pthread_mutex_unlock(&q->lock);
}