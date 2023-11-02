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
