#!/usr/bin/env python

import numpy as np
import sys, math, os
import CrcMoose
import cairo
from code_table import *
import getopt

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


def get_pdf_surface(page_width, page_height, filename):

    surface = cairo.PDFSurface(filename, page_width, page_height)
    return surface


def finish_surface(surface):

    surface.finish()


def short_description(desc):
    if desc == "":
        return ""
    return "'%s'" % (desc)


def render_marker(surface, marker_num, overall_width, offset_x, offset_y,
                  desc="", show_text=1):

    fwd = gen_forwards_table()
    rev = gen_reverse_table(fwd)

    grid = code_grid(get_code(rev[marker_num]))

    marker_width = overall_width * (10.0/12.0)
    cell_width = marker_width / 10
    cell_grid_offset_x = cell_width * 2
    cell_grid_offset_y = cell_width * 2

    cr = cairo.Context(surface)

    # draw outline
    cr.set_line_width(1)
    grey = 0.7
    cr.set_source_rgb(grey, grey, grey)
    cr.rectangle(offset_x, offset_y, overall_width, overall_width)
    cr.stroke()

    # draw black border
    cr.set_source_rgb(0, 0, 0)
    cr.rectangle(offset_x + cell_width,
                 offset_y + cell_width,
                 marker_width, marker_width)
    cr.fill()

    # draw white grid background (i.e. zero grid)
    cr.set_source_rgb(1, 1, 1)
    cr.rectangle(offset_x + cell_width + cell_width * 2,
                 offset_y + cell_width + cell_width * 2,
                 marker_width * 0.6, marker_width * 0.6)
    cr.fill()

    #draw cells
    cr.set_source_rgb(0, 0, 0)
    for row in range(6):
        for col in range(6):

            if grid[row][col] == 1:
                #draw the 1 bit
                cr.rectangle(offset_x + cell_width + cell_width * 2 + col * cell_width,
                             offset_y + cell_width + cell_width * 2 + row * cell_width,
                             marker_width * 0.1, marker_width * 0.1)

            cr.fill()

    # write on marker
    if show_text:

        font_size = 6
        grey = 0.5

        cr.select_font_face('Sans')
        cr.set_font_size(font_size)
        cr.set_source_rgb(grey, grey, grey)

        cr.move_to(offset_x + cell_width + font_size, offset_y + cell_width + marker_width - font_size)
        cr.show_text('libkoki marker #%d (%s)   %s' % (marker_num, MARKER_VERSION, short_description(desc)))

    # put dot in top left
    cr.new_sub_path()
    grey = 0.2
    cr.set_source_rgb(grey, grey, grey)
    cr.arc(offset_x + cell_width + cell_width,
           offset_y + cell_width + cell_width,
           cell_width/8, 0, 2 * math.pi)
    cr.fill()




if __name__ == '__main__':

    if len(sys.argv) < 3:
        print "Usage: ./markergen.py [--4up] [--desc val] <code> <output_prefix>"
        sys.exit(1)

    optlist, args = getopt.getopt(sys.argv[1:], '', ['4up', 'desc='])

    if len(args) != 2:
        print "Usage: ./markergen.py [--4up] [--desc val] <code> <output_prefix>"
        sys.exit(1)

    CODE = int(args[0])
    OUTFNAME = "%s-%i.pdf" % (args[1], CODE)

    FOURUP = False
    DESC = ""

    # check for options
    for opt in optlist:

        if opt[0] == "--4up":
            FOURUP = True

        elif opt[0] == "--desc":
            DESC = opt[1]


    surface = get_pdf_surface(mm_to_pt(210), mm_to_pt(297), OUTFNAME)

    if not FOURUP:
        render_marker(surface, CODE, mm_to_pt(100),
                      mm_to_pt((210 - 100) / 2),
                      mm_to_pt((297 - 100) / 2), DESC)

    else:
        render_marker(surface, CODE, mm_to_pt(100),
                      mm_to_pt(5), mm_to_pt(10), DESC)
        render_marker(surface, CODE, mm_to_pt(100),
                      mm_to_pt(105), mm_to_pt(10), DESC)
        render_marker(surface, CODE, mm_to_pt(100),
                      mm_to_pt(5), mm_to_pt(110), DESC)
        render_marker(surface, CODE, mm_to_pt(100),
                      mm_to_pt(105), mm_to_pt(110), DESC)

    finish_surface(surface)
