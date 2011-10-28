/* Copyright 2011 Chris Kirkham

   This file is part of libkoki

   libkoki is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   libkoki is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with libkoki.  If not, see <http://www.gnu.org/licenses/>. */

/**
 * @file  crc12.c
 * @brief Implementation for 12-bit CRC
 */

#include <stdint.h>

#define WIDTH 12
#define SEED  0
#define XOR_MASK 0

#define POLY_LEN 6
static uint8_t polynomial[] = {12, 11, 3, 2, 1, 0};

/**
 * @brief reflects (i.e. reverses) the bits in the given input \c num
 *
 * @param num    the input to reverese/reflect
 * @param width  the width of the input in bits (e.g. if reversing 7
 *               bits, use 7)
 * @return       the input \c num, reversed
 */
static uint16_t reflect(uint16_t num, uint8_t width)
{

	uint16_t ret = 0;

	for (int i=0; i<width; i++){
		ret |= ((num >> i) & 0x1) << (width-1-i);
	}

	return ret;

}



/**
 * @brief computes the 12-bit CRC on an 8-bit input
 *
 * Based on Ray Burr's CrcMoose.py (included).  Please see CrcMoose.py
 * for MIT license, copyright, etc... Thanks Ray!
 *
 * Note that this fonction only operates on a single value.
 *
 * @param input  the number/char/etc... to CRC
 * @return       the 12-bit CRC of \c input
 */
uint16_t koki_crc12(uint8_t input)
{

	uint16_t value, bit, out_bit;
	uint16_t bit_mask, poly_mask, word;
	uint16_t /* in_bit_mask,*/ out_bit_mask;

	bit_mask = (1 << WIDTH) - 1;

	word = 0;
	for (int i=0; i<POLY_LEN; i++)
		word |= (1 << polynomial[i]);

	poly_mask = word & bit_mask;

	// lsb first
	poly_mask = reflect(poly_mask, WIDTH);

	//in_bit_mask = 1 << (WIDTH - 1);
	out_bit_mask = 1;

	value = 0;


	// take bit
	for (uint8_t i=0; i<8; i++){

		bit = ((input & (1 << i)) >> i) & 0x1;

		out_bit = ((value & out_bit_mask) != 0) & (0x1);
		value = value >> 1;
		value = value & bit_mask;

		if (out_bit ^ (bit & 0x1))
			value ^= poly_mask;

	}

	return (value ^ XOR_MASK) & 0xFFF;

}
