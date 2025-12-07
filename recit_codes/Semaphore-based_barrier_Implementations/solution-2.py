######
''' 
This one is also a correct solution. Instead of using the turnstile system, the last comer releases all threads one-by-one.
'''

import threading

barrier = threading.Semaphore(0)  # global semaphore
barrier2 = threading.Semaphore(0)  # global semaphore
mutex = threading.Lock()  # global mutex
count = 0  # global variable for count
thread_amount = 5

def gameTime():
    
    global count 

    for turn in range(3):
        print(f"Turn: {turn}, I'm {threading.get_ident()}")
        #sleep(0.01)

        mutex.acquire()
        count = count + 1
        if count == thread_amount:
            for i in range(thread_amount):
                barrier.release()
        mutex.release()
        
        barrier.acquire()

        mutex.acquire()
        count -= 1
        if count == 0:
            for i in range(thread_amount):
                barrier2.release()
        mutex.release()
        
        barrier2.acquire()
        print(f"I'm {threading.get_ident()}, I passed the barrier! ")

    
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


'''