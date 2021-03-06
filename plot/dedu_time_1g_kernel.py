#!/usr/bin/env python
# _*_ coding:utf-8 _*-


import numpy as np
import matplotlib.pyplot as plt

# 设置显示中文
plt.rcParams['font.sans-serif'] = ['SimSun GB']   #['Hiragino Sans GB']
plt.rcParams['axes.unicode_minus'] = False

N = 100


base_time = [35.864084, 25.984043, 22.809581, 20.854368, 21.540684, 20.986777, 30.65078, 25.449317, 24.095248, 24.627791, 20.6026, 25.250811, 24.661246, 24.578511, 24.395426, 24.566146, 30.369837, 26.218681, 33.145348, 30.403454, 34.384884, 28.605804, 30.936569, 29.485927, 29.023289, 29.30296, 30.167173, 26.393532, 26.361653, 25.707609, 26.287501, 26.505135, 40.07638, 125.398518, 54.457605, 34.69576, 34.012292, 37.529002, 36.7996, 37.859057, 39.622989, 35.089736, 38.114096, 39.036498, 30.487628, 36.528917, 36.940481, 39.113457, 42.172363, 43.210634, 41.890665, 40.047441, 44.448117, 44.052758, 42.820569, 32.416522, 41.68344, 41.088757, 44.933058, 41.494707, 48.797684, 48.979888, 45.427157, 47.412854, 43.459107, 44.342739, 35.262351, 52.676937, 54.610058, 51.826021, 49.913013, 49.95549, 46.935362, 49.54493, 48.863674, 52.023476, 56.199504, 33.721839, 47.756891, 47.93745, 61.540703, 60.277767, 59.267777, 84.59218, 61.913512, 59.720249, 57.7022, 60.73286, 61.498139, 61.292145, 60.46862, 61.450914, 58.994622, 61.648853, 64.727572, 75.7737, 76.32984, 84.279029, 73.854571, 75.775568]

sdna_time_cache_full_bf50k = [3.52194, 3.326431, 3.422871, 3.630502, 3.222517, 3.230395, 4.210521, 3.924686, 4.203968, 4.338076, 3.841369, 4.690643, 4.596318, 4.444491, 3.863757, 4.090796, 4.470908, 4.017424, 4.948002, 4.68503, 4.871497, 4.487968, 5.694021, 5.707382, 5.889879, 6.015573, 6.105161, 5.448093, 6.17433, 5.235259, 5.589878, 6.03994, 5.485664, 15.109012, 8.887594, 6.172025, 6.101093, 7.123986, 6.064062, 6.267095, 6.96048, 5.749672, 6.460605, 8.226885, 6.127437, 7.444641, 7.418043, 8.111957, 7.400765, 7.590226, 7.273604, 7.126262, 7.91503, 7.758239, 7.654032, 6.871301, 8.743946, 8.14937, 9.434864, 8.731709, 10.125483, 9.941248, 9.517718, 9.970281, 7.869458, 7.932473, 6.150183, 10.733488, 9.980461, 9.084119, 11.012918, 10.489933, 9.584832, 10.718879, 8.534527, 9.179512, 11.209295, 7.234235, 10.375859, 10.803705, 13.447527, 12.605586, 11.973166, 22.163557, 11.319466, 10.986806, 10.218841, 10.042426, 10.888059, 10.912685, 11.423053, 11.422885, 10.895553, 12.792907, 13.320633, 20.37959, 21.436388, 27.332503, 18.464939, 21.843313]
x = np.arange(N)

plt.figure(figsize=(10,6))
plt.plot(x, base_time, label='$baseline-1Gb$',marker="x", color='red' , linewidth = 2)
plt.plot(x, sdna_time_cache_full_bf50k, label='$sdna-1Gb$', color='purple', linewidth = 2)

yticks = np.arange(0, 150, step = 15)
plt.yticks(yticks)

# add some text for labels, title and axes ticks
plt.xlabel(u'版本号')
plt.ylabel(u'备份时间（秒）')

#plt.title(u'备份时间（RDB）')
plt.legend()

plt.gca().yaxis.grid(True)
plt.show()

