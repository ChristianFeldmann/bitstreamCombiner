/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "FileSourceAnnexB.h"

namespace combiner
{

const auto BUFFERSIZE = 500'000;
const auto STARTCODE  = {char(0), char(0), char(1)};

FileSourceAnnexB::FileSourceAnnexB(const std::filesystem::path &filePath)
{
  this->inputFile.open(filePath);

  if (!this->inputFile.is_open())
    throw std::runtime_error("Error opening input file " + filePath.string());

  this->fileBuffer.resize(BUFFERSIZE);
  this->inputFile.read(this->fileBuffer.data(), BUFFERSIZE);
  this->fileBufferSize = this->inputFile.gcount();

  this->seekToFirstNAL();
}

void FileSourceAnnexB::seekToFirstNAL()
{
  this->fileBufferPosition = std::search(
      this->fileBuffer.begin(), this->fileBuffer.end(), std::begin(STARTCODE), std::end(STARTCODE));

  if (this->fileBufferPosition == this->fileBuffer.end())
    throw std::runtime_error("Unabel to find any NAL units in first 500k of input file. Aborting.");
}

ByteVector FileSourceAnnexB::getNextNALUnit()
{
  const auto nextStartCodePosition = std::search(this->fileBufferPosition + 3,
                                                 this->fileBuffer.end(),
                                                 std::begin(STARTCODE),
                                                 std::end(STARTCODE));
  if (nextStartCodePosition != this->fileBuffer.end())
  {
    const auto start         = this->fileBufferPosition;
    const auto end           = nextStartCodePosition;
    this->fileBufferPosition = nextStartCodePosition;
    return ByteVector(start, end);
  }

  // The next start code is not in this buffer. We have to load data
  // from the file

  auto startCodeFound = false;
  while (!startCodeFound)
  {

    if (nextStartCodePosition != this->fileBuffer.end())
    {
      const auto start         = this->fileBufferPosition;
      const auto end           = nextStartCodePosition;
      this->fileBufferPosition = nextStartCodePosition;
      return ByteVector(start, end);
    }
    else
    {
      // Handle special case ...
    }
  }

  return {};
}

} // namespace combiner
