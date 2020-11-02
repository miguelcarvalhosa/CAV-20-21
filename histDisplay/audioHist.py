import numpy      as np
import gnuplotlib as gp

r= np.array([],dtype = int)
l= np.array([],dtype = int)
m= np.array([],dtype = int)

count = 0
f = open("snd_hist2.txt", "r")
lines=f.readlines()
for x in lines:
    r=np.append(r,int(x.split(' ')[0]))
    l=np.append(l,int(x.split(' ')[1]))
    m = np.append(m, int(x.split(' ')[2]))
    count = count + 1

gp.plot( (r, dict(histogram=0)), (l, dict(histogram=0)), (m, dict(histogram=0)), _xmin=-32767, _xmax=32767, _with='boxes',
         multiplot='title "(R,L,M) Histograms" layout 3,1')