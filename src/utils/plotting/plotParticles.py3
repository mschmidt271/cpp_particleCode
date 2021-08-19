import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as ani

fname = "../data/particles.txt"

with open(fname) as f:
    shapeData = f.readline()
data = np.loadtxt(fname, skiprows=1)

shapeData = shapeData.split()
shapeData = [int(i) for i in shapeData]

data = np.reshape(data, (shapeData[1] + 1, shapeData[0]))

def histAnimation(frame):
    if frame == framesNum:
        plt.close(fig)
    else:
        plt.cla()
        p = plt.hist(data[frame, :])

framesNum = shapeData[1]

# # plot the initial condition
# fig = plt.figure()
# plt.hist(data[0, :])
# plt.show()

# plot the animation
fig = plt.figure()
animator = ani.FuncAnimation(fig, histAnimation, frames=framesNum + 1,
                             interval=3e2, repeat=False)
plt.show()
