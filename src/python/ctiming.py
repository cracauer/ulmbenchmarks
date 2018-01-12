import signal
import time

class Stopit(Exception):
    True

def handler(signum, strackframe):
    raise Stopit()

signal.signal(signal.SIGALRM, handler)

def onerun(description, arg1, arg2, tocall):
    signal.setitimer(signal.ITIMER_REAL, 0.3)
    count = 0
    start_time = time.time()
    try:
        while True:
            res = tocall(arg1, arg2)
            count += 1
    except Stopit as e:
        True
    time_passed = time.time() - start_time
    print("%10.3f nsec/call '%s'" % (time_passed * 1000000000 / count, description))
