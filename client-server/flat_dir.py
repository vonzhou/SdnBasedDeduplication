

# Input : directory
# Output : a folder with all files without sub dir
import os
from os import listdir
from os.path import isfile,join

def reconstruct_dir(pathname):
    count = 0
    for dirName, subDirLists, fileLists in os.walk(pathname):
        print 'Found Dir: %s' % dirName
        for fname in fileLists:
            count = count + 1
            print fname
            fullfname = os.path.join(dirName + '/' + fname)
            print fullfname
    print 'File Counts:',count


reconstruct_dir(".")
