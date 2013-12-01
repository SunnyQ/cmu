#!/usr/bin/python

import matplotlib
matplotlib.use('agg')
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import matplotlib.pyplot as plt
from optparse import OptionParser

def draw_surface(surfaces, figname, length):
    fig=plt.figure()
    ax = Axes3D(fig)
    for surface in surfaces:
        verts = [zip(surface[0], surface[1], surface[2])]
        poly = Poly3DCollection(verts, alpha=0.7)
        ax.add_collection3d(poly)
    ax.set_xlim3d(0, length)
    ax.set_ylim3d(0, length)
    ax.set_zlim3d(0, length)
    if figname is not None:
        plt.savefig(figname)
    plt.show()

def read_points(inputfilename):
    surfaces = []
    f = open(inputfilename, 'r')
    m = int(f.readline()[:-1])
    for i in range(m):
        l = f.readline().split()
        x = []
        y = []
        z = []
        for j in range(0, len(l), 3):
            x.append(float(l[j]))
            y.append(float(l[j+1]))
            z.append(float(l[j+2]))
        surfaces.append((x, y, z))
    f.close()
    return surfaces

def main():
    parser = OptionParser(usage="usage: %prog [options] input_filename")
    parser.add_option("-o", "--output",
                      dest="output",
                      default="a.png",
                      help="The output filename of the figure")
    parser.add_option("-l", "--length",
                      dest="length",
                      type="float",
                      default=1.,
                      help="The length of the initial square")
    (options, args) = parser.parse_args()

    if len(args) != 1:
        parser.error("wrong number of arguments")

    inputfilename = args[0]
    surfaces = read_points(inputfilename)

    if len(surfaces) > 0:
        draw_surface(surfaces, options.output, options.length)

main()
