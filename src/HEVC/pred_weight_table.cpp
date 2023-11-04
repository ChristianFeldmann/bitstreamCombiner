/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "pred_weight_table.h"

#include "seq_parameter_set_rbsp.h"
#include "slice_segment_header.h"

namespace combiner::parser::hevc
{

void pred_weight_table::parse(SubByteReader &               reader,
                              const seq_parameter_set_rbsp &sps,
                              const slice_segment_header *  slice)
{
  this->luma_log2_weight_denom = reader.readUEV();
  if (sps.ChromaArrayType != 0)
    this->delta_chroma_log2_weight_denom = reader.readSEV();
  for (unsigned i = 0; i <= slice->num_ref_idx_l0_active_minus1; i++)
    this->luma_weight_l0_flag.push_back(reader.readFlag());
  if (sps.ChromaArrayType != 0)
    for (unsigned i = 0; i <= slice->num_ref_idx_l0_active_minus1; i++)
      this->chroma_weight_l0_flag.push_back(reader.readFlag());
  for (unsigned i = 0; i <= slice->num_ref_idx_l0_active_minus1; i++)
  {
    if (this->luma_weight_l0_flag[i])
    {
      this->delta_luma_weight_l0.push_back(reader.readSEV());
      this->luma_offset_l0.push_back(reader.readSEV());
    }
    if (this->chroma_weight_l0_flag[i])
      for (unsigned j = 0; j < 2; j++)
      {
        this->delta_chroma_weight_l0.push_back(reader.readSEV());
        this->delta_chroma_offset_l0.push_back(reader.readSEV());
      }
  }

  if (slice->slice_type == SliceType::B)
  {
    for (unsigned i = 0; i <= slice->num_ref_idx_l1_active_minus1; i++)
      this->luma_weight_l1_flag.push_back(reader.readFlag());
    if (sps.ChromaArrayType != 0)
      for (unsigned i = 0; i <= slice->num_ref_idx_l1_active_minus1; i++)
        this->chroma_weight_l1_flag.push_back(reader.readFlag());
    for (unsigned i = 0; i <= slice->num_ref_idx_l1_active_minus1; i++)
    {
      if (luma_weight_l1_flag[i])
      {
        this->delta_luma_weight_l1.push_back(reader.readSEV());
        this->luma_offset_l1.push_back(reader.readSEV());
      }
      if (chroma_weight_l1_flag[i])
        for (unsigned j = 0; j < 2; j++)
        {
          this->delta_chroma_weight_l1.push_back(reader.readSEV());
          this->delta_chroma_offset_l1.push_back(reader.readSEV());
        }
    }
  }
}

void pred_weight_table::write(SubByteWriter &               writer,
                              const seq_parameter_set_rbsp &sps,
                              const slice_segment_header *  slice) const
{
  writer.writeUEV(this->luma_log2_weight_denom);
  if (sps.ChromaArrayType != 0)
    writer.writeSEV(this->delta_chroma_log2_weight_denom);
  for (unsigned i = 0; i <= slice->num_ref_idx_l0_active_minus1; i++)
    writer.writeFlag(this->luma_weight_l0_flag.at(i));
  if (sps.ChromaArrayType != 0)
    for (unsigned i = 0; i <= slice->num_ref_idx_l0_active_minus1; i++)
      writer.writeFlag(this->chroma_weight_l0_flag.at(i));
  for (unsigned i = 0; i <= slice->num_ref_idx_l0_active_minus1; i++)
  {
    if (this->luma_weight_l0_flag[i])
    {
      writer.writeSEV(this->delta_luma_weight_l0.at(i));
      writer.writeSEV(this->luma_offset_l0.at(i));
    }
    if (this->chroma_weight_l0_flag[i])
      for (unsigned j = 0; j < 2; j++)
      {
        writer.writeSEV(this->delta_chroma_weight_l0.at(i));
        writer.writeSEV(this->delta_chroma_offset_l0.at(i));
      }
  }

  if (slice->slice_type == SliceType::B)
  {
    for (unsigned i = 0; i <= slice->num_ref_idx_l1_active_minus1; i++)
      writer.writeFlag(this->luma_weight_l1_flag.at(i));
    if (sps.ChromaArrayType != 0)
      for (unsigned i = 0; i <= slice->num_ref_idx_l1_active_minus1; i++)
        writer.writeFlag(this->chroma_weight_l1_flag.at(i));
    for (unsigned i = 0; i <= slice->num_ref_idx_l1_active_minus1; i++)
    {
      if (luma_weight_l1_flag[i])
      {
        writer.writeSEV(this->delta_luma_weight_l1.at(i));
        writer.writeSEV(this->luma_offset_l1.at(i));
      }
      if (chroma_weight_l1_flag[i])
        for (unsigned j = 0; j < 2; j++)
        {
          writer.writeSEV(this->delta_chroma_weight_l1.at(i));
          writer.writeSEV(this->delta_chroma_offset_l1.at(i));
        }
    }
  }
}

} // namespace combiner::parser::hevc
