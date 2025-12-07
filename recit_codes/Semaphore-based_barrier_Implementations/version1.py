######
''' 
not working solution
deadlock is present
only the first thread is able to pass the barrier
'''


import threading
from time import sleep

barrier = threading.Semaphore(0)  # global semaphore
mutex = threading.Lock()  # global mutex
count = 0  # global variable for count
thread_amount = 5

def gameTime():
    global count 

    print(f"I'm {threading.get_ident()}, pretending to do some tasks but actually sleeping :)")
    sleep(0.01)

    # Barrier.wait()
    mutex.acquire()
    count = count + 1
    mutex.release()

    if count == thread_amount:      # 5 threads
        barrier.release()
    barrier.acquire() # 4 threads wait here


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

