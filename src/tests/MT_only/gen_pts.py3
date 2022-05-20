#!/usr/bin/env python3

# this is a script to generate 1, 2, or 3d points for input to the particle code

import numpy as np
import yaml
import argparse

# Instantiate the parser and parse them args
parser = argparse.ArgumentParser(description='Generate particle IC points')
parser.add_argument('--fname', type=str, required=True,
                    help='yaml input/output file name--e.g., pts.yaml')

args = parser.parse_args()
fname = str(args.fname)

with open(fname, 'r') as yamlfile:
    cur_yaml = yaml.safe_load(yamlfile)
    dim = cur_yaml['dimension']
    N = cur_yaml['Np']
    omega = cur_yaml['omega']
    pt_style = cur_yaml['initial_condition']['space']['type']

if pt_style == 'rand':
    pts = np.random.uniform(omega[0], omega[1], [int(N), dim])
elif pt_style == 'equi':
    pts = np.linspace(omega[0], omega[1], N)
elif pt_style == 'point':
    with open(fname, 'r') as yamlfile:
        cur_yaml = yaml.safe_load(yamlfile)
        X0 = cur_yaml['initial_condition']['space']['X0']
    pts = X0 * np.ones((N, dim))
else:
    raise ValueError("How did we get here??")

data = dict()
data['pts'] = {}
pts_out = data['pts']

if pt_style == 'rand':
    if dim == 1:
        pts_out['x'] = pts[:, 0].tolist()
    elif dim == 2:
        pts_out['x'] = pts[:, 0].tolist()
        pts_out['y'] = pts[:, 1].tolist()
    elif dim == 3:
        pts_out['x'] = pts[:, 0].tolist()
        pts_out['y'] = pts[:, 1].tolist()
        pts_out['z'] = pts[:, 2].tolist()
elif pt_style == 'equi':
    if dim == 1:
        pts_out['x'] = pts.tolist()
    elif dim == 2:
        xgrid, ygrid = np.meshgrid(pts, pts)
        # note that the -1 value below causes the dimension length to be
        # inferred, and the empty argument makes it a 1-d array, rather than
        # 2-d with a null dimension
        xgrid = np.reshape(xgrid, (-1,))
        ygrid = np.reshape(ygrid, (-1,))
        pts_out['x'] = xgrid.tolist()
        pts_out['y'] = ygrid.tolist()
        # update the total number of particles
        N = N**2
    elif dim == 3:
        xgrid, ygrid, zgrid = np.meshgrid(pts, pts, pts)
        # note that the -1 value below causes the dimension length to be
        # inferred, and the empty argument makes it a 1-d array, rather than
        # 2-d with a null dimension
        xgrid = np.reshape(xgrid, (-1,))
        ygrid = np.reshape(ygrid, (-1,))
        zgrid = np.reshape(zgrid, (-1,))
        pts_out['x'] = xgrid.tolist()
        pts_out['y'] = ygrid.tolist()
        pts_out['z'] = zgrid.tolist()
        # update the total number of particles
        N = N**3
elif pt_style == 'point':
    if dim == 1:
        pts_out['x'] = pts[:, 0].tolist()
    elif dim == 2:
        pts_out['x'] = pts[:, 0].tolist()
        pts_out['y'] = pts[:, 1].tolist()
    elif dim == 3:
        pts_out['x'] = pts[:, 0].tolist()
        pts_out['y'] = pts[:, 1].tolist()
        pts_out['z'] = pts[:, 2].tolist()
else:
    raise ValueError("How did we get here??")

# open the file and write the yaml
with open(fname, 'r') as yamlfile:
    cur_yaml = yaml.safe_load(yamlfile)
    cur_yaml['points'] = {}
    cur_yaml['points'] = pts_out
    cur_yaml['Np'] = N


if cur_yaml:
    with open(fname, 'w') as yamlfile:
        yaml.safe_dump(cur_yaml, yamlfile)
