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

  this->profileTierLevel.parse(reader, true, this->sps_max_sub_layers_minus1);

  this->sps_seq_parameter_set_id = reader.readUEV();
  this->chroma_format_idc        = reader.readUEV();
  if (this->chroma_format_idc == 3)
    this->separate_colour_plane_flag = reader.readFlag();

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

  rbsp_trailing_bits::parse(reader);

  this->updateCalculatedValues();
}

void seq_parameter_set_rbsp::write(SubByteWriter &writer) const
{
  writer.writeBits(this->sps_video_parameter_set_id, 4);
  writer.writeBits(this->sps_max_sub_layers_minus1, 3);
  writer.writeFlag(this->sps_temporal_id_nesting_flag);

  this->profileTierLevel.write(writer, true, this->sps_max_sub_layers_minus1);

  writer.writeUEV(this->sps_seq_parameter_set_id);
  writer.writeUEV(this->chroma_format_idc);
  if (this->chroma_format_idc == 3)
    writer.writeFlag(this->separate_colour_plane_flag);

  writer.writeUEV(this->pic_width_in_luma_samples);
  writer.writeUEV(this->pic_height_in_luma_samples);
  writer.writeFlag(this->conformance_window_flag);

  if (this->conformance_window_flag)
  {
    writer.writeUEV(this->conf_win_left_offset);
    writer.writeUEV(this->conf_win_right_offset);
    writer.writeUEV(this->conf_win_top_offset);
    writer.writeUEV(this->conf_win_bottom_offset);
  }

  writer.writeUEV(this->bit_depth_luma_minus8);
  writer.writeUEV(this->bit_depth_chroma_minus8);
  writer.writeUEV(this->log2_max_pic_order_cnt_lsb_minus4);

  writer.writeFlag(this->sps_sub_layer_ordering_info_present_flag);
  for (uint64_t i =
           (this->sps_sub_layer_ordering_info_present_flag ? 0 : this->sps_max_sub_layers_minus1);
       i <= this->sps_max_sub_layers_minus1;
       i++)
  {
    writer.writeUEV(this->sps_max_dec_pic_buffering_minus1.at(i));
    writer.writeUEV(this->sps_max_num_reorder_pics.at(i));
    writer.writeUEV(this->sps_max_latency_increase_plus1.at(i));
  }

  writer.writeUEV(this->log2_min_luma_coding_block_size_minus3);
  writer.writeUEV(this->log2_diff_max_min_luma_coding_block_size);
  writer.writeUEV(this->log2_min_luma_transform_block_size_minus2);
  writer.writeUEV(this->log2_diff_max_min_luma_transform_block_size);
  writer.writeUEV(this->max_transform_hierarchy_depth_inter);
  writer.writeUEV(this->max_transform_hierarchy_depth_intra);
  writer.writeFlag(this->scaling_list_enabled_flag);

  if (this->scaling_list_enabled_flag)
  {
    writer.writeFlag(this->sps_scaling_list_data_present_flag);
    if (this->sps_scaling_list_data_present_flag)
      this->scalingListData.write(writer);
  }

  writer.writeFlag(this->amp_enabled_flag);
  writer.writeFlag(this->sample_adaptive_offset_enabled_flag);
  writer.writeFlag(this->pcm_enabled_flag);

  if (this->pcm_enabled_flag)
  {
    writer.writeBits(this->pcm_sample_bit_depth_luma_minus1, 4);
    writer.writeBits(this->pcm_sample_bit_depth_chroma_minus1, 4);
    writer.writeUEV(this->log2_min_pcm_luma_coding_block_size_minus3);
    writer.writeUEV(this->log2_diff_max_min_pcm_luma_coding_block_size);
    writer.writeFlag(this->pcm_loop_filter_disabled_flag);
  }

  writer.writeUEV(this->num_short_term_ref_pic_sets);
  for (uint64_t i = 0; i < this->num_short_term_ref_pic_sets; i++)
    this->stRefPicSets.at(i).write(writer, i, this->num_short_term_ref_pic_sets);

  writer.writeFlag(this->long_term_ref_pics_present_flag);
  if (this->long_term_ref_pics_present_flag)
  {
    writer.writeUEV(this->num_long_term_ref_pics_sps);
    for (unsigned int i = 0; i < this->num_long_term_ref_pics_sps; i++)
    {
      const auto nrBits = this->log2_max_pic_order_cnt_lsb_minus4 + 4;
      writer.writeBits(this->lt_ref_pic_poc_lsb_sps.at(i), nrBits);
      writer.writeFlag(this->used_by_curr_pic_lt_sps_flag.at(i));
    }
  }

  writer.writeFlag(this->sps_temporal_mvp_enabled_flag);
  writer.writeFlag(this->strong_intra_smoothing_enabled_flag);
  writer.writeFlag(this->vui_parameters_present_flag);
  if (this->vui_parameters_present_flag)
    this->vuiParameters.write(writer, this->sps_max_sub_layers_minus1);

  writer.writeFlag(this->sps_extension_present_flag);
  if (this->sps_extension_present_flag)
  {
    writer.writeFlag(this->sps_range_extension_flag);
    writer.writeFlag(this->sps_multilayer_extension_flag);
    writer.writeFlag(this->sps_3d_extension_flag);
    writer.writeBits(this->sps_extension_5bits, 5);
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

  rbsp_trailing_bits::write(writer);
}

FrameSize seq_parameter_set_rbsp::getFrameSize() const
{
  return {this->pic_width_in_luma_samples, this->pic_height_in_luma_samples};
}

void seq_parameter_set_rbsp::updateCalculatedValues()
{
  this->ChromaArrayType = (this->separate_colour_plane_flag) ? 0 : this->chroma_format_idc;

  // Rec. ITU-T H.265 v3 (04/2015) - 6.2 - Table 6-1
  this->SubWidthC  = (this->chroma_format_idc == 1 || this->chroma_format_idc == 2) ? 2 : 1;
  this->SubHeightC = (this->chroma_format_idc == 1) ? 2 : 1;

  // Rec. ITU-T H.265 v3 (04/2015) 7.4.3.2.1 Equation 7-10 to 7-22
  this->MinCbLog2SizeY    = this->log2_min_luma_coding_block_size_minus3 + 3;
  this->CtbLog2SizeY      = this->MinCbLog2SizeY + this->log2_diff_max_min_luma_coding_block_size;
  this->MinCbSizeY        = 1 << MinCbLog2SizeY;
  this->CtbSizeY          = 1 << this->CtbLog2SizeY;
  this->PicWidthInMinCbsY = this->pic_width_in_luma_samples / this->MinCbSizeY;
  this->PicWidthInCtbsY   = (this->pic_width_in_luma_samples + this->CtbSizeY - 1) / this->CtbSizeY;
  this->PicHeightInMinCbsY = this->pic_height_in_luma_samples / this->MinCbSizeY;
  this->PicHeightInCtbsY = (this->pic_height_in_luma_samples + this->CtbSizeY - 1) / this->CtbSizeY;
  this->PicSizeInMinCbsY = this->PicWidthInMinCbsY * this->PicHeightInMinCbsY;
  this->PicSizeInCtbsY   = this->PicWidthInCtbsY * this->PicHeightInCtbsY;
  this->PicSizeInSamplesY   = this->pic_width_in_luma_samples * this->pic_height_in_luma_samples;
  this->PicWidthInSamplesC  = this->pic_width_in_luma_samples / this->SubWidthC;
  this->PicHeightInSamplesC = this->pic_height_in_luma_samples / this->SubHeightC;
}

} // namespace combiner::parser::hevc
