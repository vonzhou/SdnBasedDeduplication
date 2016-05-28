#!/usr/bin/env python
# _*_ coding:utf-8 _*-


import numpy as np
import matplotlib.pyplot as plt

# 设置显示中文
plt.rcParams['font.sans-serif'] = ['SimSun GB']   #['Hiragino Sans GB']
plt.rcParams['axes.unicode_minus'] = False

N = 100


base_time = [35.864084, 25.984043, 22.809581, 20.854368, 21.540684, 20.986777, 30.65078, 25.449317, 24.095248, 24.627791, 20.6026, 25.250811, 24.661246, 24.578511, 24.395426, 24.566146, 30.369837, 26.218681, 33.145348, 30.403454, 34.384884, 28.605804, 30.936569, 29.485927, 29.023289, 29.30296, 30.167173, 26.393532, 26.361653, 25.707609, 26.287501, 26.505135, 40.07638, 125.398518, 54.457605, 34.69576, 34.012292, 37.529002, 36.7996, 37.859057, 39.622989, 35.089736, 38.114096, 39.036498, 30.487628, 36.528917, 36.940481, 39.113457, 42.172363, 43.210634, 41.890665, 40.047441, 44.448117, 44.052758, 42.820569, 32.416522, 41.68344, 41.088757, 44.933058, 41.494707, 48.797684, 48.979888, 45.427157, 47.412854, 43.459107, 44.342739, 35.262351, 52.676937, 54.610058, 51.826021, 49.913013, 49.95549, 46.935362, 49.54493, 48.863674, 52.023476, 56.199504, 33.721839, 47.756891, 47.93745, 61.540703, 60.277767, 59.267777, 84.59218, 61.913512, 59.720249, 57.7022, 60.73286, 61.498139, 61.292145, 60.46862, 61.450914, 58.994622, 61.648853, 64.727572, 75.7737, 76.32984, 84.279029, 73.854571, 75.775568]

sdna_time_cache_800k = [4.008264, 3.943874, 4.339101, 3.975689, 3.926724, 3.859574, 4.778007, 4.488389, 4.231914, 4.418918, 4.195945, 4.702788, 5.734566, 4.580693, 4.499628, 4.762909, 5.24696, 4.846964, 5.663082, 4.968633, 5.272255, 4.695161, 4.866314, 4.695586, 4.698431, 4.963016, 5.658102, 4.764334, 4.988954, 4.614567, 4.846316, 4.713139, 5.688324, 15.445399, 27.535297, 20.677558, 19.383036, 21.324289, 18.953932, 19.879203, 20.010588, 17.28086, 18.208451, 19.364916, 34.864549, 17.138319, 18.326486, 18.667829, 19.465755, 20.262784, 20.249425, 20.318964, 21.874531, 24.074825, 24.820009, 33.118647, 23.71087, 21.666799, 23.944867, 22.526474, 26.710171, 28.052541, 26.314192, 27.369413, 26.237364, 26.938115, 35.9486, 33.591101, 34.180185, 32.81941, 33.208255, 35.12703, 31.673467, 35.718749, 33.877988, 37.123852, 47.18931, 36.929817, 41.259977, 44.581518, 56.894743, 60.041606, 60.394941, 69.795629, 51.483234, 47.983144, 48.390903, 48.424277, 47.338305, 47.171521, 48.308821, 48.362543, 46.641551, 56.00155, 48.724354, 55.599379, 54.392824, 57.586435, 47.300062, 47.578204]

sdna_time_cache_1mb = [3.984502, 3.978295, 4.055235, 3.993375, 4.383148, 3.927606, 4.730271, 4.541602, 4.286892, 4.438667, 4.089661, 4.570778, 3.990463, 4.017057, 3.868141, 4.052472, 4.754992, 4.232118, 5.151035, 4.873002, 5.061407, 4.723242, 4.931769, 4.690151, 4.76042, 4.95769, 5.509543, 4.668759, 4.858196, 4.622892, 4.79398, 4.692734, 5.676105, 15.189873, 9.171806, 6.409044, 6.377981, 7.602265, 6.362422, 6.617593, 7.899549, 6.887017, 7.746075, 8.904938, 18.004779, 8.748015, 9.111124, 9.267747, 10.099658, 10.426705, 10.049477, 9.652989, 12.33597, 13.947567, 16.13302, 16.928868, 15.435737, 14.797598, 17.207249, 15.715893, 19.330303, 19.811345, 18.591849, 20.625415, 20.48825, 21.357842, 17.209632, 27.039573, 28.058846, 26.905148, 25.958425, 28.380134, 25.208383, 26.0, 25.5, 18.7, 28.3, 18.5737, 43.208179, 37.923778, 49.914768, 52.891665, 53.515978, 63.293819, 44.917694, 42.630468, 41.305929, 40.596914, 43.035925, 42.189875, 42.566317, 42.14391, 40.389066, 48.369807, 43.926239, 49.948413, 47.952198, 52.185479, 42.409381, 43.404226]

sdna_time_cache_full = [3.52194, 3.326431, 3.422871, 3.630502, 3.222517, 3.230395, 4.210521, 3.924686, 4.203968, 4.338076, 3.841369, 4.690643, 4.596318, 4.444491, 3.863757, 4.090796, 4.470908, 4.017424, 4.948002, 4.68503, 4.871497, 4.487968, 5.694021, 5.707382, 5.889879, 6.015573, 6.105161, 5.448093, 6.17433, 5.235259, 5.589878, 6.03994, 5.485664, 15.109012, 8.887594, 6.172025, 6.101093, 7.123986, 6.064062, 6.267095, 6.96048, 5.749672, 6.460605, 8.226885, 6.127437, 7.444641, 7.418043, 8.111957, 7.400765, 7.590226, 7.273604, 7.126262, 7.91503, 7.758239, 7.654032, 6.871301, 8.743946, 8.14937, 9.434864, 8.731709, 10.125483, 9.941248, 9.517718, 9.970281, 7.869458, 7.932473, 6.150183, 10.733488, 9.980461, 9.084119, 11.012918, 10.489933, 9.584832, 10.718879, 8.534527, 9.179512, 11.209295, 7.234235, 10.375859, 10.803705, 13.447527, 12.605586, 11.973166, 22.163557, 11.319466, 10.986806, 10.218841, 10.042426, 10.888059, 10.912685, 11.423053, 11.422885, 10.895553, 12.792907, 13.320633, 20.37959, 21.436388, 27.332503, 18.464939, 21.843313]

x = np.arange(N)

plt.figure(figsize=(10,6))
plt.plot(x, base_time, label='$baseline$',marker="D",color='red' , linewidth = 2)
plt.plot(x, sdna_time_cache_800k, label='$sdna-800KB$',color='green', linewidth = 2)
plt.plot(x, sdna_time_cache_1mb, label='$sdna-1MB$', marker="+",color='blue', linewidth = 2)
plt.plot(x, sdna_time_cache_full, label='$sdna-full$', marker="x",color='purple', linewidth = 2)

yticks = np.arange(0, 150,10)
plt.yticks(yticks)

# add some text for labels, title and axes ticks
plt.xlabel(u'版本号')
plt.ylabel(u'备份时间（秒）')

#plt.title(u'备份时间（RDB）')
plt.legend()

plt.gca().yaxis.grid(True)
plt.show()

