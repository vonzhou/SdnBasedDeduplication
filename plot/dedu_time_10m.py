#!/usr/bin/env python
# _*_ coding:utf-8 _*-

import numpy as np
import matplotlib.pyplot as plt

# 设置显示中文
plt.rcParams['font.sans-serif'] = ['SimSun GB']   #['Hiragino Sans GB']
plt.rcParams['axes.unicode_minus'] = False

N = 37

base_time_10m = (188.13, 98.08, 96.1, 95.14, 95.14, 114.24, 97.46, 98.16, 102.21, 99.1, 101.1, 100.1, 104.64, 102.4, 104.5, 105.7, 99.6, 157.85, 127, 124.9, 117.2, 116.2, 111.2, 129.4, 119.9, 119.5, 117.9, 119, 115.3, 142.1, 120.8, 125, 126.9, 123.1, 124.9, 124.4,  123.8)
sdna_time_10m = (22, 15, 14.7, 14.8, 17.4, 15.2, 15.5, 16.4, 16.5, 15.93, 15.95, 16.2, 16.82, 16.32, 16.45, 15.76, 23.3, 19.4, 20.3, 18.6, 18, 17.5, 20.34, 18.4, 19.44, 18.4, 19, 17.9, 18.2, 22.83, 19.14, 19.63, 20.87, 20.57, 19.37, 20.29, 19.8)

print base_time_10m[23]
print sdna_time_10m[23]

x = np.arange(N)

#plt.figure(figsize=(8,6))
plt.plot(x, base_time_10m, label='$baseline-10Mb$', marker="x",color='red', linewidth = 2)
plt.plot(x, sdna_time_10m, label='$sdna-10Mb$', color='blue', linewidth = 2)

# add some text for labels, title and axes ticks
plt.xlabel(u'版本号')
plt.ylabel(u'备份时间（秒）')

plt.legend()

plt.gca().yaxis.grid(True)


plt.show()

