/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/Typedef.h>

#include <bitset>

namespace combiner::parser
{

class SubByteWriter
{
public:
  SubByteWriter() = default;

  [[nodiscard]] ByteVector finishWritingAddRbspAndGetData();

  void writeFlag(const bool flag);
  void writeBits(const uint64_t value, const size_t nrBits);
  void writeBytes(const ByteVector &bytes);

  void writeUEV(const uint64_t value);
  void writeSEV(const int64_t value);

  [[nodiscard]] bool byte_aligned() const;

private:
  ByteVector byteVector;

  bool skipEmulationPrevention{true};

  void writeCurrentByteToVector();

  std::bitset<8> currentByte{};

  // The sub byte (bit) position in the buffer (7...0). We are writing bits from most significant
  // (7) to least significant (0).
  std::size_t posInCurrentByte{7};

  int numEmuPrevZeroBytes{0}; // The number of emulation prevention three bytes that were found
};

} // namespace combiner::parser