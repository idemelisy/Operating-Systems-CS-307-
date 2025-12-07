#ifndef STUDY_H
#define STUDY_H

#include <semaphore.h>
#include <pthread.h>
#include <cstdio>
#include <stdexcept>

class Study {
private:
    // Synchronization primitives
    sem_t mutex;              // Protect shared state
    sem_t roomSlots;          // Counting semaphore for available slots in the room
    sem_t sessionStarted;     // Semaphore to notify waiting students when session starts
    sem_t tutorAnnounced;     // Semaphore to block students until tutor announces (if tutor exists)
    sem_t departureBarrier;   // Barrier for departure synchronization (unused)
    
    // Shared state variables
    int sessionSize;          // Minimum students needed for group study
    int tutorPresent;         // Whether tutor is available (0 or 1)
    int studentsInside;       // Current count of students in study center
    int studentsInSession;    // Students currently in active session
    int waitingForSession;    // Threads waiting for a session to start
    bool sessionActive;       // Is a session currently running
    pthread_t sessionTutor;   // Thread ID of the tutor for current session
    bool tutorHasAnnounced;   // Has tutor announced session end

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

