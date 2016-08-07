#!/usr/bin/env python

import curses
import time
import random
import signal
import threading


def update(screen, data_list):
    for i, d in enumerate(data_list):
        screen.addstr(1 + i, 1, '■' * d)
        
def randomData():
    dataList = []
    for _ in range(32):
        dataList.append(random.Random().randint(0, 32))
    return dataList

def sq(screen, event):
    for _ in range(300):
        if event.isSet():
            break
        screen.clear()
        update(screen, randomData())
        screen.refresh()
        time.sleep(0.04)
        

class Spectrum:
    
    def __init__(self, char='■'):
        self.char = char
        self.screen = curses.initscr()
        self.screen.border(0)
        
        self.stopEvent = threading.Event()
        signal.signal(signal.SIGINT, self.onStop)
        signal.signal(signal.SIGTERM, self.onStop)

    def onStop(self, signum, frame):
        self.stopEvent.set()
        
    def run(self):
        sq(self.screen, self.stopEvent)
        self.screen.getch()
        curses.endwin()
    
if __name__ == '__main__':
    spectrum = Spectrum()
    spectrum.run()
    
