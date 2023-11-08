/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "pic_parameter_set_rbsp.h"

namespace combiner::parser::hevc
{

void pic_parameter_set_rbsp::parse(SubByteReader &reader)
{
  this->pps_pic_parameter_set_id              = reader.readUEV();
  this->pps_seq_parameter_set_id              = reader.readUEV();
  this->dependent_slice_segments_enabled_flag = reader.readFlag();
  this->output_flag_present_flag              = reader.readFlag();
  this->num_extra_slice_header_bits           = reader.readBits(3);

  this->sign_data_hiding_enabled_flag        = reader.readFlag();
  this->cabac_init_present_flag              = reader.readFlag();
  this->num_ref_idx_l0_default_active_minus1 = reader.readUEV();
  this->num_ref_idx_l1_default_active_minus1 = reader.readUEV();
  this->init_qp_minus26                      = reader.readSEV();
  this->constrained_intra_pred_flag          = reader.readFlag();
  this->transform_skip_enabled_flag          = reader.readFlag();
  this->cu_qp_delta_enabled_flag             = reader.readFlag();
  if (this->cu_qp_delta_enabled_flag)
    this->diff_cu_qp_delta_depth = reader.readUEV();
  this->pps_cb_qp_offset                         = reader.readSEV();
  this->pps_cr_qp_offset                         = reader.readSEV();
  this->pps_slice_chroma_qp_offsets_present_flag = reader.readFlag();
  this->weighted_pred_flag                       = reader.readFlag();
  this->weighted_bipred_flag                     = reader.readFlag();
  this->transquant_bypass_enabled_flag           = reader.readFlag();
  this->tiles_enabled_flag                       = reader.readFlag();
  this->entropy_coding_sync_enabled_flag         = reader.readFlag();
  if (this->tiles_enabled_flag)
  {
    this->num_tile_columns_minus1 = reader.readUEV();
    this->num_tile_rows_minus1    = reader.readUEV();
    this->uniform_spacing_flag    = reader.readFlag();
    if (!this->uniform_spacing_flag)
    {
      for (unsigned i = 0; i < this->num_tile_columns_minus1; i++)
        this->column_width_minus1.push_back(reader.readUEV());
      for (unsigned i = 0; i < this->num_tile_rows_minus1; i++)
        this->row_height_minus1.push_back(reader.readUEV());
    }
    this->loop_filter_across_tiles_enabled_flag = reader.readFlag();
  }
  this->pps_loop_filter_across_slices_enabled_flag = reader.readFlag();
  this->deblocking_filter_control_present_flag     = reader.readFlag();
  if (this->deblocking_filter_control_present_flag)
  {
    this->deblocking_filter_override_enabled_flag = reader.readFlag();
    this->pps_deblocking_filter_disabled_flag     = reader.readFlag();
    if (!this->pps_deblocking_filter_disabled_flag)
    {
      this->pps_beta_offset_div2 = reader.readSEV();
      this->pps_tc_offset_div2   = reader.readSEV();
    }
  }
  this->pps_scaling_list_data_present_flag = reader.readFlag();
  if (this->pps_scaling_list_data_present_flag)
    this->scalingListData.parse(reader);
  this->lists_modification_present_flag             = reader.readFlag();
  this->log2_parallel_merge_level_minus2            = reader.readUEV();
  this->slice_segment_header_extension_present_flag = reader.readFlag();
  this->pps_extension_present_flag                  = reader.readFlag();
  if (this->pps_extension_present_flag)
  {
    this->pps_range_extension_flag      = reader.readFlag();
    this->pps_multilayer_extension_flag = reader.readFlag();
    this->pps_3d_extension_flag         = reader.readFlag();
    this->pps_extension_5bits           = reader.readBits(5);
  }

  // Now the extensions follow ...

  if (this->pps_range_extension_flag)
    this->ppsRangeExtension.parse(reader, this->transform_skip_enabled_flag);

  // This would also be interesting but later.
  if (this->pps_multilayer_extension_flag)
    throw std::runtime_error("Not implemented yet");

  if (this->pps_3d_extension_flag)
    throw std::runtime_error("Not implemented yet");

  if (this->pps_extension_5bits != 0)
    throw std::runtime_error("Not implemented yet");

  // There is more to parse but we are not interested in this information (for now)
  rbsp_trailing_bits::parse(reader);
}

void pic_parameter_set_rbsp::write(SubByteWriter &writer) const
{
  writer.writeUEV(this->pps_pic_parameter_set_id);
  writer.writeUEV(this->pps_seq_parameter_set_id);
  writer.writeFlag(this->dependent_slice_segments_enabled_flag);
  writer.writeFlag(this->output_flag_present_flag);
  writer.writeBits(this->num_extra_slice_header_bits, 3);

  writer.writeFlag(this->sign_data_hiding_enabled_flag);
  writer.writeFlag(this->cabac_init_present_flag);
  writer.writeUEV(this->num_ref_idx_l0_default_active_minus1);
  writer.writeUEV(this->num_ref_idx_l1_default_active_minus1);
  writer.writeSEV(this->init_qp_minus26);
  writer.writeFlag(this->constrained_intra_pred_flag);
  writer.writeFlag(this->transform_skip_enabled_flag);
  writer.writeFlag(this->cu_qp_delta_enabled_flag);
  if (this->cu_qp_delta_enabled_flag)
    writer.writeUEV(this->diff_cu_qp_delta_depth);
  writer.writeSEV(this->pps_cb_qp_offset);
  writer.writeSEV(this->pps_cr_qp_offset);
  writer.writeFlag(this->pps_slice_chroma_qp_offsets_present_flag);
  writer.writeFlag(this->weighted_pred_flag);
  writer.writeFlag(this->weighted_bipred_flag);
  writer.writeFlag(this->transquant_bypass_enabled_flag);
  writer.writeFlag(this->tiles_enabled_flag);
  writer.writeFlag(this->entropy_coding_sync_enabled_flag);
  if (this->tiles_enabled_flag)
  {
    writer.writeUEV(this->num_tile_columns_minus1);
    writer.writeUEV(this->num_tile_rows_minus1);
    writer.writeFlag(this->uniform_spacing_flag);
    if (!this->uniform_spacing_flag)
    {
      for (unsigned i = 0; i < this->num_tile_columns_minus1; i++)
        writer.writeUEV(this->column_width_minus1.at(i));
      for (unsigned i = 0; i < this->num_tile_rows_minus1; i++)
        writer.writeUEV(this->row_height_minus1.at(i));
    }
    writer.writeFlag(this->loop_filter_across_tiles_enabled_flag);
  }
  writer.writeFlag(this->pps_loop_filter_across_slices_enabled_flag);
  writer.writeFlag(this->deblocking_filter_control_present_flag);
  if (this->deblocking_filter_control_present_flag)
  {
    writer.writeFlag(this->deblocking_filter_override_enabled_flag);
    writer.writeFlag(this->pps_deblocking_filter_disabled_flag);
    if (!this->pps_deblocking_filter_disabled_flag)
    {
      writer.writeSEV(this->pps_beta_offset_div2);
      writer.writeSEV(this->pps_tc_offset_div2);
    }
  }
  writer.writeFlag(this->pps_scaling_list_data_present_flag);
  if (this->pps_scaling_list_data_present_flag)
    this->scalingListData.write(writer);
  writer.writeFlag(this->lists_modification_present_flag);
  writer.writeUEV(this->log2_parallel_merge_level_minus2);
  writer.writeFlag(this->slice_segment_header_extension_present_flag);
  writer.writeFlag(this->pps_extension_present_flag);
  if (this->pps_extension_present_flag)
  {
    writer.writeFlag(this->pps_range_extension_flag);
    writer.writeFlag(this->pps_multilayer_extension_flag);
    writer.writeFlag(this->pps_3d_extension_flag);
    writer.writeBits(this->pps_extension_5bits, 5);
  }

  // Now the extensions follow ...

  if (this->pps_range_extension_flag)
    this->ppsRangeExtension.write(writer, this->transform_skip_enabled_flag);

  // This would also be interesting but later.
  if (this->pps_multilayer_extension_flag)
    throw std::runtime_error("Not implemented yet");

  if (this->pps_3d_extension_flag)
    throw std::runtime_error("Not implemented yet");

  if (this->pps_extension_5bits != 0)
    throw std::runtime_error("Not implemented yet");

  // There is more to parse but we are not interested in this information (for now)
  rbsp_trailing_bits::write(writer);
}

} // namespace combiner::parser::hevc
