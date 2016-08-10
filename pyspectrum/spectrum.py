#!/usr/bin/env python
# encoding: utf-8

import curses
import time
import random
import signal
import threading

class Spectrum:
    
    QUEUE_SIZE = 100
    
    def __init__(self, char='■'):
        self.char = char
        self.screen = curses.initscr()
        self.screen.border(0)
        self.audioData = []
        self.dataLock = threading.RLock()
        
        self.stopEvent = threading.Event()
        self.jobEvent  = threading.Event()
        signal.signal(signal.SIGINT, self.onStop)
        signal.signal(signal.SIGTERM, self.onStop)

    def onStop(self, signum, frame):
        self.stopEvent.set()
        self.jobEvent.set()
        
    def getAudioDataFromRandom(self):
        while not self.stopEvent.isSet():
            dataList = []
            for _ in range(32):
                dataList.append(random.Random().randint(0, 32))
            with self.dataLock:
                self.audioData = dataList
            self.jobEvent.set()
            time.sleep(0.04)
            
        
    def getAudioDataFromSerial(self, device="/dev/ttyUSB0"):
        pass
    
    def getAudioDataFromSoundCar(self):
        pass
    
    def updateScreen(self, data_list):
        self.screen.clear()
        for i, d in enumerate(data_list):
            self.screen.addstr(1 + i, 1, self.char * d)
        self.screen.refresh()
        
    def run(self):
        t = threading.Thread(target=self.getAudioDataFromRandom)
        t.start()
        while not self.stopEvent.isSet():
            self.jobEvent.wait()
            if self.stopEvent.isSet():
                break
            with self.dataLock:
                data_list = self.audioData.copy()
            self.updateScreen(data_list)
            
        self.screen.getch()
        curses.endwin()
    
if __name__ == '__main__':
    spectrum = Spectrum()
    spectrum.run()
    
