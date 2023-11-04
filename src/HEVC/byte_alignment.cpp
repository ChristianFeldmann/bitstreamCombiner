/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "byte_alignment.h"

#include <stdexcept>

namespace combiner::parser::hevc::byte_alignment
{

void parse(SubByteReader &reader)
{
  const auto alignment_bit_equal_to_one = reader.readFlag();
  if (!alignment_bit_equal_to_one)
    throw std::runtime_error("alignment_bit_equal_to_one must be set");

  while (!reader.byte_aligned())
  {
    const auto alignment_bit_equal_to_zero = reader.readFlag();
    if (alignment_bit_equal_to_zero)
      throw std::runtime_error("alignment_bit_equal_to_zero must not be set");
  }
}

void write(SubByteWriter &writer)
{
  const auto alignment_bit_equal_to_one = true;
  writer.writeFlag(alignment_bit_equal_to_one);

  while (!writer.byte_aligned())
  {
    const auto alignment_bit_equal_to_zero = false;
    writer.writeFlag(alignment_bit_equal_to_zero);
  }
}

} // namespace combiner::parser::hevc::byte_alignment
