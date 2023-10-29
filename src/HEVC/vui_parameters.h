/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>

#include "hrd_parameters.h"

namespace combiner::parser::hevc
{

class vui_parameters
{
public:
  vui_parameters() {}

  void parse(SubByteReader &reader, const uint64_t sps_max_sub_layers_minus1);

  bool     aspect_ratio_info_present_flag{};
  uint64_t aspect_ratio_idc{};
  uint64_t sar_width{};
  uint64_t sar_height;
  bool     overscan_info_present_flag{};
  bool     overscan_appropriate_flag{};
  bool     video_signal_type_present_flag{};
  uint64_t video_format{5};
  bool     video_full_range_flag{};
  bool     colour_description_present_flag{};
  uint64_t colour_primaries{};
  uint64_t transfer_characteristics{};
  uint64_t matrix_coeffs{};
  bool     chroma_loc_info_present_flag{};
  uint64_t chroma_sample_loc_type_top_field{};
  uint64_t chroma_sample_loc_type_bottom_field{};
  bool     neutral_chroma_indication_flag{};
  bool     field_seq_flag{};
  bool     frame_field_info_present_flag{};
  bool     default_display_window_flag{};
  uint64_t def_disp_win_left_offset{};
  uint64_t def_disp_win_right_offset{};
  uint64_t def_disp_win_top_offset{};
  uint64_t def_disp_win_bottom_offset{};

  bool           vui_timing_info_present_flag{};
  uint64_t       vui_num_units_in_tick{};
  uint64_t       vui_time_scale{};
  bool           vui_poc_proportional_to_timing_flag{};
  uint64_t       vui_num_ticks_poc_diff_one_minus1{};
  bool           vui_hrd_parameters_present_flag{};
  hrd_parameters hrdParameters;

  bool     bitstream_restriction_flag{};
  bool     tiles_fixed_structure_flag{};
  bool     motion_vectors_over_pic_boundaries_flag{};
  bool     restricted_ref_pic_lists_flag{};
  uint64_t min_spatial_segmentation_idc{};
  uint64_t max_bytes_per_pic_denom{};
  uint64_t max_bits_per_min_cu_denom{};
  uint64_t log2_max_mv_length_horizontal{};
  uint64_t log2_max_mv_length_vertical{};

  // Calculated values
  double frameRate{};
};

} // namespace combiner::parser::hevc