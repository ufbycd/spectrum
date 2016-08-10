#!/usr/bin/env python
# encoding: utf-8

'''
Created on 2016年8月10日

@author: chenss
'''

OCTAVE_CENTRE_FREQ = (20, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 
                      250, 315, 400, 500, 630, 800, 1 * 1000, 1.25 * 1000, 1.6 * 1000, 2 * 1000, 
                      2.5 * 1000, 3.15 * 1000, 4 * 1000, 5 * 1000, 6.3 * 1000, 8 * 1000, 10 * 1000, 
                      12.5 * 1000, 16 * 1000, 20 * 1000)
MY_FREQS = [40, 80, 120, 160, 200, 240, 280, 320, 360, 400, 440, 480, 520, 560, 600, 640, 680, 720, 767, 825, 887, 954, 1025, 1102, 1185, 1274, 1370, 1473, 1584, 1703, 1831, 1968, 2116, 2275, 2446, 2630, 2828, 3040, 3269, 3515, 3779, 4063, 4368, 4696, 5049, 5428, 5836, 6275, 6746, 7253, 7798, 8384, 9014, 9692, 10420, 11203, 12044, 12949, 13922, 14968, 16093, 17302, 18602, 20000]

def frange(x, y, jump):
    while x < y:
        yield x
        x += jump


def octaveFreq():
    base = 1
    f0 = 400
    i0 = 10
    f = lambda base, i: f0 * base ** i
    for dx in frange(0, 2, 0.000001):
        base = 1 + dx
        freqi = f(base, 64 - i0)
        if freqi >= 20000:
            print(base, freqi)
            break
    
    freqs = [int(f(base, i)) for i in range(64 - i0 + 1)]
    print(len(freqs))
    print(freqs)
    for i in range(1, len(freqs)):
        if freqs[i] - freqs[i - 1] < 40:
            freqs[i] = freqs[i - 1] + 40
    print(freqs)
            
            
        
def octaveEdge(octave_freqs):
    edges = []
    for i in range(1, len(octave_freqs)):
        freqLow = octave_freqs[i]
        freqHigh = octave_freqs[i - 1]
        edge = freqLow + ((freqHigh - freqLow) / 3)
        edges.append(int(edge))
    return edges

if __name__ == '__main__':
    octaveFreq()