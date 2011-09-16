#!/usr/bin/env python

from markergen import *
import numpy as np
import matplotlib.pyplot as plt


def gen_grids():
    ret = []
    for i in range(256):
        ret.append(code_grid(get_code(i)))
    return ret


def rot_90(grid):

    g = [[0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0]]

    for y in range(6):
        for x in range(6):
            g[y][x] = grid[x][5-y]

    return g


def rot_180(grid):

    g = [[0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0]]

    for y in range(6):
        for x in range(6):
            g[y][x] = grid[5-y][5-x]

    return g


def rot_270(grid):

    g = [[0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0],
         [0, 0, 0, 0, 0, 0]]

    for y in range(6):
        for x in range(6):
            g[y][x] = grid[5-x][y]

    return g


def grid_rotate(grid, rot):

    if rot == 0:
        return grid

    elif rot == 1:
        return rot_90(grid)

    elif rot == 2:
        return rot_180(grid)

    elif rot == 3:
        return rot_270(grid)

    else:
        print "WHAT!?"


def hamming_distance(grid_a, grid_b):

    dist = 0

    for y in range(6):
        for x in range(6):
            if grid_a[y][x] != grid_b[y][x]:
                dist = dist + 1

    return dist


def gen_hamming_table():

    table = []
    for i in range(256):
        l = []
        for j in range(256):
            l.append([-1, -1, -1, -1])
        table.append(l)

    grids = gen_grids()

    for i in range(256):
        for j in range(i, 256):

            a = grids[i]
            b = grids[j]

            for k in range(4):
                dist = hamming_distance(a, grid_rotate(b, k))
                table[i][j][k] = dist

    return table



def gen_distribution(hamming_table):

    distribution = {}

    for i in range(256):
        for j in range(i, 256):
            for k in range(4):

                dist = hamming_table[i][j][k]

                if i == j and k == 0:
                    continue

                if dist == -1:
                    continue

                if not distribution.has_key(dist):
                    distribution[dist] = []

                distribution[dist].append( (i, j, k, dist) )

    return distribution


def plot_distribution(distribution):

    idx = distribution.keys()

    vals = []
    for i in idx:
        vals.append(len(distribution[i]))

    p1 = plt.bar(idx, vals, 0.2, color='r')

    plt.ylabel("No. of pair orientations")
    plt.xlabel("Hamming distance")

    plt.show()


def pairs_with_hamming_distance(hamming_dist, distribution, hamming_table):

    l = []

    if not distribution.has_key(hamming_dist):
        return l

    for i in range(256):
        for j in range(i, 256):
            for k in range(4):
                if hamming_table[i][j][k] == hamming_dist:
                    l.append( (i, j, k) )

    return l


def pairs_with_hamming_distance_lteq(hamming_dist, distribution, hamming_table):

    l = []

    for i in range(1, hamming_dist+1):
        if distribution.has_key(i):
            for item in distribution[i]:
                l.append(item)

    return l


def codes_involved(lteq):

    l = []

    for item in lteq:
        if not item[0] in l:
            l.append(item[0])
        if not item[1] in l:
            l.append(item[1])

    return sorted(l)



def lteq_filtered(hamming_dist, distribution, hamming_table):

    lteq = pairs_with_hamming_distance_lteq(hamming_dist, distribution, hamming_table)
    print lteq

    s = 0

    table = [0] * 256

    for i in lteq:
        if i[0] != i[1] or (i[0] == i[1] and i[2] != 0):
            s += 1
            table[i[0]] += 1
            table[i[1]] += 1

    return s



if __name__ == "__main__":

    t = gen_hamming_table()
    d = gen_distribution(t)

    x = []
    y = []

    for i in range(36):
        x.append(i)
        y.append(len(codes_involved(pairs_with_hamming_distance_lteq(i, d, t))))

    plt.bar(x, y)
    plt.show()

