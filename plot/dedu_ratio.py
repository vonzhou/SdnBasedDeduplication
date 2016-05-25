#!/usr/bin/env python
# _*_ coding:utf-8 _*-

"""
重删率的对比
形式为   重复的文件/新文件  以此计算重删率   （过于粗糙）
sed 数据集
"""


import numpy as np
import matplotlib.pyplot as plt




def get_dedu_ratio(duplicated, new):
	return float(duplicated) / float(duplicated + new)


# 设置显示中文
plt.rcParams['font.sans-serif'] = ['SimSun GB']   #['Hiragino Sans GB']
plt.rcParams['axes.unicode_minus'] = False

N = 10
source_dedu_ratio = (get_dedu_ratio(0, 16), get_dedu_ratio(6, 11), get_dedu_ratio(3, 140), get_dedu_ratio(129, 15), get_dedu_ratio(74, 215),
					get_dedu_ratio(221, 85), get_dedu_ratio(218, 121), get_dedu_ratio(204, 141), get_dedu_ratio(236, 118), get_dedu_ratio(275, 79) )

ind = np.arange(N)  # the x locations for the groups
width = 0.35       # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(ind, source_dedu_ratio, width, color='r', yerr=None)

source_dedu_sdn_time =  (get_dedu_ratio(0, 16), get_dedu_ratio(6, 11), get_dedu_ratio(0, 140), get_dedu_ratio(128, 16), get_dedu_ratio(73, 216),
					get_dedu_ratio(218, 88), get_dedu_ratio(213, 126), get_dedu_ratio(203, 142), get_dedu_ratio(235, 119), get_dedu_ratio(274, 80) )
rects2 = ax.bar(ind + width, source_dedu_sdn_time, width, color='y', yerr=None)

# add some text for labels, title and axes ticks
ax.set_ylabel(u'重删率')
ax.set_title('Deduplication Raion Comparison')
ax.set_xticks(ind + width)
ax.set_xticklabels(('1.18', '2.05', '3.01', '3.02', '4.0.6', '4.0.7', '4.0.8', '4.0.9', '4.1.1', '4.1.2'))

box = ax.get_position()
ax.set_position([box.x0, box.y0 + box.height * 0.1,
                 box.width, box.height * 0.9])

ax.legend((rects1[0], rects2[0]), ('Source Dedupe', 'With SDN'), loc='upper center', bbox_to_anchor=(0.5, -0.05),
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
