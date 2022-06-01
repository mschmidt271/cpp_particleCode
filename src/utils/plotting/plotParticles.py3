import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as ani

fname = "../data/particles.txt"

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

# print('IC_type_space: python var, file var = ', IC_type_space, float(params[0]))
# print('IC_type_mass: python var, file var = ', IC_type_mass, float(params[1]))
# print('omega: python var, file var = ', omega, float(params[2]), float(params[3]))
# print('X0_space: python var, file var = ', X0_space, float(params[4]))
# print('hat_pct: python var, file var = ', hat_pct, float(params[5]))
# print('X0_mass: python var, file var = ', X0_mass, float(params[6]))
# print('maxT: python var, file var = ', maxT, float(params[7]))
# print('dt: python var, file var = ', dt, float(params[8]))
# print('D: python var, file var = ', D, float(params[9]))
# print('pctRW: python var, file var = ', pctRW, float(params[10]))
# print('cdist_coeff: python var, file var = ', cdist_coeff, float(params[11]))
# print('cutdist: python var, file var = ', cutdist, float(params[12]))

shapeData = shapeData.split()
shapeData = [int(i) for i in shapeData]

X = np.reshape(data[:, 0], (shapeData[1] + 1, shapeData[0]))
mass = np.reshape(data[:, 1], (shapeData[1] + 1, shapeData[0]))

def histAnimation(frame):
    if frame == framesNum:
        plt.close(fig)
    else:
        plt.cla()
        p = plt.hist(X[frame, :])

def massAnimation(frame):
    if frame == framesNum:
        plt.close(fig)
    else:
        plt.cla()
        p = plt.scatter(X[frame, :], mass[frame, :])

framesNum = shapeData[1]

N = shapeData[0]
if IC_type_space == 0: # delta
    sigma = 0.0
elif IC_type_space == 1: # uniform
    sigma = 0.0
elif IC_type_space == 2: # equi-spaced
    sigma = 0.0
elif IC_type_space == 3: # hat/equi
    sigma = 0.0
else:
    print("spatial IC type not yet supported")
if IC_type_mass == 0: # point
    sigma = 0.0
elif IC_type_mass == 1: # uniform
    sigma = 0.0
elif IC_type_mass == 2: # equi-spaced
    sigma = 0.0
elif IC_type_mass == 3: # hat/equi
    sigma = 0.0
else:
    print("spatial IC type not yet supported")

L = omega[1] - omega[0]
def analytic(X, t, sigma, D, L):
    sol =  (1 / np.sqrt(2 * np.pi * (sigma**2 + 2 * D * t)))\
        * np.exp(-((0.5 * L - X[:])**2 / (2 * (sigma**2 + 2 * D * t))));
    return sol
# print('middle final masses = ', mass[-1, ])

fig = plt.figure()
plt.scatter(X[0, :], mass[0, :] / sum(mass[0, :]), label='PT')
# plt.scatter(X[0, :], asoln, label='analytic')
plt.legend()
plt.title('Initial')
plt.show()

asoln = analytic(X[-1, :], maxT, sigma, D, L)
asoln = asoln / sum(asoln)
error = np.linalg.norm(asoln - mass[-1, :])
print('error = ', error)
fig = plt.figure()
plt.scatter(X[-1, :], mass[-1, :] / sum(mass[-1, :]), label='PT')
plt.scatter(X[-1, :], asoln, label='analytic')
plt.legend()
plt.title('Final')
plt.show()

fig = plt.figure()
plt.subplot(211)
plt.hist(X[-1, :], density=True, bins=20)
plt.subplot(212)
plt.scatter(X[-1, :], asoln / (L / N))
plt.show()

# fig, ax0 = plt.subplots(nrows=1, ncols=1)
# ax0.scatter(X[-1, :], analytic, color='g')
# ax0.hist(X[-1, :], density=True)
# plt.show()

# # plot the initial condition
# fig = plt.figure()
# plt.hist(data[0, :])
# plt.show()

# plot the animation
fig = plt.figure()
# animator = ani.FuncAnimation(fig, histAnimation, frames=framesNum + 1,
#                              interval=3e2, repeat=False)
animator = ani.FuncAnimation(fig, massAnimation, frames=framesNum + 1,
                             interval=3e2, repeat=False)
plt.show()
