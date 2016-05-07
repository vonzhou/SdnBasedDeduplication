
import hashes as h

inf = open('dictionary', 'r')
outf = open('dictionary.py.out', 'w')

for line in inf:
    line = line.strip()
    #res = str(h.sax_hash(line)) + '\n'
    # res = str(h.sdbm_hash(line)) + '\n'
    #res = str(h.jenkins_one_at_a_time_hash(line)) + '\n'
    res = str(h.murmur2_hash(line)) + '\n'
    outf.write(res)

inf.close()
outf.flush()
outf.close()

# print str(h.sdbm_hash('1CC91CD7FB0F9CD986476D9AF91AB5CF1A621E12'))
# print str(h.jenkins_one_at_a_time_hash('1CC91CD7FB0F9CD986476D9AF91AB5CF1A621E12'))

