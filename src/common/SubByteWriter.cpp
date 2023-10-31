/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "SubByteWriter.h"

#include <stdexcept>

namespace combiner::parser
{

void SubByteWriter::writeFlag(const bool flag)
{
  this->currentByte.set(this->posInCurrentByte, flag);

  if (this->posInCurrentByte == 0)
  {
    this->writeCurrentByteToVector();
    this->posInCurrentByte = 7;
  }
  else
    this->posInCurrentByte--;
}

void SubByteWriter::writeBits(const uint64_t value, const size_t nrBits)
{
  if (nrBits > 64)
    throw std::logic_error("Trying to write more than 64 bits at once to the bitstream.");
  if (nrBits == 0)
    return;

  for (auto bitPos = static_cast<int>(nrBits - 1); bitPos >= 0; --bitPos)
  {
    int        mask    = (1 << bitPos);
    const auto bitFlag = ((value & mask) != 0);

    this->writeFlag(bitFlag);
  }
}

void SubByteWriter::writeUEV(const uint64_t value)
{
  if (value == 0)
  {
    this->writeFlag(true);
    return;
  }

  unsigned golLength    = 1;
  auto     valueShifter = value + 1;

  while (1 != valueShifter)
  {
    valueShifter >>= 1;
    golLength += 2;
  }

  this->writeBits(0, (golLength >> 1));
  this->writeBits(value, ((golLength + 1) >> 1));
}

void SubByteWriter::writeSEV(const int64_t value)
{
}

void SubByteWriter::writeCurrentByteToVector()
{
  const auto byteValue = static_cast<char>(this->currentByte.to_ulong());
  this->byteVector.push_back(byteValue);
}

bool SubByteWriter::byte_aligned() const
{
  return this->posInCurrentByte == 7;
}

} // namespace combiner::parser