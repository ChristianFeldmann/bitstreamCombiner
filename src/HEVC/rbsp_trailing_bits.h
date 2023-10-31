/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>
#include <common/SubByteWriter.h>

namespace combiner::parser::hevc::rbsp_trailing_bits
{

void parse(SubByteReader &reader);
void write(SubByteWriter &writer);

} // namespace combiner::parser::hevc::rbsp_trailing_bits
