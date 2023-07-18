import sys
import time

idx = 0
while True:
    print("test", idx)
    sys.stdout.flush()
    time.sleep(2)
    idx += 1
