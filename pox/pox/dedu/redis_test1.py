"""
Test Redis LRU cache
memory size <--> fp amount

"""
import redis

r = redis.StrictRedis(host='localhost', port=6379, db=0)

def lookup(fp):
	return r.sismember("fpset", fp)
	# res = r.get(fp)
	# if res == None:
	# 	return False
	# return True

def add(fp):
	# r.set(fp, 1)
	r.sadd("fpset", fp)

fpfile = open("filefps.txt", "r")
count = 1
for line in fpfile:
	line = line.strip()
	add(line)
	print '---------', count
	count = count + 1
