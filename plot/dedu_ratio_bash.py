#!/usr/bin/env python
# _*_ coding:utf-8 _*-

"""
重删率的对比
形式为   重复的文件/新文件  以此计算重删率  
bash 数据集
"""


import numpy as np
import matplotlib.pyplot as plt


def get_dedu_ratio_old(duplicated, total):
	return float(duplicated) / float(duplicated + new)

def get_dedu_ratio(duplicated, total):
  return float(duplicated) / float(total)



N = 13


ind = np.arange(N) + 1 # the x locations for the groups
width = 0.35       # the width of the bars

fig, ax = plt.subplots()

source_dedu_ratio = (get_dedu_ratio(21, 402), get_dedu_ratio(103, 511), get_dedu_ratio(300, 579), get_dedu_ratio(355, 644), get_dedu_ratio(488, 675), 
          get_dedu_ratio(362, 731), get_dedu_ratio(516, 761), get_dedu_ratio(349, 919), get_dedu_ratio(685, 946), get_dedu_ratio(775, 951) ,
          get_dedu_ratio(421, 1076), get_dedu_ratio(821, 1115), get_dedu_ratio(857, 1154))
rects1 = ax.bar(ind-width, source_dedu_ratio, width, color='r', yerr=None)

source_dedu_sdn_time =  (get_dedu_ratio(0, 402), get_dedu_ratio(92, 511), get_dedu_ratio(292, 579), get_dedu_ratio(353, 644), get_dedu_ratio(484, 675), 
          get_dedu_ratio(355, 731), get_dedu_ratio(512, 761), get_dedu_ratio(343, 919), get_dedu_ratio(683, 946), get_dedu_ratio(774, 951) ,
          get_dedu_ratio(412, 1076), get_dedu_ratio(817, 1115), get_dedu_ratio(854, 1154))
rects2 = ax.bar(ind, source_dedu_sdn_time, width, color='y', yerr=None)

# add some text for labels, title and axes ticks
ax.set_ylabel('Dedu Ratio')
ax.set_title('Deduplication Raion Comparison')
ax.set_xlabel('bash version')
ax.set_xticks(ind)
ax.set_xticklabels(('1.14.7', '2.0', '2.01', '2.02','2.03','2.04','2.05','3.0', '3.1','3.2','4.0', '4.1', '4.2'))

box = ax.get_position()
ax.set_position([box.x0, box.y0 + box.height * 0.1,
                 box.width, box.height * 0.9])

ax.legend((rects1[0], rects2[0]), ('Source Dedupe', 'SDN cache'), loc='upper left',
          fancybox=True, shadow=True, ncol=5)


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