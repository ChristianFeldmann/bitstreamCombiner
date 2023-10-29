/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>

namespace combiner::parser::hevc
{

class pps_range_extension
{
public:
  pps_range_extension() {}

  void parse(SubByteReader &reader, const bool transform_skip_enabled_flag);

  uint64_t        log2_max_transform_skip_block_size_minus2{};
  bool            cross_component_prediction_enabled_flag{};
  bool            chroma_qp_offset_list_enabled_flag{};
  uint64_t        diff_cu_chroma_qp_offset_depth{};
  uint64_t        chroma_qp_offset_list_len_minus1{};
  vector<int64_t> cb_qp_offset_list;
  vector<int64_t> cr_qp_offset_list;
  uint64_t        log2_sao_offset_scale_luma{};
  uint64_t        log2_sao_offset_scale_chroma{};
};

} // namespace combiner::parser::hevc