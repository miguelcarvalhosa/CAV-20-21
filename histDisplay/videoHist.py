import matplotlib.pyplot as plt
import numpy as np
from matplotlib import colors
from matplotlib.ticker import PercentFormatter

b=[]
g=[]
r=[]
gray=[]

gg = {}
rr = {}
bb = {}
ggray = {}


f = open("video_hist.txt", "r")
lines=f.readlines()

count = 0
for x in lines:
    count = count + 1
    nFrames = int(x.split(' ')[0])
    b.append(int(x.split(' ')[1]))
    g.append(int(x.split(' ')[2]))
    r.append(int(x.split(' ')[3]))
    gray.append(int(x.split(' ')[4]))

fig, axs = plt.subplots(2, 2)
for n in range(nFrames):
    for y in range(256):
        bb[y] = (b[n * 256 + y])
        gg[y] = (g[n * 256 + y])
        rr[y] = (r[n * 256 + y])
        ggray[y] = (gray[n * 256 + y])

    b_val, b_weight = zip(*[(k, v) for k,v in bb.items()])
    g_val, g_weight = zip(*[(k, v) for k, v in gg.items()])
    r_val, r_weight = zip(*[(k, v) for k, v in rr.items()])
    gray_val, gray_weight = zip(*[(k, v) for k, v in ggray.items()])
    axs[0, 0].hist(b_val, bins = 64, weights=b_weight, color="b")
    axs[0, 1].hist(g_val, bins=64, weights=g_weight, color="g")
    axs[1, 0].hist(r_val, bins=64, weights=r_weight, color="r")
    axs[1, 1].hist(gray_val, bins=64, weights=gray_weight, color="k")

    plt.draw()
    plt.pause(0.001)
