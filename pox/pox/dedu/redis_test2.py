"""
Test Redis LRU cache
memory size <--> fp amount

"""
import redis

r = redis.StrictRedis(host='localhost', port=6379, db=0)

r.set("foo", 'bar')
print r.get('foo')
print r.get('not exist')
