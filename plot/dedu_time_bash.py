#!/usr/bin/env python
# _*_ coding:utf-8 _*-

"""
利用bash数据集测试的在有无SDN控制器缓存情况下 备份窗口的 对比 
13组
秒
server端通过文件是否存在判断文件是否重复
后面两组Bloom filter的大小分别为5000， 50000



"""


import numpy as np
import matplotlib.pyplot as plt

N = 13

ind = np.arange(N)  # the x locations for the groups
width = 0.25       # the width of the bars

fig, ax = plt.subplots()

source_dedu_time= (97.022823, 96.20613, 67.744214, 78.829066, 62.570538, 92.620999, 77.010950, 147.090866, 89.583345, 80.134119, 211.407158, 148.663094, 151.183888)
rects1 = ax.bar(ind - width, source_dedu_time, width, color='r', yerr=None, align='center')

source_dedu_sdn_time_5k =(25.776082, 23.435398, 23.678801, 31.879209, 30.488970, 44.618101, 46.834508, 103.220524, 67.620173, 52.417804, 190.274428, 137.545056, 139.674190)
rects2 = ax.bar(ind, source_dedu_sdn_time_5k, width, color='y', yerr=None, align='center')


source_dedu_sdn_time_50k =(23.92715, 19.691256, 16.342635, 22.180061, 20.652163, 26.709741, 25.962967, 41.927505, 32.799218, 28.053388, 80.862378, 83.89176, 90.669632)
rects3 = ax.bar(ind + width, source_dedu_sdn_time_50k, width, color='b', yerr=None, align='center')

# add some text for labels, title and axes ticks
ax.set_ylabel('Backup Time(s)')
ax.set_xlabel('bash version')
ax.set_title('Backup Time Comparison')
ax.set_xticks(ind)
ax.set_xticklabels(('1.14.7', '2.0', '2.01', '2.02','2.03','2.04','2.05','3.0', '3.1','3.2','4.0', '4.1', '4.2'))



ax.legend(('Base', 'SDN cache-bf5k', 'SDN cache-bf50k'), loc='upper center',
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