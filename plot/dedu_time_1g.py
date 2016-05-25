#!/usr/bin/env python
# _*_ coding:utf-8 _*-


import numpy as np
import matplotlib.pyplot as plt

# 设置显示中文
plt.rcParams['font.sans-serif'] = ['SimSun GB']   #['Hiragino Sans GB']
plt.rcParams['axes.unicode_minus'] = False

N = 37

base_time_1000m = (27.5, 12.3, 12.1, 11.9, 14.6, 12.1, 11.9, 12.5, 12.48, 12.17, 12.2, 12.73, 12.68, 12.38, 12.59, 11.91, 19.8, 14.8, 14.97, 14, 13.76, 14.62, 16.4, 14.5, 15, 15.1, 15, 14.4, 14.2, 17.5, 15.15, 15.4, 15.36, 15.43, 15, 14.8, 14.2)

sdna_time_1000m = (3.32, 2.33, 2.37, 2.62, 2.91, 2.53, 2.5, 3.75, 2.63, 3.1, 2.4, 2.1, 2.34, 2.4, 2.3, 2.2, 3.14, 2.75, 2.84, 2.62, 2.7, 2.5, 2.96, 2.95, 3.55, 3.37, 3.1, 2.88, 2.72, 3.27, 2.58, 2.92, 2.99, 2.96, 3.36, 3, 2.84)
print base_time_1000m[23]
print sdna_time_1000m[23]
x = np.arange(N)

#plt.figure(figsize=(10,8))
plt.plot(x, base_time_1000m, label='$baseline-1Gb$',color='red' , linewidth = 2)
plt.plot(x, sdna_time_1000m, label='$sdna-1Gb$', color='blue', linewidth = 2)

#yticks = np.linspace(0, 200, num = 10)

# add some text for labels, title and axes ticks
plt.xlabel(u'版本号')
plt.ylabel(u'备份时间（秒）')

#plt.title(u'备份时间（RDB）')
plt.legend()

plt.gca().yaxis.grid(True)
plt.show()

