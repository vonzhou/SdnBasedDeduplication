import numpy as np 
import matplotlib.pyplot as plt 

n = 12
X = np.arange(n)

Y1 = (1 - X/float(n)) * np.random.uniform(0.5, 1.0, n)

plt.axes([0.025, 0.025, 0.95, 0.95])
plt.bar(X, +Y1, facecolor='#9999ff', edgecolor='white')

for x,y in zip(X, Y1):
	plt.text(x+0.4, y+0.5, '%.2f' % y, ha='center', va = 'bottom')

plt.xlim(-.5, n), plt.xticks([])
plt.ylim(-1.25, + 1.25), plt.yticks([])

plt.show()