######
''' 
let's try to fix our code for loops
the barrier should be set to its initial value after each iteration of the loop
so that the next iteration can happen safely
----
but again there is a problem, look at the end for the problem description
'''

import threading

barrier = threading.Semaphore(0)  # global semaphore
mutex = threading.Lock()  # global mutex
count = 0  # global variable for count
thread_amount = 5

def gameTime():
    
    global count 

    for turn in range(10):
        print(f"Turn: {turn}, I'm {threading.get_ident()}")
        #sleep(0.01)

        mutex.acquire() # thread5 coems here
        count = count + 1
        mutex.release()

        if count == thread_amount: # thread 4 was here - count value was 5
            barrier.release()
        
        barrier.acquire()
        barrier.release() 

        # We add below part for reusability
        mutex.acquire()
        count -= 1
        mutex.release()

        if count == 0:
            barrier.acquire()
        print(f"I'm {threading.get_ident()}, I passed the barrier!")    
    
t1 = threading.Thread(target = gameTime)
t2 = threading.Thread(target = gameTime)
t3 = threading.Thread(target = gameTime)
t4 = threading.Thread(target = gameTime)
t5 = threading.Thread(target = gameTime)
t1.start()
t2.start()
t3.start()
t4.start()
t5.start()


'''
line 29 and line 40 are problematic
assume we have n threads
say n-1th thread is interrupted just before executing line 29
and then nth thread is scheduled, it passes through the first mutex
now, both threads find that count==n
and both threads will release the barrier 
---
Similar problem might happen at line 40


Note: this problem was there all along in the previous versions
'''