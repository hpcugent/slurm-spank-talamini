#!/usr/bin/env python3
#
# Copyright 2012-2025 Ghent University
#
# This file is part of slurm-spank-talamini,
# originally created by the HPC team of Ghent University (http://ugent.be/hpc/en),
# with support of Ghent University (http://ugent.be/hpc),
# the Flemish Supercomputer Centre (VSC) (https://www.vscentrum.be),
# the Flemish Research Foundation (FWO) (http://www.fwo.be/en)
# and the Department of Economy, Science and Innovation (EWI) (http://www.ewi-vlaanderen.be/en).
#
# https://github.com/hpcugent/slurm-spank-talamini
#
# vsc-utils is free software: you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation, either version 2 of
# the License, or (at your option) any later version.
#
# slurm-spank-talamini is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with vsc-utils. If not, see <http://www.gnu.org/licenses/>.
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
    SLURM_NODELIST = os.environ.get('SLURM_NODELIST', '')

    if not SLURM_JOB_CPUS_PER_NODE or not SLURM_NODELIST:
        exit(1)

    nodes = SLURM_NODELIST.split(',')

    # SLURM_JOB_CPUS_PER_NODE is of the form (n(xm),)?((i,)+(s(xt))?)+ with n,m,i,s,t integers
    cpus = expand_cpus_pattern(SLURM_JOB_CPUS_PER_NODE)

    with NamedTemporaryFile(mode="w", dir="/tmp", delete=False) as pbsnodefile:
        for node, cpu_count in zip(nodes, cpus):
            pbsnodefile.write(f"{node}\n" * cpu_count)

        print(f"{pbsnodefile.name}")


if __name__ == '__main__':
    main()
