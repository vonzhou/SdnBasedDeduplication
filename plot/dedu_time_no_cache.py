#!/usr/bin/env python
# _*_ coding:utf-8 _*-

"""
源端去重，服务器端重复查找使用哈希表 对比于  直接利用系统调用判断对应文件是否存在（文件名为指纹）
单位 ms

"""


import numpy as np
import matplotlib.pyplot as plt

N = 10
source_dedu_time = 		(10816.978, 4940.254, 31796.82, 5650.817, 51419.827, 23500.126, 34612.247, 42159.744, 47191.812, 33376.285)

ind = np.arange(N)  # the x locations for the groups
width = 0.35       # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(ind, source_dedu_time, width, color='r', yerr=None)

source_dedu_access = (4780.393, 3905.44, 28772.734, 6748.863, 52602.989, 25577.762, 35731.927, 41362.129, 35621.854, 22756.835)
rects2 = ax.bar(ind + width, source_dedu_access, width, color='y', yerr=None)

# add some text for labels, title and axes ticks
ax.set_ylabel('Backup Time(ms)')
ax.set_title('Backup Time Comparison')
ax.set_xticks(ind + width)
ax.set_xticklabels(('1.18', '2.05', '3.01', '3.02', '4.0.6', '4.0.7', '4.0.8', '4.0.9', '4.1.1', '4.1.2'))

ax.legend((rects1[0], rects2[0]), ('hash table', 'access syscall'))


def autolabel(rects):
    # attach some text labels
    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x() + rect.get_width()/2., 1.05*height,
                '%d' % int(height),
                ha='center', va='bottom')

# autolabel(rects1)
# autolabel(rects2)

plt.show()