#!/usr/bin/env python3

#
# Script to generate the pbs nodefile based on the
# environment variables set by Slurm:
# - SLURM_JOB_CPUS_PER_NODE
# - SLURM_JOB_NODELIST

from tempfile import NamedTemporaryFile
import os
import re

def expand_cpus_pattern(input_string):
    # Pattern to match parts like 3(x2),4, etc.
    pattern = r'(\d+)(?:\(x(\d+)\))?'

    result = []
    matches = re.findall(pattern, input_string)

    for number, repeat in matches:

        num = int(number)
        if repeat:  # If there's a repetition (xN)
            count = int(repeat)
            result.extend([num] * count)  # Add the number `count` times
        else:
            result.append(num)  # Add the single number

    return result

def main():

    SLURM_JOB_CPUS_PER_NODE = os.environ.get('SLURM_JOB_CPUS_PER_NODE', '')
    SLURM_JOB_NODELIST = os.environ.get('SLURM_JOB_NODELIST', '')

    nodes = SLURM_JOB_NODELIST.split(',')

    # SLURM_JOB_CPUS_PER_NODE is of the form (n(xm),)?((i,)+(s(xt))?)+ with n,m,i,s,t integers
    cpus = expand_cpus_pattern(SLURM_JOB_CPUS_PER_NODE)

    with NamedTemporaryFile(mode="w", dir="/tmp", delete=False) as pbsnodefile:
        for node, cpu_count in zip(nodes, cpus):
            pbsnodefile.write(f"{node}\n" * cpu_count)

        print(f"{pbsnodefile.name}")

if __name__ == 'main':
    main()
