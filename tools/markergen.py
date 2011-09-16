#!/usr/bin/env python

import numpy as np
import sys, math, os
import CrcMoose
import cairo
from code_table import *

MARKER_VERSION = "v0.5"

G = np.matrix([[1, 1, 0, 1],
               [1, 0, 1, 1],
               [1, 0, 0, 0],
               [0, 1, 1, 1],
               [0, 1, 0, 0],
               [0, 0, 1, 0],
               [0, 0, 0, 1]])

H = np.matrix([[1, 0, 1, 0, 1, 0, 1],
               [0, 1, 1, 0, 0, 1, 1],
               [0, 0, 0, 1, 1, 1, 1]])

R = np.matrix([[0, 0, 1, 0, 0, 0, 0],
               [0, 0, 0, 0, 1, 0, 0],
               [0, 0, 0, 0, 0, 1, 0],
               [0, 0, 0, 0, 0, 0, 1]])



def hamming_encode(l):

    p = np.matrix([l]).T

    tmp = G * p
    output = tmp.A % 2

    return output.T[0]



def hamming_syndrome(l):

    r = np.matrix([l]).T

    z = H * r
    output = z.A % 2

    return output.T[0]



def hamming_correct(l, z):

    syndrome_val = z[0] + z[1]*2 + z[2]*4

    # no errors, return original
    if (syndrome_val == 0):
        return l

    # flip the error bit
    l[syndrome_val-1] = (l[syndrome_val-1] + 1) % 2

    return l



def hamming_decode(l):

    syndrome = hamming_syndrome(l)
    corrected = hamming_correct(l, syndrome)

    pr = R * np.matrix([corrected]).T

    return pr.T.A[0]



def get_code(marker_num):

    CRC12 = CrcMoose.CrcAlgorithm(
        name         = "CRC-12",
        width        = 12,
        polynomial   = (12, 11, 3, 2, 1, 0),
        seed         = 0,
        lsbFirst     = True,
        xorMask      = 0)


    marker_chr = chr(int((marker_num+1) % 256))
    crc = CRC12.calcString(marker_chr)

    code = (crc << 8) | marker_num
    # print "Marker No:", marker_num, "\t\t(", hex(marker_num), ")"
    # print "      CRC:", crc, "\t(", hex(crc), ")"
    # print "     Code:", code, "\t(", hex(code), ")"

    return code


def code_to_lists(code):

    output = []

    for i in range(5):

        l = []

        for j in range(4):

            mask = 0x1 << (i*4+j)
            tmp = code & mask
            bit = 1
            if (tmp == 0):
                bit = 0

            l.append(bit)

        output.append(l)

    return output



def encoded_lists(l):

    return map(hamming_encode, l)



def code_grid(code):

    blocks = encoded_lists(code_to_lists(code))
    cell = 0

    grid = [[-1, -1, -1, -1, -1, -1],
            [-1, -1, -1, -1, -1, -1],
            [-1, -1, -1, -1, -1, -1],
            [-1, -1, -1, -1, -1, -1],
            [-1, -1, -1, -1, -1, -1],
            [-1, -1, -1, -1, -1, -1]]

    for i in range(7):
        for j in range(5):
            grid[cell / 6][cell % 6] = blocks[j][i]
            cell = cell + 1

    return grid


def print_grid(grid):
    sys.stdout.write("# # # # # # # # # #\n")
    sys.stdout.write("# # # # # # # # # #\n")
    for i in range(6):
        sys.stdout.write("# # ")
        for j in range(6):
            if grid[i][j] == 1:
                sys.stdout.write("# ")
            else:
                sys.stdout.write("  ")
        sys.stdout.write("# #\n")
    sys.stdout.write("# # # # # # # # # #\n")
    sys.stdout.write("# # # # # # # # # #\n")




def mm_to_in(x):
    return x * 0.0393700787


def mm_to_pt(x):
    return 72 * mm_to_in(x)


def render_marker_to_pdf(marker_num, outfname, marker_width, page_width,
                         page_height, show_text=1):

    fwd = gen_forwards_table()
    rev = gen_reverse_table(fwd)

    marker_offset_x = (page_width - marker_width) / 2
    marker_offset_y = (page_height - marker_width) / 2
    cell_width = marker_width / 10
    cell_grid_offset_x = cell_width * 2
    cell_grid_offset_y = cell_width * 2

    grid = code_grid(get_code(rev[marker_num]))

    # setup a place to draw
    surface = cairo.PDFSurface("%s" % outfname,
                               page_width, page_height)

    # get a context
    cr = cairo.Context(surface)

    # draw border
    cr.set_source_rgb(0, 0, 0)
    cr.rectangle(marker_offset_x, marker_offset_y,
                 marker_width, marker_width)
    cr.fill()

    #draw centre
    cr.set_source_rgb(1, 1, 1)
    cr.rectangle(marker_offset_x + cell_grid_offset_x,
                 marker_offset_y + cell_grid_offset_y,
                 marker_width * 0.6, marker_width * 0.6)
    cr.fill()

    #draw cells
    cr.set_source_rgb(0, 0, 0)
    for row in range(6):
        for col in range(6):

            if grid[row][col] == 1:
                #draw the circle
                cr.arc(marker_offset_x + cell_grid_offset_x + col * cell_width + cell_width/2,
                       marker_offset_y + cell_grid_offset_y + row * cell_width +cell_width/2,
                       cell_width/2, 0, 2 * math.pi)
                cr.rectangle(marker_offset_x + cell_grid_offset_x + col * cell_width,
                             marker_offset_y + cell_grid_offset_y + row * cell_width,
                             marker_width * 0.1, marker_width * 0.1)

            cr.fill()

    if show_text:

        font_size = 6
        grey = 0.5

        cr.select_font_face('Sans')
        cr.set_font_size(font_size)
        cr.set_source_rgb(grey, grey, grey)

        cr.move_to(marker_offset_x + font_size, marker_offset_y + marker_width - font_size)
        cr.show_text('libkoki marker #%d (%s)' % (marker_num, MARKER_VERSION))

    surface.finish()





if __name__ == '__main__':

    if len(sys.argv) != 3:
        print "Usage: ./markergen.py <code> <output_prefix>"
        sys.exit(1)

    CODE = int(sys.argv[1])
    OUTFNAME = "%s-%i.pdf" % (sys.argv[2], CODE)

    render_marker_to_pdf( CODE, OUTFNAME,
                         mm_to_pt(83),
                         mm_to_pt(210),
                         mm_to_pt(297))
