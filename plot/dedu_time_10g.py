#!/usr/bin/env python
# _*_ coding:utf-8 _*-


import numpy as np
import matplotlib.pyplot as plt

# 设置显示中文
plt.rcParams['font.sans-serif'] = ['SimSun GB']   #['Hiragino Sans GB']
plt.rcParams['axes.unicode_minus'] = False

N = 37

base_time = (4.5, 0.56, 0.61, 0.58, 0.83, 0.46, 0.44, 0.5, 0.58, 0.51, 0.45, 0.61, 0.51, 0.58, 0.34, 0.47, 2.1, 0.62, 0.66, 0.54, 0.43, 0.41, 0.81, 0.74, 0.73, 0.58, 0.5, 0.51, 0.49, 1.26, 0.55, 0.55, 0.63, 0.63, 0.46, 0.64, 0.52)

sdna_time_10g = (2.25, 0.95, 0.95, 1.31, 1.4, 1.2, 1.1, 1.4, 0.9, 0.73, 0.76, 0.86, 0.76, 0.76, 0.82, 0.77, 0.93, 0.75, 0.82, 0.86, 1.1, 1, 1.04, 1.2, 1.3, 1.1, 0.9, 0.99, 1.1, 1, 0.6, 0.9, 0.87, 0.89, 0.94, 0.88, 0.92)
print base_time[23]
print sdna_time_10g[23]

x = np.arange(N)

#plt.figure(figsize=(10,8))
plt.plot(x, base_time, label='$baseline-10Gb$', marker = "x", color="red", linewidth=2)
plt.plot(x, sdna_time_10g, label='$sdna-10Gb$', color='blue', linewidth = 2)

#yticks = np.linspace(0, 200, num = 10)
#yticks = np.arange(0, 1, 10)
#plt.yticks(yticks)

# add some text for labels, title and axes ticks
plt.xlabel(u'版本号')
plt.ylabel(u'备份时间（秒）')

plt.legend()

plt.gca().yaxis.grid(True)


#ax.set_xticks(ind + width)
#ax.set_xticklabels(('1.18', '2.05', '3.01', '3.02', '4.0.6', '4.0.7', '4.0.8', '4.0.9', '4.1.1', '4.1.2'))

#ax.legend((rects1[0], rects2[0]), ('Source Dedupe', 'With SDN'))

plt.show()

