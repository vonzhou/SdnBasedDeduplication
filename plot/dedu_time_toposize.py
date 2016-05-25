#!/usr/bin/env python
# _*_ coding:utf-8 _*-


import numpy as np
import matplotlib.pyplot as plt

# 设置显示中文
plt.rcParams['font.sans-serif'] = ['SimSun GB']   #['Hiragino Sans GB']
plt.rcParams['axes.unicode_minus'] = False

N = 37

# 1Gb/s  ,  full cache ,


linear_15 = (4.07, 2.71, 2.59, 2.69, 3.84, 2.88, 2.91, 3.05, 3.05, 2.72, 2.51, 2.59, 2.68, 2.55, 2.6, 2.46, 3.78, 3.08, 3.34, 2.93, 2.76, 2.6, 3.38, 2.74, 3.13, 2.81, 2.96, 2.95, 3.24, 4.43, 3.38, 3.45, 3.82, 3.66, 3.56, 3.38, 3.3)

# linear_10 = (3.32, 2.33, 2.37, 2.62, 2.91, 2.53, 2.5, 3.75, 2.63, 3.1, 2.4, 2.1, 2.34, 2.4, 2.3, 2.2, 3.14, 2.75, 2.84, 2.62, 2.7, 2.5, 2.96, 2.95, 3.55, 3.37, 3.1, 2.88, 2.72, 3.27, 2.58, 2.92, 2.99, 2.96, 3.36, 3, 2.84)

linear_10 = (3.58, 2.35, 2.32, 3.43, 2.86, 2.57, 2.47, 2.69, 2.64, 2.56, 2.89, 2.94, 2.69, 2.5, 2.48, 2.51, 3.47, 3.71, 3.1, 2.85, 2.78, 2.75, 3.1, 2.85, 3.16, 2.5, 2.59, 2.5, 2.46, 3.16, 2.6, 2.66, 2.79, 2.79, 2.67, 2.73, 2.74)


linear_20 = (5.38, 2.75, 2.83, 2.84, 3.75, 2.69, 2.49, 2.79, 2.69, 2.63, 2.62, 2.54, 2.81, 2.6, 2.62, 2.51, 4.4, 3.23, 3.52, 3.02, 2.66, 2.58, 3.65, 2.75, 3.1, 2.81, 2.98, 2.81, 2.7, 3.87, 2.86, 2.95, 3.16, 3.15, 2.99, 3.25,2.97)


x = np.arange(N)

#plt.figure(figsize=(10,8))
plt.plot(x, linear_10, label='$linear10$',color='red' , linewidth = 2)
#plt.plot(x, linear_15, label='$linear15$', color='black', linewidth = 2)
plt.plot(x, linear_20, label='$linear20$', color='green', linewidth = 2)

#yticks = np.linspace(0, 200, num = 10)

# add some text for labels, title and axes ticks
plt.xlabel(u'版本号')
plt.ylabel(u'备份时间（秒）')

#plt.title(u'备份时间（RDB）')
plt.legend()

plt.gca().yaxis.grid(True)
plt.show()

