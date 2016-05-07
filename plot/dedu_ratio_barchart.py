#!/usr/bin/env python
# _*_ coding:utf-8 _*-


# a bar plot with errorbars
import numpy as np
import matplotlib.pyplot as plt

plt.rcParams['font.sans-serif'] = ['SimSun GB']   #['Hiragino Sans GB']
plt.rcParams['axes.unicode_minus'] = False
N = 2
#  kernel , RDB
base = [0.852489457492, 0.919957189989]
sdna = [0.852354046734, 0.919043119895]
base = [float('%.5f'%x) for x in base]
sdna = [float('%.5f'%x) for x in sdna]
ind = np.arange(N)  # the x locations for the groups

width = 0.25       # the width of the bars

fig, ax = plt.subplots()
fig.set_size_inches(8, 6)
rects1 = ax.bar(ind, base, width, color='r', yerr=None)
rects2 = ax.bar(ind + width, sdna, width, color='y')

# add some text for labels, title and axes ticks
ax.set_ylabel(u'重删率')
ax.set_xticks(ind + width)
ax.set_xticklabels(('RDB', 'kernel'))
ax.set_ylim([0,1.2])
ax.legend((rects1[0], rects2[0]), ('$baseline$', '$sdna$'))


def autolabel(rects):
    # attach some text labels
    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x() + rect.get_width()/2., 1.02*height,
                '%s' % float(height),
                ha='center', va='bottom')

def autolabel2(rects):
    # attach some text labels
    for rect in rects:
        height = rect.get_height()
        ax.text(rect.get_x() + rect.get_width()/2., 1.06*height,
                '%s' % float(height),
                ha='center', va='bottom')

autolabel(rects1)
autolabel2(rects2)

plt.show()
