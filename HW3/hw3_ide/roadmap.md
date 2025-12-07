# Project Roadmap: Study Session Synchronization (PA3)

## Project Overview
Implement a `Study` class that manages concurrent study sessions using pthreads and semaphores. Students arrive, wait for enough peers to form a group study session, participate in the session, and then leave.

## Key Requirements (from test files and sample runs)
- **Parameters:**
  - `sessionSize`: Minimum number of students needed to form a group study session
  - `tutorPresent`: Boolean (0 or 1) indicating if a tutor is present
  - `studentNum`: Total number of student threads to create

- **Methods to implement:**
  - `arrive()`: Called when a student arrives at the study center
  - `start()`: Called when a study session begins
  - `leave()`: Called when a student leaves

- **Expected behavior:**
  - Students can study individually if there aren't enough students yet
  - When `sessionSize` students arrive, a group study session starts
  - If tutor is present, tutor speaks and ends the session
  - Students leave after the session ends
  - Students can leave if no group study forms while waiting

## Implementation Roadmap

### Phase 1: Basic Structure Setup
**Goal:** Create the basic class structure and header file

1. **Create `Study.h` header file**
   - Define the `Study` class
   - Declare private member variables:
     - `int sessionSize` - minimum students for group study
     - `int tutorPresent` - whether tutor is available
     - Semaphores and mutexes (to be determined in Phase 2)
     - Counters for tracking students
   - Declare public methods:
     - Constructor: `Study(int sessionSize, int tutorPresent)`
     - `void arrive()`
     - `void start()`
     - `void leave()`
   - Include necessary headers: `<semaphore.h>`, `<pthread.h>`, `<iostream>`

2. **Create `Study.cpp` implementation file**
   - Implement constructor (initialize semaphores and variables)
   - Implement stub methods (empty bodies for now)

3. **Test compilation**
   - Modify Makefile to compile Study.cpp
   - Ensure basic structure compiles without errors

**Reference:** Basic structure similar to `study_test.cpp` and `study_test2.cpp` in `/home/ide.yilmaz/HW3/hw3/`

---

### Phase 2: Understanding Synchronization Requirements
**Goal:** Analyze the synchronization patterns needed

1. **Identify synchronization needs:**
   - **Barrier pattern:** Students wait until `sessionSize` students arrive
   - **Mutex protection:** Protect shared counters and state
   - **Condition signaling:** Signal when session starts/ends
   - **Tutor coordination:** If tutor present, tutor must speak before students leave

2. **Study barrier implementations:**
   - Review reusable barrier patterns from `recit_codes/Semaphore-based_barrier_Implementations/`
   - Understand the two-phase barrier pattern (arrival and departure)
   - Note: The barrier needs to be reusable for multiple sessions

**Reference Code:**
- Barrier pattern concepts from `/home/ide.yilmaz/recit_codes/Semaphore-based_barrier_Implementations/solution.py`
- Reusable barrier ideas from `/home/ide.yilmaz/recit_codes/Semaphore-based_barrier_Implementations/reusable-1.py` and `reusable-2.py`

**Citation format when using:**
```cpp
// Barrier synchronization pattern adapted from 
// recit_codes/Semaphore-based_barrier_Implementations/solution.py
```

---

### Phase 3: Implement Basic Student Arrival (arrive())
**Goal:** Handle student arrival and individual study

1. **Add member variables:**
   ```cpp
   sem_t mutex;              // Protect shared state
   sem_t barrier1;           // First barrier for arrival
   sem_t barrier2;           // Second barrier for departure
   int studentsInside;       // Current count of students
   int studentsInSession;    // Students in current session
   bool sessionActive;       // Is a session currently running
   ```

2. **Implement arrive() method:**
   - Lock mutex
   - Increment `studentsInside`
   - Print status: "Arrived at the IC"
   - If `studentsInside < sessionSize`:
     - Print: "Only X students inside, studying individually"
     - Unlock mutex
     - Wait for barrier (students wait here)
   - If `studentsInside == sessionSize`:
     - Set `sessionActive = true`
     - Print: "There are enough students, the study session is starting"
     - Signal barrier to wake waiting students
   - Unlock mutex

