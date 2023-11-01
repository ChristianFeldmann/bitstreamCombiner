/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "vui_parameters.h"

namespace combiner::parser::hevc
{

void vui_parameters::parse(SubByteReader &reader, const uint64_t sps_max_sub_layers_minus1)
{
  this->aspect_ratio_info_present_flag = reader.readFlag();
  if (this->aspect_ratio_info_present_flag)
  {
    this->aspect_ratio_idc = reader.readBits(8);
    if (this->aspect_ratio_idc == 255) // EXTENDED_SAR=255
    {
      this->sar_width  = reader.readBits(16);
      this->sar_height = reader.readBits(16);
    }
  }

  this->overscan_info_present_flag = reader.readFlag();
  if (this->overscan_info_present_flag)
    this->overscan_appropriate_flag = reader.readFlag();

  this->video_signal_type_present_flag = reader.readFlag();
  if (video_signal_type_present_flag)
  {
    this->video_format          = reader.readBits(3);
    this->video_full_range_flag = reader.readFlag();

    this->colour_description_present_flag = reader.readFlag();
    if (this->colour_description_present_flag)
    {
      this->colour_primaries         = reader.readBits(8);
      this->transfer_characteristics = reader.readBits(8);
      this->matrix_coeffs            = reader.readBits(8);
    }
  }

  this->chroma_loc_info_present_flag = reader.readFlag();
  if (this->chroma_loc_info_present_flag)
  {
    this->chroma_sample_loc_type_top_field    = reader.readUEV();
    this->chroma_sample_loc_type_bottom_field = reader.readUEV();
  }

  this->neutral_chroma_indication_flag = reader.readFlag();
  this->field_seq_flag                 = reader.readFlag();
  this->frame_field_info_present_flag  = reader.readFlag();
  this->default_display_window_flag    = reader.readFlag();
  if (this->default_display_window_flag)
  {
    this->def_disp_win_left_offset   = reader.readUEV();
    this->def_disp_win_right_offset  = reader.readUEV();
    this->def_disp_win_top_offset    = reader.readUEV();
    this->def_disp_win_bottom_offset = reader.readUEV();
  }

  this->vui_timing_info_present_flag = reader.readFlag();
  if (this->vui_timing_info_present_flag)
  {
    // The VUI has timing information for us
    this->vui_num_units_in_tick = reader.readBits(32);
    this->vui_time_scale        = reader.readBits(32);

    this->frameRate = double(this->vui_time_scale) / double(this->vui_num_units_in_tick);

    this->vui_poc_proportional_to_timing_flag = reader.readFlag();
    if (this->vui_poc_proportional_to_timing_flag)
      this->vui_num_ticks_poc_diff_one_minus1 = reader.readUEV();
    this->vui_hrd_parameters_present_flag = reader.readFlag();
    if (this->vui_hrd_parameters_present_flag)
      this->hrdParameters.parse(reader, 1, sps_max_sub_layers_minus1);
  }

  this->bitstream_restriction_flag = reader.readFlag();
  if (this->bitstream_restriction_flag)
  {
    this->tiles_fixed_structure_flag              = reader.readFlag();
    this->motion_vectors_over_pic_boundaries_flag = reader.readFlag();
    this->restricted_ref_pic_lists_flag           = reader.readFlag();
    this->min_spatial_segmentation_idc            = reader.readUEV();
    this->max_bytes_per_pic_denom                 = reader.readUEV();
    this->max_bits_per_min_cu_denom               = reader.readUEV();
    this->log2_max_mv_length_horizontal           = reader.readUEV();
    this->log2_max_mv_length_vertical             = reader.readUEV();
  }
}

void vui_parameters::write(SubByteWriter &writer, const uint64_t sps_max_sub_layers_minus1) const
{
  writer.writeFlag(this->aspect_ratio_info_present_flag);
  if (this->aspect_ratio_info_present_flag)
  {
    writer.writeBits(this->aspect_ratio_idc, 8);
    if (this->aspect_ratio_idc == 255) // EXTENDED_SAR=255
    {
      writer.writeBits(this->sar_width, 16);
      writer.writeBits(this->sar_height, 16);
    }
  }

  writer.writeFlag(this->overscan_info_present_flag);
  if (this->overscan_info_present_flag)
    writer.writeFlag(this->overscan_appropriate_flag);

  writer.writeFlag(this->video_signal_type_present_flag);
  if (video_signal_type_present_flag)
  {
    writer.writeBits(this->video_format, 3);
    writer.writeFlag(this->video_full_range_flag);

    writer.writeFlag(this->colour_description_present_flag);
    if (this->colour_description_present_flag)
    {
      writer.writeBits(this->colour_primaries, 8);
      writer.writeBits(this->transfer_characteristics, 8);
      writer.writeBits(this->matrix_coeffs, 8);
    }
  }

  writer.writeFlag(this->chroma_loc_info_present_flag);
  if (this->chroma_loc_info_present_flag)
  {
    writer.writeUEV(this->chroma_sample_loc_type_top_field);
    writer.writeUEV(this->chroma_sample_loc_type_bottom_field);
  }

  writer.writeFlag(this->neutral_chroma_indication_flag);
  writer.writeFlag(this->field_seq_flag);
  writer.writeFlag(this->frame_field_info_present_flag);
  writer.writeFlag(this->default_display_window_flag);
  if (this->default_display_window_flag)
  {
    writer.writeUEV(this->def_disp_win_left_offset);
    writer.writeUEV(this->def_disp_win_right_offset);
    writer.writeUEV(this->def_disp_win_top_offset);
    writer.writeUEV(this->def_disp_win_bottom_offset);
  }

  writer.writeFlag(this->vui_timing_info_present_flag);
  if (this->vui_timing_info_present_flag)
  {
    // The VUI has timing information for us
    writer.writeBits(this->vui_num_units_in_tick, 32);
    writer.writeBits(this->vui_time_scale, 32);

    writer.writeFlag(this->vui_poc_proportional_to_timing_flag);
    if (this->vui_poc_proportional_to_timing_flag)
      writer.writeUEV(this->vui_num_ticks_poc_diff_one_minus1);
    writer.writeFlag(this->vui_hrd_parameters_present_flag);
    if (this->vui_hrd_parameters_present_flag)
      this->hrdParameters.write(writer, 1, sps_max_sub_layers_minus1);
  }

  writer.writeFlag(this->bitstream_restriction_flag);
  if (this->bitstream_restriction_flag)
  {
    writer.writeFlag(this->tiles_fixed_structure_flag);
    writer.writeFlag(this->motion_vectors_over_pic_boundaries_flag);
    writer.writeFlag(this->restricted_ref_pic_lists_flag);
    writer.writeUEV(this->min_spatial_segmentation_idc);
    writer.writeUEV(this->max_bytes_per_pic_denom);
    writer.writeUEV(this->max_bits_per_min_cu_denom);
    writer.writeUEV(this->log2_max_mv_length_horizontal);
    writer.writeUEV(this->log2_max_mv_length_vertical);
  }
}

} // namespace combiner::parser::hevc
