/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "seq_parameter_set_rbsp.h"

#include <cmath>

namespace combiner::parser::hevc
{

void seq_parameter_set_rbsp::parse(SubByteReader &reader)
{
  this->sps_video_parameter_set_id   = reader.readBits(4);
  this->sps_max_sub_layers_minus1    = reader.readBits(3);
  this->sps_temporal_id_nesting_flag = reader.readFlag();

  // parse profile tier level
  this->profileTierLevel.parse(reader, true, sps_max_sub_layers_minus1);

  /// Back to the seq_parameter_set_rbsp
  this->sps_seq_parameter_set_id = reader.readUEV();
  this->chroma_format_idc        = reader.readUEV();
  if (this->chroma_format_idc == 3)
    this->separate_colour_plane_flag = reader.readFlag();
  this->ChromaArrayType = (this->separate_colour_plane_flag) ? 0 : this->chroma_format_idc;

  // Rec. ITU-T H.265 v3 (04/2015) - 6.2 - Table 6-1
  this->SubWidthC  = (this->chroma_format_idc == 1 || this->chroma_format_idc == 2) ? 2 : 1;
  this->SubHeightC = (this->chroma_format_idc == 1) ? 2 : 1;

  this->pic_width_in_luma_samples  = reader.readUEV();
  this->pic_height_in_luma_samples = reader.readUEV();
  this->conformance_window_flag    = reader.readFlag();

  if (this->conformance_window_flag)
  {
    this->conf_win_left_offset   = reader.readUEV();
    this->conf_win_right_offset  = reader.readUEV();
    this->conf_win_top_offset    = reader.readUEV();
    this->conf_win_bottom_offset = reader.readUEV();
  }

  this->bit_depth_luma_minus8             = reader.readUEV();
  this->bit_depth_chroma_minus8           = reader.readUEV();
  this->log2_max_pic_order_cnt_lsb_minus4 = reader.readUEV();

  this->sps_sub_layer_ordering_info_present_flag = reader.readFlag();
  for (uint64_t i =
           (this->sps_sub_layer_ordering_info_present_flag ? 0 : this->sps_max_sub_layers_minus1);
       i <= this->sps_max_sub_layers_minus1;
       i++)
  {
    this->sps_max_dec_pic_buffering_minus1.push_back(reader.readUEV());
    this->sps_max_num_reorder_pics.push_back(reader.readUEV());
    this->sps_max_latency_increase_plus1.push_back(reader.readUEV());
  }

  this->log2_min_luma_coding_block_size_minus3      = reader.readUEV();
  this->log2_diff_max_min_luma_coding_block_size    = reader.readUEV();
  this->log2_min_luma_transform_block_size_minus2   = reader.readUEV();
  this->log2_diff_max_min_luma_transform_block_size = reader.readUEV();
  this->max_transform_hierarchy_depth_inter         = reader.readUEV();
  this->max_transform_hierarchy_depth_intra         = reader.readUEV();
  this->scaling_list_enabled_flag                   = reader.readFlag();

  if (this->scaling_list_enabled_flag)
  {
    this->sps_scaling_list_data_present_flag = reader.readFlag();
    if (this->sps_scaling_list_data_present_flag)
      this->scalingListData.parse(reader);
  }

  this->amp_enabled_flag                    = reader.readFlag();
  this->sample_adaptive_offset_enabled_flag = reader.readFlag();
  this->pcm_enabled_flag                    = reader.readFlag();

  if (this->pcm_enabled_flag)
  {
    this->pcm_sample_bit_depth_luma_minus1             = reader.readBits(4);
    this->pcm_sample_bit_depth_chroma_minus1           = reader.readBits(4);
    this->log2_min_pcm_luma_coding_block_size_minus3   = reader.readUEV();
    this->log2_diff_max_min_pcm_luma_coding_block_size = reader.readUEV();
    this->pcm_loop_filter_disabled_flag                = reader.readFlag();
  }

  this->num_short_term_ref_pic_sets = reader.readUEV();
  for (uint64_t i = 0; i < this->num_short_term_ref_pic_sets; i++)
  {
    st_ref_pic_set rps;
    rps.parse(reader, i, this->num_short_term_ref_pic_sets);
    this->stRefPicSets.push_back(rps);
  }

  this->long_term_ref_pics_present_flag = reader.readFlag();
  if (this->long_term_ref_pics_present_flag)
  {
    this->num_long_term_ref_pics_sps = reader.readUEV();
    for (unsigned int i = 0; i < this->num_long_term_ref_pics_sps; i++)
    {
      const auto nrBits = this->log2_max_pic_order_cnt_lsb_minus4 + 4;
      this->lt_ref_pic_poc_lsb_sps.push_back(reader.readBits(nrBits));
      this->used_by_curr_pic_lt_sps_flag.push_back(reader.readFlag());
    }
  }

  this->sps_temporal_mvp_enabled_flag       = reader.readFlag();
  this->strong_intra_smoothing_enabled_flag = reader.readFlag();
  this->vui_parameters_present_flag         = reader.readFlag();
  if (this->vui_parameters_present_flag)
    this->vuiParameters.parse(reader, this->sps_max_sub_layers_minus1);

  this->sps_extension_present_flag = reader.readFlag();
  if (this->sps_extension_present_flag)
  {
    this->sps_range_extension_flag      = reader.readFlag();
    this->sps_multilayer_extension_flag = reader.readFlag();
    this->sps_3d_extension_flag         = reader.readFlag();
    this->sps_extension_5bits           = reader.readBits(5);
  }

  // Now the extensions follow ...
  // This would also be interesting but later.
  if (this->sps_range_extension_flag)
    throw std::runtime_error("Not implemented yet");

  if (this->sps_multilayer_extension_flag)
    throw std::runtime_error("Not implemented yet");

  if (this->sps_3d_extension_flag)
    throw std::runtime_error("Not implemented yet");

  if (this->sps_extension_5bits != 0)
    throw std::runtime_error("Not implemented yet");

  // rbspTrailingBits.parse(reader);

  // Calculate some values - Rec. ITU-T H.265 v3 (04/2015) 7.4.3.2.1
  this->MinCbLog2SizeY = this->log2_min_luma_coding_block_size_minus3 + 3; // (7-10)
  this->CtbLog2SizeY =
      this->MinCbLog2SizeY + this->log2_diff_max_min_luma_coding_block_size; // (7-11)
  this->CtbSizeY = uint64_t(1) << this->CtbLog2SizeY;                        // (7-13)
  this->PicWidthInCtbsY =
      (this->pic_width_in_luma_samples + this->CtbSizeY - 1) / this->CtbSizeY; // (7-15)
  this->PicHeightInCtbsY =
      (this->pic_height_in_luma_samples + this->CtbSizeY - 1) / this->CtbSizeY; // (7-17)
  this->PicSizeInCtbsY = this->PicWidthInCtbsY * this->PicHeightInCtbsY;        // (7-19)
}

} // namespace combiner::parser::hevc