3. **Test with small number of students**
   - Test with students < sessionSize (should study individually)
   - Test with students == sessionSize (should form session)

**Reference:** Basic mutex and semaphore usage from pthread examples

---

### Phase 4: Implement Session Start (start())
**Goal:** Coordinate the study session

1. **Implement start() method:**
   - Wait at barrier (all students synchronize here)
   - Once barrier passed, check if in active session
   - If tutor present:
     - Print: "Session tutor speaking, the session is over"
     - (Tutor logic - may need separate handling)
   - If no tutor:
     - Session continues (students study together)
     - Need to determine how session ends without tutor

2. **Handle barrier synchronization:**
   - Use reusable barrier pattern
   - All students must reach barrier before proceeding
   - After session, reset barrier for next session

**Reference Code:**
```cpp
// Reusable barrier pattern adapted from
// recit_codes/Semaphore-based_barrier_Implementations/reusable-2.py
// Modified for C++ semaphores and pthreads
```

---

### Phase 5: Implement Student Departure (leave())
**Goal:** Handle students leaving after session

1. **Implement leave() method:**
   - Lock mutex
   - Decrement `studentsInside`
   - Decrement `studentsInSession`
   - Print: "I am a student and I am leaving"
   - If `studentsInSession == 0`:
     - Set `sessionActive = false`
     - Print: "All students have left, the new students can come"
     - Signal barrier2 to allow next session
   - Unlock mutex
   - Pass through departure barrier

2. **Handle students leaving without session:**
   - Students who don't form a session should be able to leave
   - Print: "No group study formed while I was waiting, I am leaving"
   - Need mechanism to detect when waiting students should give up

**Reference:** Thread cleanup and barrier reset patterns

---

### Phase 6: Implement Reusable Barrier Pattern
**Goal:** Make the barrier reusable for multiple sessions

1. **Two-phase barrier implementation:**
   - **Phase 1 (Arrival):**
     - Students increment counter
     - Last student signals barrier1
     - All students pass barrier1
   - **Phase 2 (Departure):**
     - Students decrement counter
     - Last student resets barrier1 and signals barrier2
     - All students pass barrier2

2. **Add barrier reset logic:**
   - After session ends, reset counters
   - Prepare for next group of students

**Reference Code:**
```cpp
// Reusable barrier implementation based on
// recit_codes/Semaphore-based_barrier_Implementations/solution.py
// Adapted for C++ semaphores (sem_wait/sem_post instead of acquire/release)
```

---

### Phase 7: Handle Tutor Logic
**Goal:** Implement tutor behavior when tutorPresent == 1

1. **Tutor coordination:**
   - If tutor present, tutor must "speak" during session
   - Tutor speaking signals end of session
   - Students wait for tutor to finish before leaving

2. **Implementation approach:**
   - Add semaphore for tutor signaling: `sem_t tutorSemaphore`
   - When session starts and tutor present:
     - Tutor thread (or designated student) prints "Session tutor speaking, the session is over"
     - Signal tutor semaphore
   - Students wait for tutor signal before leaving

**Note:** Need to determine if tutor is a separate thread or handled by one of the students

---

### Phase 8: Handle Edge Cases
**Goal:** Handle all edge cases from sample runs

1. **Students leaving without session:**
   - If student waits too long without session forming
   - Need timeout or detection mechanism
   - Print: "No group study formed while I was waiting, I am leaving"

2. **Multiple sessions:**
   - After one session ends, new students can form another session
   - Barrier must be properly reset

3. **Race conditions:**
   - Ensure all shared state access is protected by mutex
   - Test with various thread scheduling

---

### Phase 9: Testing and Debugging
**Goal:** Verify correctness against sample runs

1. **Test cases to verify:**
   - `study_test_4_4_1.txt`: 4 students, sessionSize=4, tutor=1
   - `study_test_4_5_0.txt`: 4 students, sessionSize=5, tutor=0
   - `study_test_12_4_1.txt`: 12 students, sessionSize=4, tutor=1
   - `study_test2_9_2_1.txt`: 9 students, sessionSize=2, tutor=1

