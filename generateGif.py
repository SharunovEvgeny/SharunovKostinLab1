from celluloid import Camera
from matplotlib import pyplot as plt


f = open('resources/outputForPython.csv', 'r')

fig = plt.figure()
camera = Camera(fig)
for line in f.readlines()[1:]:
    spline = line.split(',')[1:-1]
    plt.scatter(list(map(float, spline[::2])), list(map(float, spline[1::2])), color='purple', s=3)
    camera.snap()
camera.animate().save('vis.gif')