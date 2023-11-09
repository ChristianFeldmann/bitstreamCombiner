/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "FileSourceAnnexB.h"

#include <algorithm>
#include <array>

namespace combiner
{

namespace
{

constexpr auto BUFFERSIZE     = 500'000;
constexpr auto STARTCODE_SIZE = 3;
constexpr auto STARTCODE      = {char(0), char(0), char(1)};

void removeTailingZeroByte(ByteVector &data)
{
  if (data.back() == 0)
    data.pop_back();
}

} // namespace

FileSourceAnnexB::FileSourceAnnexB(const std::filesystem::path &filePath)
{
  this->inputFile.open(filePath, std::ios_base::binary);

  if (!this->inputFile.is_open())
    throw std::runtime_error("Error opening input file " + filePath.string());

  this->fileBuffer.resize(BUFFERSIZE);
  this->readNextBuffer();
  this->seekToFirstNAL();
}

void FileSourceAnnexB::seekToFirstNAL()
{
  this->fileBufferPosition = std::search(
      this->fileBuffer.begin(), this->fileBufferEnd, std::begin(STARTCODE), std::end(STARTCODE));

  if (this->fileBufferPosition == this->fileBufferEnd)
    throw std::runtime_error("Unabel to find any NAL units in first 500k of input file. Aborting.");

  this->fileBufferPosition += STARTCODE_SIZE;
}

void FileSourceAnnexB::readNextBuffer()
{
  this->inputFile.read(reinterpret_cast<char *>(this->fileBuffer.data()), BUFFERSIZE);
  const auto bytesRead     = this->inputFile.gcount();
  this->fileBufferPosition = this->fileBuffer.begin();
  this->fileBufferEnd      = this->fileBuffer.begin() + bytesRead;
  this->canReadMoreData    = (bytesRead == BUFFERSIZE);
}

ByteVector FileSourceAnnexB::getNextNALUnit()
{
  if (!this->canReadMoreData && this->fileBufferPosition == this->fileBufferEnd)
    return {};

  ByteVector nalData;
  while (true)
  {
    const auto nextStartCodePosition = std::search(
        this->fileBufferPosition, this->fileBufferEnd, std::begin(STARTCODE), std::end(STARTCODE));
    if (nextStartCodePosition != this->fileBufferEnd)
    {
      nalData.insert(nalData.end(), this->fileBufferPosition, nextStartCodePosition);
      this->fileBufferPosition = nextStartCodePosition + STARTCODE_SIZE;
      removeTailingZeroByte(nalData);
      return nalData;
    }

    nalData.insert(nalData.end(), this->fileBufferPosition, this->fileBufferEnd);

    if (!this->canReadMoreData)
    {
      this->fileBufferPosition = this->fileBufferEnd;
      return nalData;
    }

    const auto last2BytesInLastBuffer = ByteVector(this->fileBufferEnd - 2, this->fileBufferEnd);
    this->readNextBuffer();

    if (const auto result = this->analyzeIfStartCodeOnBufferBoder(last2BytesInLastBuffer))
    {
      nalData.erase(nalData.end() - result->numberStartCodeBytesInLastBuffer, nalData.end());
      this->fileBufferPosition += result->numberStartCodeBytesInNewBuffer;
      removeTailingZeroByte(nalData);
      return nalData;
    }
  }
}

std::optional<FileSourceAnnexB::BorderCaseResult>
FileSourceAnnexB::analyzeIfStartCodeOnBufferBoder(ByteVector last2BytesInLastBuffer)
{
  auto borderData = last2BytesInLastBuffer;

  const auto copyToInNewBuffer = std::min(this->fileBufferPosition + 2, this->fileBufferEnd);
  borderData.insert(borderData.end(), this->fileBufferPosition, copyToInNewBuffer);

  const auto borderDataStartCodePos =
      std::search(borderData.begin(), borderData.end(), std::begin(STARTCODE), std::end(STARTCODE));

  if (borderDataStartCodePos == borderData.end())
    return {};

  BorderCaseResult result;
  const auto distanceFromBeginning = std::distance(borderData.begin(), borderDataStartCodePos);
  result.numberStartCodeBytesInLastBuffer = 2 - static_cast<int>(distanceFromBeginning);
  result.numberStartCodeBytesInNewBuffer  = static_cast<int>(distanceFromBeginning);
  return result;
}

} // namespace combiner
