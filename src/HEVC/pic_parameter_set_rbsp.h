/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>

#include "NalUnitHEVC.h"
#include "pps_range_extension.h"
#include "rbsp_trailing_bits.h"
#include "scaling_list_data.h"

namespace combiner::parser::hevc
{

class pic_parameter_set_rbsp : public NalRBSP
{
public:
  pic_parameter_set_rbsp() {}

  void parse(SubByteReader &reader);

  uint64_t          pps_pic_parameter_set_id{};
  uint64_t          pps_seq_parameter_set_id{};
  bool              dependent_slice_segments_enabled_flag{};
  bool              output_flag_present_flag{};
  uint64_t          num_extra_slice_header_bits{};
  bool              sign_data_hiding_enabled_flag{};
  bool              cabac_init_present_flag{};
  uint64_t          num_ref_idx_l0_default_active_minus1{};
  uint64_t          num_ref_idx_l1_default_active_minus1{};
  int64_t           init_qp_minus26{};
  bool              constrained_intra_pred_flag{};
  bool              transform_skip_enabled_flag{};
  bool              cu_qp_delta_enabled_flag{};
  uint64_t          diff_cu_qp_delta_depth{};
  int64_t           pps_cb_qp_offset{};
  int64_t           pps_cr_qp_offset{};
  bool              pps_slice_chroma_qp_offsets_present_flag{};
  bool              weighted_pred_flag{};
  bool              weighted_bipred_flag{};
  bool              transquant_bypass_enabled_flag{};
  bool              tiles_enabled_flag{};
  bool              entropy_coding_sync_enabled_flag{};
  uint64_t          num_tile_columns_minus1{};
  uint64_t          num_tile_rows_minus1{};
  bool              uniform_spacing_flag{};
  vector<uint64_t>  column_width_minus1;
  vector<uint64_t>  row_height_minus1;
  bool              loop_filter_across_tiles_enabled_flag{};
  bool              pps_loop_filter_across_slices_enabled_flag{};
  bool              deblocking_filter_control_present_flag{};
  bool              deblocking_filter_override_enabled_flag{};
  bool              pps_deblocking_filter_disabled_flag{};
  int64_t           pps_beta_offset_div2{};
  int64_t           pps_tc_offset_div2{};
  bool              pps_scaling_list_data_present_flag{};
  scaling_list_data scalingListData;
  bool              lists_modification_present_flag{};
  uint64_t          log2_parallel_merge_level_minus2{};
  bool              slice_segment_header_extension_present_flag{};
  bool              pps_extension_present_flag{};
  bool              pps_range_extension_flag{};
  bool              pps_multilayer_extension_flag{};
  bool              pps_3d_extension_flag{};
  uint64_t          pps_extension_5bits{};

  pps_range_extension ppsRangeExtension;

  rbsp_trailing_bits rbspTrailingBits;
};

} // namespace combiner::parser::hevc