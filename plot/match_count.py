#!/usr/bin/env python
# _*_ coding:utf-8 _*-

"""
有无SDN控制器缓存情况下 备份窗口的 对比
redis数据集43组
秒



"""


import numpy as np
import matplotlib.pyplot as plt
import matplotlib
# 设置显示中文
#plt.rcParams['font.sans-serif'] = ['sans-']   #['Hiragino Sans GB']
#plt.rcParams['axes.unicode_minus'] = False

#matplotlib.rc('font', family='sans-serif')
#matplotlib.rc('font', serif='Helvetica Neue')
#matplotlib.rc('text', usetex='false')
#matplotlib.rcParams.update({'font.size': 22})
matplotlib.rcParams['font.sans-serif'] = ['SemiHei'] #指定默认字体
matplotlib.rcParams['axes.unicode_minus'] = False #解决保存图像是负号'-'显示为方块的问题
print matplotlib.get_configdir()
N = 37

match_count = (392, 785, 1178, 1571, 1971, 2371, 2771, 3171, 3571, 3971, 4371, 4780, 5189, 5603, 6017, 6431, 6904, 7382, 7860, 8338, 8816, 9294, 9775, 10256, 10737, 11219, 11701, 12183, 12665, 13176, 13687, 14199, 14711, 15223, 15735, 16247, 16759)

match_count_delta = []
start = 0
for x in match_count:
    match_count_delta.append(x - start)
    start = x


x = np.arange(N)

#plt.figure(figsize=(10,8))
plt.plot(x, match_count_delta, linewidth = 2)
#fig1 = plt.figure(facecolor='black')

# add some text for labels, title and axes ticks
plt.xlabel(u'版本号')
plt.ylabel(u'数据包个数')

#plt.legend()

plt.gca().yaxis.grid(True)


#ax.set_xticks(ind + width)
#ax.set_xticklabels(('1.18', '2.05', '3.01', '3.02', '4.0.6', '4.0.7', '4.0.8', '4.0.9', '4.1.1', '4.1.2'))

#ax.legend((rects1[0], rects2[0]), ('Source Dedupe', 'With SDN'))

plt.show()
#fig1.savefig('whatever.png', facecolor=fig1.get_facecolor(), edgecolor='none')
#plt.savefig('myfilename.png')
