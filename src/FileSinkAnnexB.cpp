/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "FileSinkAnnexB.h"

namespace combiner
{

FileSinkAnnexB::FileSinkAnnexB(const std::filesystem::path &filePath)
{
  this->outputFile.open(filePath, std::ios_base::binary);

  if (!this->outputFile.is_open())
    throw std::runtime_error("Error opening output file " + filePath.string());
}

void FileSinkAnnexB::writeNALUnit(const ByteVector &nalData)
{
  if (!this->outputFile.is_open())
    throw std::runtime_error("Output file not open for writing");

  this->outputFile.put(0);
  this->outputFile.put(0);
  this->outputFile.put(0);
  this->outputFile.put(1);
  this->outputFile.write(nalData.data(), nalData.size());
}

} // namespace combiner
