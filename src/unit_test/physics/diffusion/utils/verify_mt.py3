#!/usr/bin/env python3

# script to verify that MT unit test is passing

import numpy as np
# import matplotlib.pyplot as plt

fname = "./data/particles.txt"

omega = np.zeros(2);

with open(fname) as f:
    shapeData = f.readline()
    dim = f.readline()
    p = f.readline()

params = p.split()
IC_type_space = float(params[0])
IC_type_mass = float(params[1])
omega[0]= float(params[2])
omega[1]= float(params[3])
X0_space = float(params[4])
hat_pct = float(params[5])
X0_mass = float(params[6])
maxT = float(params[7])
dt = float(params[8])
D = float(params[9])
pctRW = float(params[10])
cdist_coeff = float(params[11])
cutdist = float(params[12])
data = np.loadtxt(fname, skiprows=3)

# delta IC
sigma = 0.0

shapeData = shapeData.split()
shapeData = [int(i) for i in shapeData]

X = np.reshape(data[:, 0], (shapeData[1] + 1, shapeData[0]))
mass = np.reshape(data[:, 1], (shapeData[1] + 1, shapeData[0]))

framesNum = shapeData[1]

L = omega[1] - omega[0]
def analytic(dim, X, t, sigma, D, L):
    sol =  (1 / np.power(2 * np.pi * (sigma**2 + 2 * D * t), float(dim) / 2.0))\
        * np.exp(-((0.5 * L - X[:])**2 / (2 * (sigma**2 + 2 * D * t))));
    return sol

asoln = analytic(dim, X[-1, :], maxT, sigma, D, L)
asoln = asoln / sum(asoln)
error = np.linalg.norm(asoln - mass[-1, :])
# print('error = ', error)

# fig = plt.figure()
# plt.scatter(X[-1, :], mass[-1, :] / sum(mass[-1, :]), label='PT')
# plt.scatter(X[-1, :], asoln, label='analytic')
# plt.legend()
# plt.title('Final')
# plt.show()

assert error <= 1.0e-14, f"1D MT error too high: error = {error}"

print('SUCCESS: 1D MT passes.')
