/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <Typedef.h>

#include <filesystem>
#include <fstream>

namespace combiner
{

class FileSourceAnnexB
{
public:
  FileSourceAnnexB() = default;
  FileSourceAnnexB(const std::filesystem::path &path);

  ByteVector getNextNALUnit();

private:
  void seekToFirstNAL();

  std::ifstream        inputFile{};
  ByteVector           fileBuffer{};
  std::streamsize      fileBufferSize{};
  ByteVector::iterator fileBufferPosition{};
};

} // namespace combiner
