from bitarray import bitarray
import hashes as h

class BloomFilter:
    
    def __init__(self, size):
        self.size = size
        # How to pass as args ???
        self.hash_funcs = [h.sax_hash, h.sdbm_hash, h.jenkins_one_at_a_time_hash, h.murmur2_hash]
        self.bit_array = bitarray(size)
        self.bit_array.setall(0)

        
    def add(self, string):
    	for f in self.hash_funcs:
    		index = f(string) % self.size
    		self.bit_array[index] = 1
            
    def lookup(self, string):
    	for f in self.hash_funcs:
    		index = f(string) % self.size
    		if self.bit_array[index] == 0:
    			return 0  # definitely no
    	return 1	# probably yes

    def update(self, bitstr):
        self.bit_array = bitarray(bitstr)
