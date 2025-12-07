#ifndef STUDY_H
#define STUDY_H

#include <semaphore.h>
#include <pthread.h>
#include <cstdio>
#include <stdexcept>

class Study {
private:
    // Synchronization primitives (kept simple on purpose)
    sem_t mutex;              // Protect shared state; tried a pure lock once and hit busy-wait
    sem_t roomSlots;          // Counting semaphore for available slots in the room
    sem_t entryGate;          // Binary gate: blocks new arrivals while a session is active
    sem_t sessionStarted;     // Signals when a session begins
    pthread_barrier_t sessionBarrier; // Fans out tutor announcement without loops
    bool barrierInitialized;  // Tracks barrier lifecycle
    pthread_mutex_t printMutex; // Serialize stdout for atomic prints
    
    // Shared state variables
    int sessionSize;          // Minimum students needed for group study
    int tutorPresent;         // Whether tutor is available (0 or 1)
    int studentsInside;       // Current count of students in study center
    int studentsInSession;    // Students currently in active session
    int waitingForSession;    // Threads waiting inside for session to form
    bool sessionActive;       // Is a session currently running
    pthread_t sessionTutor;   // Thread ID of the tutor for current session

public:
    // Constructor
    Study(int sessionSize, int tutorPresent);
    
    // Destructor
    ~Study();
    
    // Main methods
    void arrive();
    void start();
    void leave();
};

#endif // STUDY_H

