/******************************************************************************\
Copyright (C) 2015 Peter Bosch

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
\******************************************************************************/

/**
 * @file crt/numfmt.h
 *
 * Part of posnk kernel
 *
 * Written by Peter Bosch <peterbosc@gmail.com>
 *
 */

#ifndef __numfmt__
#define __numfmt__
#include <stddef.h>
#include <stdint.h>

/**
 * Pad the leading space on the number with zeroes
 */
#define NF_ZEROPAD	(1 << 0)

/**
 * Show a + in front of positive numbers
 */
#define NF_SGNPLUS	(1 << 1)

void numfmt_signed(	intmax_t num,
                       int flags,
                       int width,
                       int base,
                       char *str,
                       size_t str_len);
void numfmt_unsigned(	uintmax_t num,
                         int flags,
                         int width,
                         unsigned int base,
                         char *str,
                         size_t str_len);

#endif
