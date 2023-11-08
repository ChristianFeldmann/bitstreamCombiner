/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>
#include <common/SubByteWriter.h>

namespace combiner::parser::hevc
{

// 7.3.4 Scaling list data syntax
class scaling_list_data
{
public:
  scaling_list_data() {}

  void parse(SubByteReader &reader);
  void write(SubByteWriter &writer) const;

  boolArray2D<4, 6>       scaling_list_pred_mode_flag{};
  array2D<uint64_t, 4, 6> scaling_list_pred_matrix_id_delta{};
  array2D<int64_t, 2, 6>  scaling_list_dc_coef_minus8{};
  array2D<int64_t, 2, 6>  scaling_list_delta_coef{};
};

} // namespace combiner::parser::hevc