2. **Compare output:**
   - Match thread IDs and status messages
   - Verify correct ordering of events
   - Check for deadlocks or race conditions

3. **Debug common issues:**
   - Deadlocks: Check semaphore order
   - Race conditions: Verify all shared access is protected
   - Incorrect counts: Double-check increment/decrement logic

---

### Phase 10: Code Cleanup and Documentation
**Goal:** Finalize code with proper citations

1. **Add comments:**
   - Document each semaphore's purpose
   - Explain barrier synchronization logic
   - Add citations for borrowed code patterns

2. **Citation format:**
   ```cpp
   // This code block is taken from 
   // recit_codes/Semaphore-based_barrier_Implementations/solution.py
   // Adapted for C++ semaphores
   ```

3. **Verify Makefile:**
   - Ensure proper compilation flags
   - Include all necessary source files
   - Test clean build

---

## Implementation Notes

### Synchronization Primitives Needed:
- `sem_t mutex` - Protect shared variables
- `sem_t barrier1` - First barrier (arrival synchronization)
- `sem_t barrier2` - Second barrier (departure synchronization)
- `sem_t tutorSemaphore` - Coordinate tutor (if needed)

### Shared Variables:
- `int studentsInside` - Current students in study center
- `int studentsInSession` - Students in active session
- `bool sessionActive` - Is session currently running
- `int sessionSize` - Required students for session
- `int tutorPresent` - Tutor availability flag

### Key Synchronization Patterns:
1. **Reusable Barrier:** Students wait until enough arrive, then synchronize departure
2. **Mutex Protection:** All shared variable access must be protected
3. **Conditional Waiting:** Students wait for session formation

---

## Student-Friendly Implementation Tips

1. **Start Simple:** Begin with basic mutex protection, then add barriers
2. **Test Incrementally:** Test each phase before moving to next
3. **Print Debugging:** Add print statements to track thread execution
4. **Use Sample Runs:** Compare your output character-by-character with expected output
5. **Common Mistakes:**
   - Forgetting to unlock mutex (deadlock)
   - Wrong semaphore order (deadlock)
   - Race conditions on counters (missing mutex protection)
   - Not resetting barrier properly (stuck threads)

---

## File Structure
```
hw3/
├── Study.h          (Header file - class declaration)
├── Study.cpp        (Implementation file)
├── study_test.cpp   (Test file 1 - provided)
├── study_test2.cpp  (Test file 2 - provided)
└── Makefile         (Build configuration)
```

---

## References and Citations

When using code from recit_codes, cite as follows:

1. **Barrier patterns:**
   - Source: `recit_codes/Semaphore-based_barrier_Implementations/solution.py`
   - Source: `recit_codes/Semaphore-based_barrier_Implementations/reusable-1.py`
   - Source: `recit_codes/Semaphore-based_barrier_Implementations/reusable-2.py`

2. **Thread creation patterns:**
   - Source: `recit_codes/example1.c` through `example6.c` (for basic pthread usage)

3. **General pthread/semaphore concepts:**
   - Source: Course recitation materials in `recit_codes/`

---

## Estimated Timeline (Student Perspective)

- **Phase 1-2:** 2-3 hours (Setup and understanding)
- **Phase 3-4:** 3-4 hours (Basic arrival and session)
- **Phase 5-6:** 3-4 hours (Departure and barrier)
- **Phase 7:** 2-3 hours (Tutor logic)
- **Phase 8:** 2-3 hours (Edge cases)
- **Phase 9:** 3-4 hours (Testing and debugging)
- **Phase 10:** 1-2 hours (Cleanup)

**Total:** ~16-23 hours of focused work

---

## Next Steps

1. Start with Phase 1: Create basic file structure
2. Read and understand barrier implementations in recit_codes
3. Implement one method at a time
4. Test frequently with provided test files
5. Compare output with sample runs after each major phase

Good luck! 🚀

