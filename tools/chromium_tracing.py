#!/usr/bin/env python2.7

import argparse
import json
import yaml

# Following Chromium tracing format as described here:
# - https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit#

def main():
    parser = argparse.ArgumentParser(description='Convert profile YAMLfile into Chromium tracing '
                                     'JSON format.')
    parser.add_argument('input', help='Name of the yaml file to be processed.')
    args = parser.parse_args()
    with open(args.input, 'r') as input_stream:
        data = yaml.load(input_stream)
    output = []
    for dat in data['records']:
        val = {'name': dat['name'],
               'cat': 'cxxprof',
               'pid': 0,
               'tid': 0}
        val['ph'] = 'B'
        val['ts'] = dat['t0']
        output.append(val.copy())
        val['ph'] = 'E'
        val['ts'] = dat['t1']
        output.append(val.copy())
    print json.dumps(output)

if __name__ == "__main__":
    main()
