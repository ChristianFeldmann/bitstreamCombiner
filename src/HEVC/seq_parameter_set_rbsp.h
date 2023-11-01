/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>
#include <common/SubByteWriter.h>

#include "NalUnitHEVC.h"
#include "profile_tier_level.h"
#include "rbsp_trailing_bits.h"
#include "scaling_list_data.h"
#include "st_ref_pic_set.h"
#include "vui_parameters.h"

namespace combiner::parser::hevc
{

class seq_parameter_set_rbsp : public NalRBSP
{
public:
  seq_parameter_set_rbsp() {}

  void parse(SubByteReader &reader);
  void write(SubByteWriter &writer) const;

  uint64_t           sps_video_parameter_set_id{};
  uint64_t           sps_max_sub_layers_minus1{};
  bool               sps_temporal_id_nesting_flag{};
  profile_tier_level profileTierLevel;

  uint64_t sps_seq_parameter_set_id{};
  uint64_t chroma_format_idc{};
  bool     separate_colour_plane_flag{false};
  uint64_t pic_width_in_luma_samples{};
  uint64_t pic_height_in_luma_samples{};
  bool     conformance_window_flag{};

  uint64_t conf_win_left_offset{};
  uint64_t conf_win_right_offset{};
  uint64_t conf_win_top_offset{};
  uint64_t conf_win_bottom_offset{};

  uint64_t         bit_depth_luma_minus8{};
  uint64_t         bit_depth_chroma_minus8{};
  uint64_t         log2_max_pic_order_cnt_lsb_minus4{};
  bool             sps_sub_layer_ordering_info_present_flag{};
  vector<uint64_t> sps_max_dec_pic_buffering_minus1;
  vector<uint64_t> sps_max_num_reorder_pics;
  vector<uint64_t> sps_max_latency_increase_plus1;

  uint64_t          log2_min_luma_coding_block_size_minus3{};
  uint64_t          log2_diff_max_min_luma_coding_block_size{};
  uint64_t          log2_min_luma_transform_block_size_minus2{};
  uint64_t          log2_diff_max_min_luma_transform_block_size{};
  uint64_t          max_transform_hierarchy_depth_inter{};
  uint64_t          max_transform_hierarchy_depth_intra{};
  bool              scaling_list_enabled_flag{};
  bool              sps_scaling_list_data_present_flag{};
  scaling_list_data scalingListData;

  bool                   amp_enabled_flag{};
  bool                   sample_adaptive_offset_enabled_flag{};
  bool                   pcm_enabled_flag{};
  uint64_t               pcm_sample_bit_depth_luma_minus1{};
  uint64_t               pcm_sample_bit_depth_chroma_minus1{};
  uint64_t               log2_min_pcm_luma_coding_block_size_minus3{};
  uint64_t               log2_diff_max_min_pcm_luma_coding_block_size{};
  bool                   pcm_loop_filter_disabled_flag{};
  uint64_t               num_short_term_ref_pic_sets{};
  vector<st_ref_pic_set> stRefPicSets;
  bool                   long_term_ref_pics_present_flag{};
  uint64_t               num_long_term_ref_pics_sps{};
  vector<uint64_t>       lt_ref_pic_poc_lsb_sps;
  vector<bool>           used_by_curr_pic_lt_sps_flag;

  bool sps_temporal_mvp_enabled_flag{};
  bool strong_intra_smoothing_enabled_flag{};

  bool           vui_parameters_present_flag{};
  vui_parameters vuiParameters;

  bool     sps_extension_present_flag{};
  bool     sps_range_extension_flag{};
  bool     sps_multilayer_extension_flag{};
  bool     sps_3d_extension_flag{};
  uint64_t sps_extension_5bits{};

  // 7.4.3.2.1
  uint64_t ChromaArrayType{};
  uint64_t SubWidthC, SubHeightC{};
  uint64_t MinCbLog2SizeY;
  uint64_t CtbLog2SizeY;
  uint64_t CtbSizeY{};
  uint64_t PicWidthInCtbsY{};
  uint64_t PicHeightInCtbsY{};
  uint64_t PicSizeInCtbsY{};

  // Get the actual size of the image that will be returned. Internally the image might be bigger.
  uint64_t get_conformance_cropping_width() const
  {
    return (this->pic_width_in_luma_samples - (this->SubWidthC * this->conf_win_right_offset) -
            this->SubWidthC * this->conf_win_left_offset);
  }
  uint64_t get_conformance_cropping_height() const
  {
    return (this->pic_height_in_luma_samples - (this->SubHeightC * this->conf_win_bottom_offset) -
            this->SubHeightC * this->conf_win_top_offset);
  }
};

} // namespace combiner::parser::hevc