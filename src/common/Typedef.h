/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include <map>
#include <vector>

namespace combiner
{

using ByteVector = std::vector<char>;
using pairUint64 = std::pair<uint64_t, uint64_t>;

template <typename T> using vector  = std::vector<T>;
template <typename T> using umap_1d = std::map<unsigned, T>;

} // namespace combiner
