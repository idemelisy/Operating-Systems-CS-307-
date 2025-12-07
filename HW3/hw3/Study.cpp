#include "Study.h"
#include <unistd.h>
#include <cstdlib>
#include <stdexcept>
#include <cstdio>
#include <ctime>
#include <cerrno>

// tiny debug switch
#define DEBUG 0
#define DBG(fmt, ...) \
    do { if (DEBUG) printf("[dbg] " fmt "\n", ##__VA_ARGS__); } while (0)

// Constructor
Study::Study(int sessionSize, int tutorPresent) 
    : sessionSize(sessionSize), 
      tutorPresent(tutorPresent),
      studentsInside(0),
      studentsInSession(0),
      waitingForSession(0),
      sessionActive(false),
      barrierInitialized(false) {
    
    
    if (sessionSize < 1) {
        throw std::runtime_error("sessionSize must be a positive integer");
    }
    if (tutorPresent != 0 && tutorPresent != 1) {
        throw std::runtime_error("tutorPresent must be either 0 or 1");
    }
    
    // Initialize semaphores. 
    int capacity = sessionSize + (tutorPresent == 1 ? 1 : 0);
    sem_init(&mutex, 0, 1);                // Binary semaphore for mutex
    sem_init(&roomSlots, 0, capacity);     // Counting semaphore for room capacity
    sem_init(&entryGate, 0, 1);            // Gate open when no active session
    sem_init(&sessionStarted, 0, 0);       // Signals session start to waiters
    pthread_mutex_init(&printMutex, nullptr);
}

// Destructor
Study::~Study() {
    sem_destroy(&mutex);
    sem_destroy(&roomSlots);
    sem_destroy(&entryGate);
    sem_destroy(&sessionStarted);
    if (barrierInitialized) {
        pthread_barrier_destroy(&sessionBarrier);
    }
    pthread_mutex_destroy(&printMutex);
}

// Student arrives to the ic 
void Study::arrive() {
    pthread_t tid = pthread_self();

    sem_wait(&entryGate);   // Block if a session is active
    sem_post(&entryGate);   // Re-open for parallel arrivals while building a session

    pthread_mutex_lock(&printMutex);
    printf("Thread ID: %lu | Status: Arrived at the IC.\n", (unsigned long)tid);
    pthread_mutex_unlock(&printMutex);

    // Take a slot in the room; blocks if room is full
    DBG("waiting roomSlots (inside=%d,inSession=%d,active=%d)", studentsInside, studentsInSession, sessionActive);
    sem_wait(&roomSlots);
    DBG("acquired roomSlots");
    
    //  **********CRITICAL*************
    sem_wait(&mutex);
    
    // Check if we can start a session
    bool canStartSession = false;
    if (tutorPresent == 0) {
        // Need exactly sessionSize students including this one
        canStartSession = (studentsInside == sessionSize - 1);
    } else {
        // Need sessionSize students already inside; this arriving thread is tutor
        canStartSession = (studentsInside == sessionSize);
    }
    
    // Enter the room
    studentsInside++;
    DBG("entered room, inside=%d", studentsInside);
    
    if (canStartSession) {
        sessionActive = true;
        studentsInSession = studentsInside;  // Everyone currently inside
        if (tutorPresent == 1) {
            sessionTutor = tid;
        }
        if (barrierInitialized) {
            pthread_barrier_destroy(&sessionBarrier);
            barrierInitialized = false;
        }
        pthread_barrier_init(&sessionBarrier, nullptr, studentsInSession);
        barrierInitialized = true;

        pthread_mutex_lock(&printMutex);
        printf("Thread ID: %lu | Status: There are enough students, the study session is starting.\n",
               (unsigned long)tid);
        pthread_mutex_unlock(&printMutex);

        // Close gate so new arrivals block until session ends
        sem_wait(&entryGate);
        
        int toRelease = waitingForSession;
        waitingForSession = 0;
        for (int i = 0; i < toRelease; i++) {
            sem_post(&sessionStarted);
        }
        
        sem_post(&mutex);
        //  **********CRITICAL END*************
    } else {
        // Not enough yet
        pthread_mutex_lock(&printMutex);
        printf("Thread ID: %lu | Status: Only %d students inside, studying individually.\n",
               (unsigned long)tid, studentsInside);
        pthread_mutex_unlock(&printMutex);
        waitingForSession++;
        sem_post(&mutex);

        // Wait for a session to start or time out if it never forms
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 5; // 5 seconds patience to allow staggered arrivals
        int res = sem_timedwait(&sessionStarted, &ts);
        if (res == -1) {
            sem_wait(&mutex);
            if (waitingForSession > 0) waitingForSession--;
            sem_post(&mutex);
        }
    }
}

// Study session starts

void Study::start() {
    // Simulate passing time without additional synchronization.
    sleep(1); // Allow time for groups to form before leaving
}

// LEAVE İC
void Study::leave() {
    pthread_t tid = pthread_self();
      // **********CRITICAL*************
    sem_wait(&mutex);
    
    if (!sessionActive) {
        pthread_mutex_lock(&printMutex);
        printf("Thread ID: %lu | Status: No group study formed while I was waiting, I am leaving.\n",
               (unsigned long)tid);
        pthread_mutex_unlock(&printMutex);
        studentsInside--;
        sem_post(&roomSlots);
        sem_post(&mutex);
        return;
    }

    // Session must be active if we are leaving after start()
    bool isTutor = (tutorPresent == 1) && pthread_equal(tid, sessionTutor);
    sem_post(&mutex);

    if (tutorPresent == 1 && !isTutor) {
        // Wait for tutor to announce via barrier
        pthread_barrier_wait(&sessionBarrier);
        pthread_mutex_lock(&printMutex);
        printf("Thread ID: %lu | Status: I am a student and I am leaving.\n",
               (unsigned long)tid);
        pthread_mutex_unlock(&printMutex);
    } else if (isTutor) {
        // Tutor announces end
        pthread_mutex_lock(&printMutex);
        printf("Thread ID: %lu | Status: Session tutor speaking, the session is over.\n",
               (unsigned long)tid);
        pthread_mutex_unlock(&printMutex);
        pthread_barrier_wait(&sessionBarrier);
    } else {
        pthread_mutex_lock(&printMutex);
        printf("Thread ID: %lu | Status: I am a student and I am leaving.\n",
               (unsigned long)tid);
        pthread_mutex_unlock(&printMutex);
    }
    
    sem_wait(&mutex);
    studentsInSession--;
    studentsInside--;
    DBG("leaving counts: inside=%d inSession=%d", studentsInside, studentsInSession);
    bool isLast = (studentsInSession == 0);
    
    sem_post(&roomSlots);
    
    if (isLast) {
        sessionActive = false;
        if (barrierInitialized) {
            pthread_barrier_destroy(&sessionBarrier);
            barrierInitialized = false;
        }
        pthread_mutex_lock(&printMutex);
        printf("Thread ID: %lu | Status: everybody left.\n",
               (unsigned long)tid);
        pthread_mutex_unlock(&printMutex);
        sem_post(&entryGate); // Allow new arrivals after the room empties
        // Wake any waiters so they can retry next session
        int toRelease = waitingForSession;
        waitingForSession = 0;
        for (int i = 0; i < toRelease; i++) {
            sem_post(&sessionStarted);
        }
    }
    
    sem_post(&mutex);
}

