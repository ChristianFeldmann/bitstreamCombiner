/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/Typedef.h>

#include <filesystem>
#include <fstream>
#include <optional>

namespace combiner
{

class FileSourceAnnexB
{
public:
  FileSourceAnnexB() = default;
  FileSourceAnnexB(const std::filesystem::path &path);

  // Get the raw data of the NAL unit without the start code
  ByteVector getNextNALUnit();

private:
  void seekToFirstNAL();
  void readNextBuffer();

  struct BorderCaseResult
  {
    int numberStartCodeBytesInLastBuffer{};
    int numberStartCodeBytesInNewBuffer{};
  };
  std::optional<BorderCaseResult>
  analyzeIfStartCodeOnBufferBoder(ByteVector last3BytesInLastBuffer);

  std::ifstream        inputFile{};
  ByteVector           fileBuffer{};
  ByteVector::iterator fileBufferPosition{};
  ByteVector::iterator fileBufferEnd{};

  bool canReadMoreData{true};
};

} // namespace combiner
