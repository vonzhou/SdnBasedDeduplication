


"""

Extract what data you want  from log file :
file_count
file_deduplicated
file_new
file_deduplicated_pox
file_new_pox
time_ms
time_s 

file_count=xx,file_deduplicated=xxx,file_new=xxx,file_deduplicated_pox=xxx,file_new_pox=xxx

"""

filename = raw_input('Enter file name: ')
f = open(filename, 'r')


file_count_arr = []
file_deduplicated_arr = []
file_new_arr = []
file_deduplicated_pox_arr = []
file_new_pox_arr = []
time_ms_arr=[]
time_s_arr = []

for line in f:
	# print line
	if line != '':
		if line.startswith('mininet'):
			continue
		pairs = line.split(',')
		for pair in pairs:
			keyvalue = pair.split('=')
			key = keyvalue[0]
			val = keyvalue[1]
			if key == 'file_count':
				file_count_arr.append(int(val))
			if key == 'file_deduplicated':
				file_deduplicated_arr.append(int(val))
			if key == 'file_new':
				file_new_arr.append(int(val))
			if key == 'file_deduplicated_pox':
				file_deduplicated_pox_arr.append(int(val))
			if key == 'file_new_pox':
				file_new_pox_arr.append(int(val))
			if key == 'time_s':
				time_s_arr.append(float(val))
f.close()


print '--------------------------dedu time-------------------'
print time_s_arr
print '--------------------------file count------------------'
print file_count_arr
print '---------------------------file new count-------------'
print file_new_arr
print '----------------------------file new count from pox---'
print file_new_pox_arr
print '----------------------------average dedu ratio--------'

file_new_all = [(x+y) for x,y in zip(file_new_arr, file_new_pox_arr)]


def get_ratio(alls,news):
	return float(alls-news)/float(alls)

ratio_arr = [get_ratio(x,y) for x,y in zip(file_count_arr, file_new_all)]
average = sum(ratio_arr)/float(len(ratio_arr))

print average

 