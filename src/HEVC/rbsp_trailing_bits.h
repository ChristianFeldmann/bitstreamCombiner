/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>

namespace combiner::parser::hevc
{

class rbsp_trailing_bits
{
public:
  rbsp_trailing_bits()  = default;
  ~rbsp_trailing_bits() = default;
  void parse(SubByteReader &reader);
};

} // namespace combiner::parser::hevc
