from os import path, walk
import fnmatch
import time

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

def main():
    files = get_files()
    checkers = [FileChange(ffile) for ffile in files]
    while True:
        if files != get_files():
            return
        if any([c.has_changed() for c in checkers]):
            return
        time.sleep(0.1)

try:
    main()
except KeyboardInterrupt:
    exit(1)
