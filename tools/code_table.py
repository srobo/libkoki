#!/usr/bin/env python

import sys

bad_codes = [ 2, 3, 8, 9, 11, 13, 17, 19,34, 42, 45, 46, 48, 51, 79,
              95, 96, 114, 126, 127, 159, 160, 178, 198, 200, 208,
              255 ]

def gen_forwards_table():

    table = {}
    count = 0
    for i in range(256):
        if not i in bad_codes:
            table[i] = count
            count += 1
        else:
            table[i] = -1
    return table


def gen_reverse_table(table):

    rev = {}
    items = table.items()

    for i in items:
        if i[1] != -1:
            rev[i[1]] = i[0]

    return rev


def gen_c_code():

    fwd = gen_forwards_table()

    c_code = "/* AUTO GENERATED -- DO NOT MODIFY */\n"
    c_code += "static int fwd_code_table[256] = {\n"

    for i in range(32):
        tmp = ""
        for j in range(8):
            tmp += "\t%d," % (fwd[i*8+j])
        if i == 31:
            tmp = tmp[:-1]
        c_code += tmp
        c_code += "\n"

    c_code += "};"

    return c_code


def gen_python_code():

    fwd = gen_forwards_table()
    rev = gen_reverse_table(fwd)

    python_code = "### AUTO GENERATED -- DO NOT MODIFY ###\n"
    python_code += "rev_code_table = [\n"

    count = 0
    for i in rev.keys():
        python_code += "\t%d," % (rev[i])
        count += 1
        if count == 8:
            count = 0
            python_code += "\n"

    python_code = python_code[:-1]
    python_code += "]\n"

    return python_code


if __name__ == '__main__':

    if len(sys.argv) != 2:
        print "Usage: %s (python|c)"
        sys.exit(1)

    if sys.argv[1] == "python":
        print gen_python_code()

    elif sys.argv[1] == "c":
        print gen_c_code()

    else:
        print "ERROR"
        sys.exit(1)

    sys.exit(0)
