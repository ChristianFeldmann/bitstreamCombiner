/*  This file is part of YUView - The YUV player with advanced analytics toolset
 *   <https://github.com/IENT/YUView>
 *   Copyright (C) 2015  Institut f�r Nachrichtentechnik, RWTH Aachen University, GERMANY
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the
 *   OpenSSL library under certain conditions as described in each
 *   individual source file, and distribute linked combinations including
 *   the two.
 *
 *   You must obey the GNU General Public License in all respects for all
 *   of the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the
 *   file(s), but you are not obligated to do so. If you do not wish to do
 *   so, delete this exception statement from your version. If you delete
 *   this exception statement from all source files in the program, then
 *   also delete it here.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
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
  this->rbspTrailingBits.parse(reader);
}

} // namespace combiner::parser::hevc
