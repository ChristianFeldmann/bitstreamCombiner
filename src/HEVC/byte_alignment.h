/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>
#include <common/SubByteWriter.h>

namespace combiner::parser::hevc::byte_alignment
{

// This is logically identical to rbsp_trailing_bits just with different names for the bits
void parse(SubByteReader &reader);
void write(SubByteWriter &writer);

} // namespace combiner::parser::hevc::byte_alignment
