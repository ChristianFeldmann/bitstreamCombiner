/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>

#include <memory>

namespace combiner::parser::hevc
{

class seq_parameter_set_rbsp;
class slice_segment_header;

// 7.3.6.3 Weighted prediction parameters syntax
class pred_weight_table
{
public:
  pred_weight_table() {}

  void parse(SubByteReader &                         reader,
             std::shared_ptr<seq_parameter_set_rbsp> sps,
             const slice_segment_header *            slice);

  uint64_t        luma_log2_weight_denom{};
  int64_t         delta_chroma_log2_weight_denom{};
  vector<bool>    luma_weight_l0_flag;
  vector<bool>    chroma_weight_l0_flag;
  vector<int64_t> delta_luma_weight_l0;
  vector<int64_t> luma_offset_l0;
  vector<int64_t> delta_chroma_weight_l0;
  vector<int64_t> delta_chroma_offset_l0;

  vector<bool>    luma_weight_l1_flag;
  vector<bool>    chroma_weight_l1_flag;
  vector<int64_t> delta_luma_weight_l1;
  vector<int64_t> luma_offset_l1;
  vector<int64_t> delta_chroma_weight_l1;
  vector<int64_t> delta_chroma_offset_l1;
};

} // namespace combiner::parser::hevc