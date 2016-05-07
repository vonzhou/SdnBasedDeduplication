#!/usr/bin/env python
# _*_ coding:utf-8 _*-

"""
利用bash数据集
测试的在不同缓存大小情况下 备份窗口的 
13组
秒
总共4411个文件(指纹)
600K  --  492  -- 0.12

(29.494149, 25.084727, 26.332677, 35.104802, 35.481014, 34.160482, 37.622102, 57.645350, 53.886600, 49.404416, 111.502371, 110.461414, 109.648958)

800K -- 1991  --  0.45
(23.121807, 19.900952, 15.985187, 19.947727, 20.186662, 26.122624, 27.628963, 42.344555, 29.986725, 26.321093, 71.705738, 78.814412, 80.843212)


无限制 -- 4411
前面测过
(23.92715, 19.691256, 16.342635, 22.180061, 20.652163, 26.709741, 25.962967, 41.927505, 32.799218, 28.053388, 80.862378, 83.89176, 90.669632)
问题：随着缓存的增大，效果并非线性提高

"""


import numpy as np
import matplotlib.pyplot as plt

N = 13

ind = np.arange(N)  # the x locations for the groups
width = 0.25       # the width of the bars

fig, ax = plt.subplots()

source_dedu_time= (97.022823, 96.20613, 67.744214, 78.829066, 62.570538, 92.620999, 77.010950, 147.090866, 89.583345, 80.134119, 211.407158, 148.663094, 151.183888)
rects1 = ax.bar(ind - width, source_dedu_time, width, color='r', yerr=None, align='center')

# 1
source_dedu_sdn_time1 =(29.494149, 25.084727, 26.332677, 35.104802, 35.481014, 34.160482, 37.622102, 57.645350, 53.886600, 49.404416, 111.502371, 110.461414, 109.648958)
rects2 = ax.bar(ind, source_dedu_sdn_time1, width, color='y', yerr=None, align='center')

# 2
source_dedu_sdn_time2 = (23.121807, 19.900952, 15.985187, 19.947727, 20.186662, 26.122624, 27.628963, 42.344555, 29.986725, 26.321093, 71.705738, 78.814412, 80.843212)
rects3 = ax.bar(ind + width, source_dedu_sdn_time2, width, color='b', yerr=None, align='center')

# add some text for labels, title and axes ticks
ax.set_ylabel('Backup Time(s)')
ax.set_xlabel('bash version')
ax.set_title('Backup Time Comparison')
ax.set_xticks(ind)
ax.set_xticklabels(('1.14.7', '2.0', '2.01', '2.02','2.03','2.04','2.05','3.0', '3.1','3.2','4.0', '4.1', '4.2'))



ax.legend(('Base', 'cache-0.12', 'cache-0.45'), loc='upper center',
          fancybox=True, shadow=True, ncol=5)

# ax.legend( ('Base', 'SDN cache-bf5k', 'SDN cache-bf50k'), loc='down center', ncol=3)


def autolabel(rects):
    # attach some text labels
    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x() + rect.get_width()/2., height,
                '%d' % int(height),
                ha='center', va='bottom')

# autolabel(rects1)
# autolabel(rects2)
# autolabel(rects3)

plt.show()