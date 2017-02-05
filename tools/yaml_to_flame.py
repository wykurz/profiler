#!/usr/bin/env python2.7

import argparse
import pandas as pd
import yaml

def main():
    parser = argparse.ArgumentParser(description='Convert profile yaml file into a FlameGraph format.')
    parser.add_argument('input', help='Name of the yaml file to be processed.')
    args = parser.parse_args()
    with open(args.input, 'r') as input_stream:
        data = yaml.load(input_stream)
    df = pd.DataFrame.from_dict(data['records'])
    df.sort_values('seq', inplace=True)
    depth_name = df[['depth', 'name']].values.tolist()
    pdepth = -1
    stacks = [[]]
    for i in range(len(depth_name)):
        idepth, iname = depth_name[i]
        if pdepth < idepth:
            assert pdepth + 1 == idepth
            stacks.append(stacks[-1] + [iname])
        else:
            pstack = stacks[-1][:pdepth - 1]
            stacks.append(pstack + [iname])
        pdepth = idepth
        assert len(stacks[-1]) == idepth + 1
    df['stack'] = [';'.join(stack) for stack in stacks[1:]]
    df['duration'] = df['t1'] - df['t0']
    for stack, duration in df[['stack', 'duration']].values.tolist():
        print stack, duration


if __name__ == "__main__":
    main()
