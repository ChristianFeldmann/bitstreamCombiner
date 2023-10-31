/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <algorithm>
#include <string>
#include <tuple>

#include <common/Typedef.h>

namespace combiner::parser
{

/* This class provides the ability to read a byte array bit wise. Reading of ue(v) symbols is also
 * supported. This class can "read out" the emulation prevention bytes. This is enabled by default
 * but can be disabled if needed.
 */
class SubByteReader
{
public:
  SubByteReader() = default;
  SubByteReader(const ByteVector &inArr, size_t inArrOffset = 0);

  [[nodiscard]] bool more_rbsp_data() const;
  [[nodiscard]] bool byte_aligned() const;

  [[nodiscard]] bool payload_extension_present() const;
  [[nodiscard]] bool canReadBits(unsigned nrBits) const;

  [[nodiscard]] size_t nrBitsRead() const;
  [[nodiscard]] size_t nrBytesRead() const;
  [[nodiscard]] size_t nrBytesLeft() const;

  [[nodiscard]] ByteVector peekBytes(unsigned nrBytes) const;

  void disableEmulationPrevention() { this->skipEmulationPrevention = false; }

  bool       readFlag();
  uint64_t   readBits(size_t nrBits);
  ByteVector readBytes(size_t nrBytes);

  uint64_t readUEV();
  int64_t  readSEV();
  uint64_t readLEB128();
  uint64_t readUVLC();
  uint64_t readNS(uint64_t maxVal);
  int64_t  readSU(unsigned nrBits);

private:
  ByteVector byteVector;

  bool skipEmulationPrevention{true};

  // Move to the next byte and look for an emulation prevention 3 byte. Remove it (skip it) if
  // found. This function is just used by the internal reading functions.
  bool gotoNextByte();

  size_t posInBufferBytes{0};    // The byte position in the buffer
  size_t posInBufferBits{0};     // The sub byte (bit) position in the buffer (0...7)
  size_t numEmuPrevZeroBytes{0}; // The number of emulation prevention three bytes that were found
  size_t initialPosInBuffer{0};  // The position that was given when creating the sub reader
};

} // namespace combiner::parser