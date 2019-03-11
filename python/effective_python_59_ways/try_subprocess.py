#!/usr/bin/python

from time import time, sleep
import subprocess

class TestPopen():
    def test0(self):
        proc = subprocess.Popen(["echo", "aaaa"],
                stdout=subprocess.PIPE)
        out, err = proc.communicate()
        print("p said: " + out.decode("UTF-8"))

    def test1(self):
        proc = subprocess.Popen(["sleep", "0.1"])
        while proc.poll() is None:
            print(time())
            print("do some of my own work")
        print("child exits as %d" % proc.poll())

    def run_sleep(self, period):
        return subprocess.Popen(["sleep", str(period)])
    
    def test3(self):
        processes = []
        for _ in range(100):
            processes.append(self.run_sleep(0.2))

        start = time()
        for p in processes:
            p.communicate()
        print("cost %f", (time() - start))
    
    def test4(self):
        start = time()
        sleep(1)
        print("cost %f", (time() - start))

def run_sleep(period):
    proc = subprocess.Popen(["sleep", str(period)])
    return proc

def test3():
    start = time()
    processes = []
    print(list(range(10)))
    for _ in range(10):
        processes.append(run_sleep(0.2))

    for p in processes:
        p.communicate()
    print("cost %f", (time() - start))

def test5():
    proc = subprocess.Popen(["ls"],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    proc.stdout.flush()
    out, err = proc.communicate()
    proc = subprocess.Popen(["wc", "-l"],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    proc.stdin.write(out)
    proc.stdin.flush()
    out, err = proc.communicate()

