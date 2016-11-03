from os import path, walk
import fnmatch
import time
from subprocess import call

class FileChange(object):
    def __init__(self, fname):
        self._fname = fname
        self._mtime = path.getmtime(self._fname)

    def has_changed(self):
        mtime = path.getmtime(self._fname)
        if self._mtime < mtime:
            self._mtime = mtime
            return True
        return False

def get_files():
    patterns = ['[!.]*.h', '[!.]*.cpp', 'Makefile']
    dirpath = '.'
    res = []
    for pat in patterns:
        res += [path.join(ddir, ffile)
                for ddir, _, files in walk(dirpath)
                for ffile in fnmatch.filter(files, pat)]
    return res

def build():
    with open('blog', 'w') as outfile:
        call(['make', '-j4', 'clean'], stderr=outfile, stdout=outfile)
        call(['make', '-j4', 'unit'], stderr=outfile, stdout=outfile)

def main():
    try:
        files = get_files()
        checkers = [FileChange(ffile) for ffile in files]
        build()
        while True:
            nfiles = get_files()
            if files != nfiles:
                files = nfiles
                checkers = [FileChange(ffile) for ffile in files]
                build()
                continue
            if any([c.has_changed() for c in checkers]):
                checkers = [FileChange(ffile) for ffile in files]
                build()
                continue
            time.sleep(0.1)

    except KeyboardInterrupt:
        exit(1)

if __name__ == "__main__":
    main()
