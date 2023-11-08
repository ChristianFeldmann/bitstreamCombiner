/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include <common/Typedef.h>

namespace combiner
{

const ByteVector RAW_VPS_DATA = {0x0C, 0x01, 0xFF, 0xFF, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0xB0,
                                 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x5A, 0x11, 0x40, 0xC0,
                                 0x00, 0x00, 0x03, 0x00, 0x40, 0x00, 0x00, 0x07, 0xBA};

const ByteVector RAW_SPS_DATA = {0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0xB0, 0x00, 0x00, 0x03,
                                 0x00, 0x00, 0x03, 0x00, 0x5A, 0xA0, 0x07, 0x82, 0x00, 0x88, 0x7D,
                                 0xE5, 0x11, 0x64, 0x91, 0x4A, 0x70, 0x16, 0xA0, 0x20, 0x20, 0x22,
                                 0x80, 0x00, 0x00, 0x03, 0x00, 0x80, 0x00, 0x00, 0x0F, 0x78, 0x25,
                                 0x7B, 0x9F, 0x80, 0x02, 0x49, 0xF0, 0x00, 0x36, 0xEE, 0xC8};

const ByteVector RAW_PPS_DATA = {0xC0, 0xAC, 0x93, 0x83, 0xC9};

const ByteVector RAW_SLICE_HEADER_DATA_SLICE_0 = {0xAF, 0x12, 0x34, 0x6D, 0xE0};
const ByteVector RAW_SLICE_HEADER_DATA_SLICE_1 = {0xD0, 0x21, 0x49, 0xF1, 0x0B, 0xDE};
const ByteVector RAW_SLICE_HEADER_DATA_SLICE_2 = {0xE0, 0x24, 0xB7, 0xF8, 0x45, 0xC9, 0x60};

} // namespace combiner