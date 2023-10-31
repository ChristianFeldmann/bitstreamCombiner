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

class FileSinkAnnexB
{
public:
  FileSinkAnnexB() = default;
  FileSinkAnnexB(const std::filesystem::path &filePath);

  // Get the raw data of the NAL unit without the start code
  void writeNALUnit(const ByteVector &nalData);

private:
  std::ofstream outputFile{};
};

} // namespace combiner
