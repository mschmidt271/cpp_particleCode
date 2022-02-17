#!/usr/bin/env python3

from doctest import testfile
import numpy as np
from sklearn.neighbors import KDTree
import yaml
import argparse
import csv

parser = argparse.ArgumentParser(description='Generate test points for ArborX test')
parser.add_argument('test_pts', type=str, default='test_pts.yaml',
                    help='yaml input points file name--e.g., test_pts.yaml')
parser.add_argument('results', type=str, default='cpp_results.txt',
                    help='cpp code output file name--e.g., cpp_results.txt')
args = parser.parse_args()

ptfile = args.test_pts
result_file = args.results

errcount = 0
with open(ptfile, 'r') as stream:
    try:
        test_pts = yaml.safe_load(stream)
    except IOError:
        print("Error: test_pts file does not appear to exist or is given in improper format.")
        errcount += 1

with open(result_file, 'r') as stream:
    try:
        results = dict()
        reader = csv.reader(stream)
        i = 0
        for row in reader:
            results[i] = np.sort([int(i) for i in row])
            i += 1
    except IOError:
        print("Error: results file does not appear to exist or is given in improper format.")
        errcount += 1

if errcount > 0:
    quit()

N = len(results)

X = np.ndarray((int(N), int(test_pts['dim'])))
for i, dim in enumerate(test_pts['pts']):
    for j, pt in enumerate(test_pts['pts'][dim]):
        X[j, i] = pt

tree = KDTree(X, leaf_size=N * 0.2)
ind, dist = tree.query_radius(X, float(test_pts['dist']),
                              sort_results=True, return_distance=True)
ind += 1

errs = np.ones(N)
tot_results = 0

for i in range(N):
    tot_results += results[i].size
    errs[i] = sum(abs(results[i] - np.sort(ind[i])))

print('Average number of neighbors found = ', float(tot_results) / float(N))
print('Total incorrect indices = ', int(sum(errs)))
