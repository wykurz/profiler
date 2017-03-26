#!/usr/bin/env python2.7

import argparse
import os
import re
import networkx as nx
import pydot

def find_files(start):
    regex = re.compile('.*\.(cpp|h)$')
    res = []
    for root, _dirs, fnames in os.walk(start):
        for fname in fnames:
            if regex.match(fname):
                res.append(os.path.join(root, fname))
    return res

def parse_includes(filepath):
    regex = re.compile('#include ["<](.*)[">]$')
    res = []
    with open(filepath) as fobj:
        for line in fobj:
            match = regex.match(line)
            if match:
                res.append(match.group(1))
    return res

def main():
    parser = argparse.ArgumentParser(description='Create dependency graph of your project files.')
    parser.add_argument('root', help='Project root directory.')
    args = parser.parse_args()

    ngraph = nx.DiGraph()
    pgraph = pydot.Dot(graph_type='digraph')
    for fname in find_files(args.root):
        rel_fname = os.path.relpath(fname, args.root)
        for include in parse_includes(fname):
            ngraph.add_edge(rel_fname, include)
            pgraph.add_edge(pydot.Edge(rel_fname, include))

    print [edge for edge in nx.simple_cycles(ngraph)]
    pgraph.write_png('include_deps.png')

if __name__ == "__main__":
    main()
