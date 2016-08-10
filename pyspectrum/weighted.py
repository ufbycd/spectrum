'''
Created on 2016年8月9日

@author: chenss
'''
import math

def frange(x, y, jump):
    while x < y:
        yield x
        x += jump

def ARate(freq_list):
    aList = []
    for freq in freq_list:
        ra1 = (12200 ** 2) * (freq ** 4) 
        ra2 = (freq ** 2 + 20.6 ** 2) * ((freq ** 2 + 107.7 ** 2) * (freq ** 2 + 737.9 ** 2)) ** 0.5 * \
        (freq ** 2 + 12200 ** 2)
        ra = ra1 / ra2
        A = 2 + 20 * math.log10(ra)
        aList.append(round(A, 2))
    print(aList)

if __name__ == '__main__':
    freq_list = [freq for freq in frange(312.5, 20000 + 312.5, 312.5)]
    print(freq_list)
    ARate(freq_list)