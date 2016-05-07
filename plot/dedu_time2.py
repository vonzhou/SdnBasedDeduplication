#!/usr/bin/env python
# _*_ coding:utf-8 _*-

"""
有无SDN控制器缓存情况下 备份窗口的 对比
redis数据集43组
秒



"""


import numpy as np
import matplotlib.pyplot as plt

# 设置显示中文
plt.rcParams['font.sans-serif'] = ['SimSun GB']   #['Hiragino Sans GB']
plt.rcParams['axes.unicode_minus'] = False

N = 37

base_time_10m = (188.13, 98.08, 96.1, 95.14, 95.14, 114.24, 97.46, 98.16, 102.21, 99.1, 101.1, 100.1, 104.64, 102.4, 104.5, 105.7, 99.6, 157.85, 127, 124.9, 117.2, 116.2, 111.2, 129.4, 119.9, 119.5, 117.9, 119, 115.3, 142.1, 120.8, 125, 126.9, 123.1, 124.9, 124.4,  123.8)
sdn_time_10m = (22, 15, 14.7, 14.8, 17.4, 15.2, 15.5, 16.4, 16.5, 15.93, 15.95, 16.2, 16.82, 16.32, 16.45, 15.76, 23.3, 19.4, 20.3, 18.6, 18, 17.5, 20.34, 18.4, 19.44, 18.4, 19, 17.9, 18.2, 22.83, 19.14, 19.63, 20.87, 20.57, 19.37, 20.29, 19.8)

base_time_1000m = (27.5, 12.3, 12.1, 11.9, 14.6, 12.1, 11.9, 12.5, 12.48, 12.17, 12.2, 12.73, 12.68, 12.38, 12.59, 11.91, 19.8, 14.8, 14.97, 14, 13.76, 14.62, 16.4, 14.5, 15, 15.1, 15, 14.4, 14.2, 17.5, 15.15, 15.4, 15.36, 15.43, 15, 14.8, 14.2)
#print len(base_time_1000m)
base_time = (4.5, 0.56, 0.61, 0.58, 0.83, 0.46, 0.44, 0.5, 0.58, 0.51, 0.45, 0.61, 0.51, 0.58, 0.34, 0.47, 2.1, 0.62, 0.66, 0.54, 0.43, 0.41, 0.81, 0.74, 0.73, 0.58, 0.5, 0.51, 0.49, 1.26, 0.55, 0.55, 0.63, 0.63, 0.46, 0.64, 0.52)

sdna_time_10g = (2.25, 0.95, 0.95, 1.31, 1.4, 1.2, 1.1, 1.4, 0.9, 0.73, 0.76, 0.86, 0.76, 0.76, 0.82, 0.77, 0.93, 0.75, 0.82, 0.86, 1.1, 1, 1.04, 1.2, 1.3, 1.1, 0.9, 0.99, 1.1, 1, 0.6, 0.9, 0.87, 0.89, 0.94, 0.88, 0.92)
sdna_time_1000m = (3.32, 2.33, 2.37, 2.62, 2.91, 2.53, 2.5, 3.75, 2.63, 3.1, 2.4, 2.1, 2.34, 2.4, 2.3, 2.2, 3.14, 2.75, 2.84, 2.62, 2.7, 2.5, 2.96, 2.95, 3.55, 3.37, 3.1, 2.88, 2.72, 3.27, 2.58, 2.92, 2.99, 2.96, 3.36, 3, 2.84)

x = np.arange(N)

plt.figure(figsize=(10,8))
plt.plot(x, base_time_100m, label='$base-100m$', linewidth = 3)
plt.plot(x, base_time_1000m, label='$base-1g$', linewidth = 3)
plt.plot(x, base_time, label='$base-10g$', color="red", linewidth=3)
plt.plot(x, sdna_time_10g, label='$sdn-10g$', color='yellow')
plt.plot(x, sdna_time_1000m, label='$sdn-1g')

#yticks = np.linspace(0, 200, num = 10)
yticks = np.arange(0, 200, 10)
plt.yticks(yticks)

# add some text for labels, title and axes ticks
plt.xlabel(u'版本号')
plt.ylabel(u'备份时间 秒')

plt.title(u'备份时间（RDB）')
plt.legend()

plt.gca().yaxis.grid(True)


#ax.set_xticks(ind + width)
#ax.set_xticklabels(('1.18', '2.05', '3.01', '3.02', '4.0.6', '4.0.7', '4.0.8', '4.0.9', '4.1.1', '4.1.2'))

#ax.legend((rects1[0], rects2[0]), ('Source Dedupe', 'With SDN'))

plt.show()

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
