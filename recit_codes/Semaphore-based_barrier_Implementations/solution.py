######
''' 
finally, a solution :)
'''

import threading

barrier1 = threading.Semaphore(0)  # global semaphore
barrier2 = threading.Semaphore(1)  # global semaphore
mutex = threading.Lock()  # global mutex
count = 0  # global variable for count
thread_amount = 5

def gameTime():
    
    global count

    for turn in range(10):
        print(f"Turn: {turn}, I'm {threading.get_ident()}")
        #sleep(0.01)

        mutex.acquire()
        count = count + 1
        if count == thread_amount: # 1 thread - count 5
            barrier2.acquire()
            barrier1.release()
        mutex.release()
        
        barrier1.acquire() # 4 threads waiting
        barrier1.release()  

        mutex.acquire()
        count -= 1
        if count == 0:
            barrier1.acquire()
            barrier2.release()
        mutex.release()

        barrier2.acquire() # 4 threades were waiting here
        barrier2.release()
        #print(f"I'm {threading.get_ident()}, I passed the barrier! ")
    
    
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

