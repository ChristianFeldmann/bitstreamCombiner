/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "SubByteReader.h"

#include <bitset>
#include <cassert>
#include <stdexcept>

namespace combiner::parser
{

SubByteReader::SubByteReader(const ByteVector &inArr, size_t inArrOffset)
    : byteVector(inArr), posInBufferBytes(inArrOffset), initialPosInBuffer(inArrOffset){};

bool SubByteReader::readFlag()
{
  const auto value = this->readBits(1);
  return value != 0;
}

uint64_t SubByteReader::readBits(size_t nrBits)
{
  uint64_t out        = 0;
  auto     nrBitsRead = nrBits;

  // The return unsigned int is of depth 64 bits
  if (nrBits > 64)
    throw std::logic_error("Trying to read more than 64 bits at once from the bitstream.");
  if (nrBits == 0)
    return 0;

  while (nrBits > 0)
  {
    if (this->posInBufferBits == 8 && nrBits != 0)
    {
      // We read all bits we could from the current byte but we need more. Go to
      // the next byte.
      if (!this->gotoNextByte())
        // We are at the end of the buffer but we need to read more. Error.
        throw std::logic_error("Error while reading annexB file. Trying to "
                               "read over buffer boundary.");
    }

    // How many bits can be gotton from the current byte?
    auto curBitsLeft = 8 - this->posInBufferBits;

    size_t readBits; // Nr of bits to read
    size_t offset;   // Offset for reading from the right
    if (nrBits >= curBitsLeft)
    {
      // Read "curBitsLeft" bits
      readBits = curBitsLeft;
      offset   = 0;
    }
    else
    {
      // Read "nrBits" bits
      assert(nrBits < 8 && nrBits < curBitsLeft);
      readBits = nrBits;
      offset   = curBitsLeft - nrBits;
    }

    // Shift output value so that the new bits fit
    out = out << readBits;

    char c   = this->byteVector[this->posInBufferBytes];
    c        = c >> offset;
    int mask = ((1 << readBits) - 1);

    // Write bits to output
    out += (c & mask);

    // Update counters
    nrBits -= readBits;
    this->posInBufferBits += readBits;
  }

  return out;
}

ByteVector SubByteReader::readBytes(size_t nrBytes)
{
  if (this->posInBufferBits != 0 && this->posInBufferBits != 8)
    throw std::logic_error("When reading bytes from the bitstream, it must be byte aligned.");

  if (this->posInBufferBits == 8)
    if (!this->gotoNextByte())
      // We are at the end of the buffer but we need to read more. Error.
      throw std::logic_error("Error while reading annexB file. Trying to read "
                             "over buffer boundary.");

  ByteVector retVector;
  for (unsigned i = 0; i < nrBytes; i++)
  {
    auto c = this->byteVector[this->posInBufferBytes];
    retVector.push_back(c);

    if (!this->gotoNextByte())
    {
      if (i < nrBytes - 1)
        throw std::logic_error("Error while reading annexB file. Trying to "
                               "read over buffer boundary.");
    }
  }

  return retVector;
}

uint64_t SubByteReader::readUEV()
{
  {
    if (this->readBits(1) == 1)
      return 0;
  }

  // Get the length of the golomb
  unsigned golLength = 0;
  while (true)
  {
    golLength++;
    if (this->readBits(1) == 1)
      break;
  }

  const auto golBits = this->readBits(golLength);
  // Exponential part
  const auto val = golBits + (uint64_t(1) << golLength) - 1;

  return val;
}

int64_t SubByteReader::readSEV()
{
  const auto val = this->readUEV();
  if (val % 2 == 0)
    return -int64_t((val + 1) / 2);
  else
    return int64_t((val + 1) / 2);
}

uint64_t SubByteReader::readLEB128()
{
  // We will read full bytes (up to 8)
  // The highest bit indicates if we need to read another bit. The rest of the
  // bits is added to the counter (shifted accordingly) See the AV1 reading
  // specification
  uint64_t value = 0;
  for (unsigned i = 0; i < 8; i++)
  {
    const auto leb128_byte = this->readBits(8);
    value |= ((leb128_byte & 0x7f) << (i * 7));
    if (!(leb128_byte & 0x80))
      break;
  }
  return value;
}

uint64_t SubByteReader::readUVLC()
{
  auto leadingZeros = 0u;

  while (true)
  {
    const auto done = this->readBits(1);
    if (done > 0)
      break;
    leadingZeros++;
  }

  if (leadingZeros >= 32)
    return ((uint64_t)1 << 32) - 1;
  const auto value = this->readBits(leadingZeros);

  return value + ((uint64_t)1 << leadingZeros) - 1;
}

uint64_t SubByteReader::readNS(uint64_t maxVal)
{
  if (maxVal == 0)
    return 0;

  // FloorLog2
  uint64_t floorVal;
  {
    auto     x = maxVal;
    unsigned s = 0;
    while (x != 0)
    {
      x = x >> 1;
      s++;
    }
    floorVal = s - 1;
  }

  auto w = floorVal + 1;
  auto m = (uint64_t(1) << w) - maxVal;

  auto v = this->readBits(w - 1);
  if (v < m)
    return v;

  const auto extra_bit = this->readBits(1);
  return (v << 1) - m + extra_bit;
}

int64_t SubByteReader::readSU(unsigned nrBits)
{
  const auto value    = readBits(nrBits);
  const int  signMask = 1 << (nrBits - 1);
  if (value & signMask)
  {
    const auto subValue = int64_t(value) - 2 * signMask;
    return subValue;
  }
  return int64_t(value);
}

/* Is there more data? There is no more data if the next bit is the terminating
 * bit and all following bits are 0. */
bool SubByteReader::more_rbsp_data() const
{
  auto posBytes            = this->posInBufferBytes;
  auto posBits             = this->posInBufferBits;
  auto terminatingBitFound = false;
  if (posBits == 8)
  {
    posBytes++;
    posBits = 0;
  }
  else if (posBits != 0)
  {
    // Check the remainder of the current byte
    unsigned char c = this->byteVector[posBytes];
    if (c & (1 << (7 - posBits)))
      terminatingBitFound = true;
    else
      return true;
    posBits++;
    while (posBits != 8)
    {
      if (c & (1 << (7 - posBits)))
        // Only zeroes should follow
        return true;
      posBits++;
    }
    posBytes++;
  }
  while (posBytes < (unsigned int)this->byteVector.size())
  {
    unsigned char c = this->byteVector[posBytes];
    if (terminatingBitFound && c != 0)
      return true;
    else if (!terminatingBitFound && (c == 128))
      terminatingBitFound = true;
    else if (c != 0)
      return true;
    posBytes++;
  }
  if (!terminatingBitFound)
    return true;
  return false;
}

bool SubByteReader::byte_aligned() const
{
  return (this->posInBufferBits == 0 || this->posInBufferBits == 8);
}

/* Is there more data? If the current position in the sei_payload() syntax
 structure is not the position of the last (least significant, right- most)
 bit that is equal to 1 that is less than 8 * payloadSize bits from the
 beginning of the syntax structure (i.e., the position of the
 payload_bit_equal_to_one syntax element), the return value of
 payload_extension_present( ) is equal to TRUE.
*/
bool SubByteReader::payload_extension_present() const
{
  // TODO: What is the difference to this?
  return more_rbsp_data();
}

bool SubByteReader::canReadBits(unsigned nrBits) const
{
  if (this->posInBufferBytes == this->byteVector.size())
    return false;

  assert(this->posInBufferBits <= 8);
  const auto curBitsLeft = 8 - this->posInBufferBits;
  assert(this->byteVector.size() > this->posInBufferBytes);
  const auto entireBytesLeft  = this->byteVector.size() - this->posInBufferBytes - 1;
  const auto nrBitsLeftToRead = curBitsLeft + entireBytesLeft * 8;

  return nrBits <= nrBitsLeftToRead;
}

size_t SubByteReader::nrBitsRead() const
{
  return (this->posInBufferBytes - this->initialPosInBuffer) * 8 + this->posInBufferBits;
}

size_t SubByteReader::nrBytesRead() const
{
  return this->posInBufferBytes - this->initialPosInBuffer + (this->posInBufferBits != 0 ? 1 : 0);
}

size_t SubByteReader::nrBytesLeft() const
{
  if (this->byteVector.size() <= this->posInBufferBytes)
    return 0;
  return this->byteVector.size() - this->posInBufferBytes - 1;
}

ByteVector SubByteReader::peekBytes(unsigned nrBytes) const
{
  if (this->posInBufferBits != 0 && this->posInBufferBits != 8)
    throw std::logic_error("When peeking bytes from the bitstream, it must be byte aligned.");

  auto pos = this->posInBufferBytes;
  if (this->posInBufferBits == 8)
    pos++;

  if (pos + nrBytes > this->byteVector.size())
    throw std::logic_error("Not enough data in the input to peek that far");

  return ByteVector(this->byteVector.begin() + pos, this->byteVector.begin() + pos + nrBytes);
}

bool SubByteReader::gotoNextByte()
{
  // Before we go to the neyt byte, check if the last (current) byte is a zero
  // byte.
  if (this->posInBufferBytes >= unsigned(this->byteVector.size()))
    throw std::out_of_range("Reading out of bounds");
  if (this->byteVector[this->posInBufferBytes] == (char)0)
    this->numEmuPrevZeroBytes++;

  // Skip the remaining sub-byte-bits
  this->posInBufferBits = 0;
  // Advance pointer
  this->posInBufferBytes++;

  if (this->posInBufferBytes >= (unsigned int)this->byteVector.size())
    // The next byte is outside of the current buffer. Error.
    return false;

  if (this->skipEmulationPrevention)
  {
    if (this->numEmuPrevZeroBytes == 2 && this->byteVector[this->posInBufferBytes] == (char)3)
    {
      // The current byte is an emulation prevention 3 byte. Skip it.
      this->posInBufferBytes++; // Skip byte

      if (this->posInBufferBytes >= (unsigned int)this->byteVector.size())
      {
        // The next byte is outside of the current buffer. Error
        return false;
      }

      // Reset counter
      this->numEmuPrevZeroBytes = 0;
    }
    else if (this->byteVector[this->posInBufferBytes] != (char)0)
      // No zero byte. No emulation prevention 3 byte
      this->numEmuPrevZeroBytes = 0;
  }

  return true;
}

} // namespace combiner::parser