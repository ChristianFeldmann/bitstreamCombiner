/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "pps_range_extension.h"

namespace combiner::parser::hevc
{

void pps_range_extension::parse(SubByteReader &reader, const bool transform_skip_enabled_flag)
{
  if (transform_skip_enabled_flag)
    this->log2_max_transform_skip_block_size_minus2 = reader.readUEV();
  this->cross_component_prediction_enabled_flag = reader.readFlag();
  this->chroma_qp_offset_list_enabled_flag      = reader.readFlag();
  if (this->chroma_qp_offset_list_enabled_flag)
  {
    this->diff_cu_chroma_qp_offset_depth   = reader.readUEV();
    this->chroma_qp_offset_list_len_minus1 = reader.readUEV();
    for (unsigned i = 0; i <= this->chroma_qp_offset_list_len_minus1; i++)
    {
      this->cb_qp_offset_list.push_back(reader.readSEV());
      this->cr_qp_offset_list.push_back(reader.readSEV());
    }
  }
  this->log2_sao_offset_scale_luma   = reader.readUEV();
  this->log2_sao_offset_scale_chroma = reader.readUEV();
}

void pps_range_extension::write(SubByteWriter &writer, const bool transform_skip_enabled_flag) const
{
  if (transform_skip_enabled_flag)
    writer.writeUEV(this->log2_max_transform_skip_block_size_minus2);
  writer.writeFlag(this->cross_component_prediction_enabled_flag);
  writer.writeFlag(this->chroma_qp_offset_list_enabled_flag);
  if (this->chroma_qp_offset_list_enabled_flag)
  {
    writer.writeUEV(this->diff_cu_chroma_qp_offset_depth);
    writer.writeUEV(this->chroma_qp_offset_list_len_minus1);
    for (unsigned i = 0; i <= this->chroma_qp_offset_list_len_minus1; i++)
    {
      writer.writeSEV(this->cb_qp_offset_list.at(i));
      writer.writeSEV(this->cr_qp_offset_list.at(i));
    }
  }
  writer.writeUEV(this->log2_sao_offset_scale_luma);
  writer.writeUEV(this->log2_sao_offset_scale_chroma);
}

} // namespace combiner::parser::hevc
