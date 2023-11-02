/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "SubByteWriter.h"

#include <stdexcept>

namespace combiner::parser
{

ByteVector SubByteWriter::finishWritingAndGetData()
{
  if (!this->byte_aligned())
    this->writeCurrentByteToVector();

  return this->byteVector;
}

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
  const auto valuePlusOne = value + 1;

  auto nrBits       = 0;
  auto valueShifter = valuePlusOne;
  while (valueShifter > 0)
  {
    valueShifter >>= 1;
    nrBits++;
  }

  const auto nrStartingZeroBits = nrBits - 1;

  this->writeBits(0, nrStartingZeroBits);
  this->writeBits(valuePlusOne, nrBits);
}

void SubByteWriter::writeSEV(const int64_t value)
{
  if (value < 0)
    this->writeUEV(std::abs(value * 2));
  else if (value > 0)
    this->writeUEV(value * 2 - 1);
  else
    this->writeUEV(0);
}

void SubByteWriter::writeCurrentByteToVector()
{
  const auto byteValue = static_cast<char>(this->currentByte.to_ulong());

  if (this->writeEmulationPrevention && this->numEmuPrevZeroBytes == 2 &&
      (byteValue == (char)0 || byteValue == (char)1 || byteValue == (char)2 ||
       byteValue == (char)3))
  {
    this->byteVector.push_back((char)3);
    this->numEmuPrevZeroBytes = 0;
  }

  if (byteValue == (char)0)
    this->numEmuPrevZeroBytes++;
  else
    this->numEmuPrevZeroBytes = 0;

  this->byteVector.push_back(byteValue);
  this->currentByte.reset();
}

bool SubByteWriter::byte_aligned() const
{
  return this->posInCurrentByte == 7;
}

} // namespace combiner::parser