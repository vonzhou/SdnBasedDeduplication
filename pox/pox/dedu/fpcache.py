
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


def lookup2(fp):
	res = r.get(fp)
	if res == None:
		return False
	return True

def add2(fp):
	r.set(fp, True)
