import argparse
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

def build(target):

    with open('blog', 'w') as outfile:
        def do_call(cmd):
            outfile.seek(0)
            outfile.truncate()
            call(cmd.split(' '), stderr=outfile, stdout=outfile)
        cmds = ['make -j4 clean', 'make -j4 lib', 'make -j4 {}'.format(target)]
        for cmd in cmds:
            if do_call(cmd):
                break

def main():
    parser = argparse.ArgumentParser(description='Contionously build profiler.')
    parser.add_argument('--target', default='all', help='an integer for the accumulator')
    args = parser.parse_args()
    try:
        files = get_files()
        checkers = [FileChange(ffile) for ffile in files]
        build(args.target)
        while True:
            nfiles = get_files()
            if files != nfiles:
                files = nfiles
                checkers = [FileChange(ffile) for ffile in files]
                build(args.target)
                continue
            if any([c.has_changed() for c in checkers]):
                checkers = [FileChange(ffile) for ffile in files]
                build(args.target)
                continue
            time.sleep(0.1)

    except KeyboardInterrupt:
        exit(1)

if __name__ == "__main__":
    main()
