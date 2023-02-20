from celluloid import Camera
from matplotlib import pyplot as plt


f = open('outputForPython.csv', 'r')

fig = plt.figure()
camera = Camera(fig)
c = 0
for line in f.readlines()[1:]:
    l = line.split(',')[1:-1]
    c=c+1
    xs = list(map(float, l[::2]))
    ys = list(map(float, l[1::2]))
    plt.scatter(xs, ys, color='red', s=3)
    camera.snap()

camera.animate().save('vis.gif')