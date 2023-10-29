/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>
#include <common/Typedef.h>

namespace combiner::parser::hevc
{

// E.2.3 Sub-layer HRD parameters syntax
class sub_layer_hrd_parameters
{
public:
  sub_layer_hrd_parameters() {}

  void parse(SubByteReader &reader,
             const uint64_t CpbCnt,
             const bool     sub_pic_hrd_params_present_flag,
             const bool     SubPicHrdFlag,
             const uint64_t bit_rate_scale,
             const uint64_t cpb_size_scale,
             const uint64_t cpb_size_du_scale);

  vector<uint64_t> bit_rate_value_minus1;
  vector<uint64_t> cpb_size_value_minus1;
  vector<uint64_t> cpb_size_du_value_minus1;
  vector<uint64_t> bit_rate_du_value_minus1;
  vector<bool>     cbr_flag;

  vector<uint64_t> BitRate;
  vector<uint64_t> CpbSize;
};

} // namespace combiner::parser::hevc