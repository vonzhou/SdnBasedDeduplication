#!/usr/bin/env python
# _*_ coding:utf-8 _*-

"""
有无SDN控制器缓存情况下 备份窗口的 对比
redis数据集43组
秒



"""


import numpy as np
import matplotlib.pyplot as plt


def get_dedu_ratio(duplicated, all):
	return float(duplicated) / float(all)

# 设置显示中文
plt.rcParams['font.sans-serif'] = ['SimSun GB']   #['Hiragino Sans GB']
plt.rcParams['axes.unicode_minus'] = False

N = 37
files_count     = [392,393,393,393, 400, 400, 400, 400, 400, 400, 400, 409, 409, 414, 414, 414, 473, 478, 478, 478, 478, 478, 481,  481, 481, 481, 482, 482, 488, 511, 511, 512, 512, 512, 512, 512, 512]
#print len(files_count)
base_files_deduplicated = [8, 377, 386,384, 315, 396, 394, 374, 379, 383, 388, 386, 384, 394, 390, 401, 298, 429, 429, 465, 473, 475, 412, 464, 458, 472, 469, 472, 477, 434, 507, 503, 501, 502, 503, 495, 504]
#print len(base_files_deduplicated)
base_ratio = [get_dedu_ratio (y,x) for x,y in zip(files_count, base_files_deduplicated)]
average_base_ratio = sum(base_ratio)/float(N)

sdna_files_duplicated = [0, 377, 386, 384, 313, 396, 394, 374, 379, 383, 388, 386, 384, 394, 390, 401, 296, 429, 427, 465, 473, 475, 412, 464, 458, 472, 469, 472, 477, 435, 506, 503, 501, 502, 503, 495, 504]
#print len(sdna_files_duplicated)
sdna_ratio = [get_dedu_ratio (y,x) for x,y in zip(files_count, sdna_files_duplicated)]
average_sdna_ratio = sum(sdna_ratio)/float(N)

print average_base_ratio
print average_sdna_ratio

x = np.arange(N)

#plt.figure(figsize=(10,8))
plt.plot(x, sdna_ratio, label='$sdna$', color='red', linewidth = 2)
plt.plot(x, base_ratio, label='$base$', linewidth = 2)

# 设置y轴范围
#yticks = np.linspace(0, 200, num = 10)
#yticks = np.arange(0, 200, 10)
#plt.yticks(yticks)

# add some text for labels, title and axes ticks
plt.xlabel(u'版本号')
plt.ylabel(u'重删率')

plt.title(u'重删率对比（RDB）')
plt.legend()

plt.gca().yaxis.grid(True)


#ax.set_xticks(ind + width)
#ax.set_xticklabels(('1.18', '2.05', '3.01', '3.02', '4.0.6', '4.0.7', '4.0.8', '4.0.9', '4.1.1', '4.1.2'))

#ax.legend((rects1[0], rects2[0]), ('Source Dedupe', 'With SDN'))

plt.show()

