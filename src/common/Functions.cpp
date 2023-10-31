/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "Functions.h"

namespace combiner::parser
{

std::string convertSliceCountsToString(const std::map<std::string, unsigned int> &sliceCounts)
{
  std::string text;
  for (auto const &key : sliceCounts)
  {
    text += key.first;
    const auto value = key.second;
    if (value > 1)
      text += "(" + std::to_string(value) + ")";
    text += " ";
  }
  return text;
}

std::vector<std::string> splitX26XOptionsString(const std::string str, const std::string seperator)
{
  std::vector<std::string> splitStrings;

  std::string::size_type prev_pos = 0;
  std::string::size_type pos      = 0;
  while ((pos = str.find(seperator, pos)) != std::string::npos)
  {
    auto substring = str.substr(prev_pos, pos - prev_pos);
    splitStrings.push_back(substring);
    prev_pos = pos + seperator.size();
    pos++;
  }
  splitStrings.push_back(str.substr(prev_pos, pos - prev_pos));

  return splitStrings;
}

size_t getStartCodeOffset(const ByteVector &data)
{
  unsigned readOffset = 0;
  if (data.at(0) == (char)0 && data.at(1) == (char)0 && data.at(2) == (char)1)
    readOffset = 3;
  else if (data.at(0) == (char)0 && data.at(1) == (char)0 && data.at(2) == (char)0 &&
           data.at(3) == (char)1)
    readOffset = 4;
  return readOffset;
}

} // namespace combiner::parser
