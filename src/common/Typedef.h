/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>

namespace combiner
{

using ByteVector = std::vector<uint8_t>;
using pairUint64 = std::pair<uint64_t, uint64_t>;

template <typename T> using vector  = std::vector<T>;
template <typename T> using umap_1d = std::map<unsigned, T>;

template <std::size_t N> using boolArray                          = std::array<bool, N>;
template <typename T, std::size_t N, std::size_t M> using array2D = std::array<std::array<T, M>, N>;
template <std::size_t N, std::size_t M> using boolArray2D         = array2D<bool, N, M>;

struct FrameSize
{
  uint64_t width{};
  uint64_t height{};

  bool operator!=(const FrameSize &other)
  {
    return this->width != other.width || this->height != other.height;
  }

  std::string toString() const
  {
    return std::to_string(this->width) + "x" + std::to_string(this->height);
  }
};

} // namespace combiner
