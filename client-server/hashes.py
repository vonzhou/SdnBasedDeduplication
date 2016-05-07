"""
Some basic hash funcs 
h & 0xffffffff to get 32bit unsigned  int otherwise error

"""
import struct 

def sax_hash(key):
    """
    shift and xor hash
    """
    h = 0
    key_len = len(key)
    for i in range(0, key_len):
        h ^= (h << 5) + (h >> 2) + ord(key[i])
        h = h & 0xffffffff
        
    return h & 0xffffffff


def sdbm_hash(key):
    """
    SDBM hash function
    """
    h = 0
    key_len = len(key)
    for i in range(0, key_len):
        h = ord(key[i]) + (h << 6) + (h << 16) - h
        h & 0xffffffff
        
    return h & 0xffffffff

def jenkins_one_at_a_time_hash(key):
    """
    jenkins one_at_a_time hash 
    """
    if key == '':
        return 0
    h = 0
    key_len = len(key)
    for i in range(0, key_len):
        h += ord(key[i])
        h += (h & 0xffffffff) << 10
        h ^= (h & 0xffffffff) >> 6


    h += (h & 0xffffffff) << 3
    h ^= (h & 0xffffffff) >> 11
    h += (h & 0xffffffff) << 15
    return h & 0xffffffff


def murmur2_hash(key):
    """
    Murmur2 hash function
    TODO : consider machine endian 
    """
    if key == '':
        return 0
    m = 0x5bd1e995
    r = 24 & 0xffffffff
    seed = int('0xdeadbeef', 16) & 0xffffffff
    key_len = len(key)
    h = seed ^ key_len
    i = 0
    while key_len >= 4:
        k = struct.unpack('<I', key[i:i+4])[0] & 0xffffffff
        k = (k * m ) & 0xffffffff
        k ^= k >> r
        k = (k * m ) & 0xffffffff
        h = (h * m) & 0xffffffff
        h = (h ^ k) & 0xffffffff
        # print str(k), str(h)

        i += 4
        key_len -= 4

    # bug: Not key[2] , but key[i+2]
    if key_len == 3:
        h = h ^ (ord(key[i+2])  << 16) 
        h = h & 0xffffffff
        
        h = h & 0xffffffff
        h = (h ^ (ord(key[i+1]) << 8)) & 0xffffffff

        h ^= ord(key[i])
        h = (h * m) & 0xffffffff
    elif key_len==2:
        
        h = h & 0xffffffff
        h = (h ^ (ord(key[i+1]) << 8)) & 0xffffffff
        h ^= ord(key[i])
        h = (h * m) & 0xffffffff
   
    elif key_len == 1:
        h ^= ord(key[i])
        h = (h * m) & 0xffffffff

    h = (h ^ (h >> 13)) & 0xffffffff
    h = (h * m) & 0xffffffff
    h ^= h >> 15

    return h & 0xffffffff

