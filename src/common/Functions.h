/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/Typedef.h>

#include <map>
#include <string>
#include <vector>

namespace combiner::parser
{

template <typename T> std::string formatArray(std::string variableName, T idx)
{
  return variableName + "[" + std::to_string(idx) + "]";
}

template <typename T1, typename T2>
std::string formatArray(std::string variableName, T1 idx1, T2 idx2)
{
  return variableName + "[" + std::to_string(idx1) + "][" + std::to_string(idx2) + "]";
}

template <typename T1, typename T2, typename T3>
std::string formatArray(std::string variableName, T1 idx1, T2 idx2, T3 idx3)
{
  return variableName + "[" + std::to_string(idx1) + "][" + std::to_string(idx2) + "][" +
         std::to_string(idx3) + "]";
}

std::string convertSliceCountsToString(const std::map<std::string, unsigned int> &sliceCounts);
std::vector<std::string> splitX26XOptionsString(const std::string str, const std::string seperator);
size_t                   getStartCodeOffset(const ByteVector &data);

} // namespace combiner::parser
