"""

Utils for dedu

"""

from pox.core import core
from bitstring import BitArray
import bloomfilter as bloom


log = core.getLogger()

def sync_from_server(payload, bf, fpcache):
    fpfile = open('/tmp/fp.latest.out', 'r')
    for line in fpfile:
    	line = line.strip()
        bf.add(line)  # Bloom Filter
        fpcache.add2(line) # Cache   FIXME: change set

    fpfile.close()
    print '[*] Sync Over!'

def sync_bloomfilter_from_server(bloom_payload):
  """
  Not Used
  FIXME
  """
  payload_len = len(bloom_payload)
  print '[*] payload_len = ', payload_len
  hex_str = ''
  for byte in bloom_payload:
    print '{:02x}'.format(ord(byte)),
    hex_str = hex_str + '{:02x}'.format(ord(byte))
  print '-------------'


  c = BitArray(hex='0x'.join(hex_str), endian='big')
  # print c.bin[2:]
  bf = bloom.BloomFilter(payload_len * 8)
  bf.update(c.bin[2:])

  return bf

























