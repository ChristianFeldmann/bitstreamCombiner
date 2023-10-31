/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "rbsp_trailing_bits.h"

#include <stdexcept>

namespace combiner::parser::hevc::rbsp_trailing_bits
{

void parse(SubByteReader &reader)
{
  const auto rbsp_stop_one_bit = reader.readFlag();
  if (!rbsp_stop_one_bit)
    throw std::runtime_error("rbsp_stop_one_bit must be set");

  while (!reader.byte_aligned())
  {
    const auto rbsp_alignment_zero_bit = reader.readFlag();
    if (rbsp_alignment_zero_bit)
      throw std::runtime_error("rbsp_alignment_zero_bit must not be set");
  }
}

void write(SubByteWriter &writer)
{
  const auto rbsp_stop_one_bit = true;
  writer.writeFlag(rbsp_stop_one_bit);

  while (!writer.byte_aligned())
  {
    const auto rbsp_alignment_zero_bit = false;
    writer.writeFlag(rbsp_alignment_zero_bit);
  }
}

} // namespace combiner::parser::hevc::rbsp_trailing_bits
