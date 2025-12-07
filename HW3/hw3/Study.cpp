#include "Study.h"
#include <unistd.h>
#include <cstdlib>
#include <stdexcept>
#include <cstdio>
#include <ctime>
#include <cerrno>

#define DEBUG 0
#define DBG(fmt, ...) \
    do { if (DEBUG) printf("[DBG] tid=%lu " fmt "\n", (unsigned long)pthread_self(), ##__VA_ARGS__); } while (0)

static thread_local bool leftEarly = false;  // Tracks threads that left before a session formed

// Constructor
Study::Study(int sessionSize, int tutorPresent) 
    : sessionSize(sessionSize), 
      tutorPresent(tutorPresent),
      studentsInside(0),
      studentsInSession(0),
      waitingForSession(0),
      sessionActive(false),
      tutorHasAnnounced(false) {
    
    // Validate inputs
    if (sessionSize < 1) {
        throw std::runtime_error("sessionSize must be a positive integer");
    }
    if (tutorPresent != 0 && tutorPresent != 1) {
        throw std::runtime_error("tutorPresent must be either 0 or 1");
    }
    
    // Initialize semaphores
    int capacity = sessionSize + (tutorPresent == 1 ? 1 : 0);
    sem_init(&mutex, 0, 1);                // Binary semaphore for mutex
    sem_init(&roomSlots, 0, capacity);     // Counting semaphore for room capacity
    sem_init(&sessionStarted, 0, 0);       // Released when session starts
    sem_init(&tutorAnnounced, 0, 0);       // Released when tutor announces
    sem_init(&departureBarrier, 0, 1);     // Unused barrier placeholder
}

// Destructor
Study::~Study() {
    sem_destroy(&mutex);
    sem_destroy(&roomSlots);
    sem_destroy(&sessionStarted);
    sem_destroy(&tutorAnnounced);
    sem_destroy(&departureBarrier);
}

// Student arrives at the study center
void Study::arrive() {
    pthread_t tid = pthread_self();
    
    // Print arrival message
    printf("Thread ID: %lu | Status: Arrived at the IC.\n", (unsigned long)tid);
    
    // Take a slot in the room (blocks if room is full while a session is running)
    DBG("waiting roomSlots (inside=%d,inSession=%d,active=%d)", studentsInside, studentsInSession, sessionActive);
    sem_wait(&roomSlots);
    DBG("acquired roomSlots");
    
    // Enter critical section
    sem_wait(&mutex);
    
    // Check if we can start a session (before increment)
    int capacity = sessionSize + (tutorPresent == 1 ? 1 : 0);
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
        tutorHasAnnounced = false;
        if (tutorPresent == 1) {
            sessionTutor = tid;
        }
        printf("Thread ID: %lu | Status: There are enough students, the study session is starting.\n",
               (unsigned long)tid);
        
        // Release start signal to all other participants
        int toRelease = waitingForSession;
        waitingForSession = 0;
        while (toRelease-- > 0) {
            sem_post(&sessionStarted);
            DBG("sessionStarted post, remaining waiters=%d", toRelease);
        }
        
        sem_post(&mutex);
    } else {
        // Not enough yet
        printf("Thread ID: %lu | Status: Only %d students inside, studying individually.\n",
               (unsigned long)tid, studentsInside);
        // This thread will wait for session start
        waitingForSession++;
        DBG("waiting sessionStarted (waiters=%d)", waitingForSession);
        sem_post(&mutex);
        
        // Wait for session start, but allow eventual timeout to avoid starvation
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 15; // generous timeout to allow group formation
        int r = sem_timedwait(&sessionStarted, &ts);
        if (r == -1 && errno == ETIMEDOUT) {
            sem_wait(&mutex);
            if (waitingForSession > 0) waitingForSession--;
            studentsInside--;
            sem_post(&roomSlots);
            sem_post(&mutex);
            printf("Thread ID: %lu | Status: No group study formed while I was waiting, I am leaving.\n",
                   (unsigned long)tid);
            leftEarly = true;
            return;
        }
        DBG("passed sessionStarted");
        
        // Re-check session status; if no session formed, leave
        sem_wait(&mutex);
        if (!sessionActive) {
            studentsInside--;
            sem_post(&roomSlots);
            sem_post(&mutex);
            printf("Thread ID: %lu | Status: No group study formed while I was waiting, I am leaving.\n",
                   (unsigned long)tid);
            leftEarly = true;
            return;
        }
        sem_post(&mutex);
    }
}

// Study session starts
// This method is declared but not implemented - test cases will provide the implementation
void Study::start() {
    // Empty - implementation provided by test cases
}

// Student leaves the study center
void Study::leave() {
    pthread_t tid = pthread_self();
    
    sem_wait(&mutex);
    
    // If this thread already left earlier (no session), do nothing
    if (leftEarly) {
        sem_post(&mutex);
        leftEarly = false;
        return;
    }

    // Session must be active if we are leaving after start()
    bool isTutor = (tutorPresent == 1) && pthread_equal(tid, sessionTutor);
    
    if (tutorPresent == 1 && !isTutor && !tutorHasAnnounced) {
        // Wait for tutor to announce before leaving
        sem_post(&mutex);
        DBG("student waiting tutorAnnounced");
        sem_wait(&tutorAnnounced);
        DBG("student passed tutorAnnounced");
        sem_wait(&mutex);
    } else if (isTutor) {
        // Tutor announces end
        printf("Thread ID: %lu | Status: Session tutor speaking, the session is over.\n",
               (unsigned long)tid);
        tutorHasAnnounced = true;
        for (int i = 0; i < studentsInSession - 1; i++) {
            sem_post(&tutorAnnounced);
            DBG("tutor posts tutorAnnounced %d/%d", i+1, studentsInSession-1);
        }
    }
    
    if (!isTutor) {
        printf("Thread ID: %lu | Status: I am a student and I am leaving.\n",
               (unsigned long)tid);
    }
    
    studentsInSession--;
    studentsInside--;
    DBG("leaving counts: inside=%d inSession=%d", studentsInside, studentsInSession);
    bool isLast = (studentsInSession == 0);
    int capacity = sessionSize + (tutorPresent == 1 ? 1 : 0);
    
    if (isLast) {
        sessionActive = false;
        tutorHasAnnounced = false;
        printf("Thread ID: %lu | Status: All students have left, the new students can come.\n",
               (unsigned long)tid);
        // Free all slots for next session
        for (int i = 0; i < capacity; i++) {
            sem_post(&roomSlots);
        }
        // Release any threads waiting for a session that will not start
        while (waitingForSession > 0) {
            sem_post(&sessionStarted);
            waitingForSession--;
        }
    }
    
    sem_post(&mutex);
}

