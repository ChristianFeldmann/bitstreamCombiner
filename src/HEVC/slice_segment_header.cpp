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

#include "slice_segment_header.h"

#include <common/CodingEnum.h>

#include "byte_alignment.h"
#include "pic_parameter_set_rbsp.h"
#include "seq_parameter_set_rbsp.h"
#include "slice_segment_layer_rbsp.h"

#include <cmath>

namespace combiner::parser::hevc
{

namespace
{

static parser::CodingEnum<SliceType> sliceTypeCoding({{0, SliceType::B, "B", "B-Slice"},
                                                      {1, SliceType::P, "P", "P-Slice"},
                                                      {2, SliceType::I, "I", "I-Slice"}},
                                                     SliceType::B);

}

void slice_segment_header::parse(SubByteReader &               reader,
                                 const bool                    firstAUInDecodingOrder,
                                 const uint64_t                prevTid0PicSlicePicOrderCntLsb,
                                 const int                     prevTid0PicPicOrderCntMsb,
                                 const nal_unit_header &       nalUnitHeader,
                                 const SPSMap &                spsMap,
                                 const PPSMap &                ppsMap,
                                 const std::optional<uint64_t> firstSliceInSegmentPicOrderCntLsb)
{
  this->first_slice_segment_in_pic_flag = reader.readFlag();

  if (nalUnitHeader.isIRAP())
    this->no_output_of_prior_pics_flag = reader.readFlag();

  this->slice_pic_parameter_set_id = reader.readUEV();

  if (ppsMap.count(this->slice_pic_parameter_set_id) == 0)
    throw std::logic_error("PPS with given slice_pic_parameter_set_id not found.");
  const auto &pps = ppsMap.at(this->slice_pic_parameter_set_id);

  if (spsMap.count(pps.pps_seq_parameter_set_id) == 0)
    throw std::logic_error("SPS with given pps_seq_parameter_set_id not found.");
  const auto &sps = spsMap.at(pps.pps_seq_parameter_set_id);

  if (!this->first_slice_segment_in_pic_flag)
  {
    if (pps.dependent_slice_segments_enabled_flag)
      this->dependent_slice_segment_flag = reader.readFlag();
    auto nrBits = static_cast<int>(std::ceil(std::log2(sps.PicSizeInCtbsY))); // 7.4.7.1
    this->slice_segment_address = reader.readBits(nrBits);
  }

  if (!this->dependent_slice_segment_flag)
  {
    for (unsigned int i = 0; i < pps.num_extra_slice_header_bits; i++)
      this->slice_reserved_flag.push_back(reader.readFlag());

    auto sliceTypeIdx = reader.readUEV();
    this->slice_type  = sliceTypeCoding.getValue(static_cast<unsigned>(sliceTypeIdx));
    if (pps.output_flag_present_flag)
      this->pic_output_flag = reader.readFlag();

    if (sps.separate_colour_plane_flag)
      this->colour_plane_id = reader.readBits(2);

    if (nalUnitHeader.nal_unit_type != NalType::IDR_W_RADL &&
        nalUnitHeader.nal_unit_type != NalType::IDR_N_LP)
    {
      this->slice_pic_order_cnt_lsb =
          reader.readBits(sps.log2_max_pic_order_cnt_lsb_minus4 + 4); // Max 16 bits read
      this->short_term_ref_pic_set_sps_flag = reader.readFlag();

      if (!this->short_term_ref_pic_set_sps_flag)
      {
        this->stRefPicSet.parse(
            reader, sps.num_short_term_ref_pic_sets, sps.num_short_term_ref_pic_sets);
      }
      else if (sps.num_short_term_ref_pic_sets > 1)
      {
        auto nrBits = static_cast<unsigned>(std::ceil(std::log2(sps.num_short_term_ref_pic_sets)));
        this->short_term_ref_pic_set_idx = reader.readBits(nrBits);

        // The short term ref pic set is the one with the given index from the SPS
        if (this->short_term_ref_pic_set_idx >= sps.stRefPicSets.size())
          throw std::logic_error(
              "Error parsing slice header. The specified short term ref pic list could not be "
              "found in the SPS.");
        this->stRefPicSet = sps.stRefPicSets[this->short_term_ref_pic_set_idx];
      }
      if (sps.long_term_ref_pics_present_flag)
      {
        if (sps.num_long_term_ref_pics_sps > 0)
          this->num_long_term_sps = reader.readUEV();
        this->num_long_term_pics = reader.readUEV();
        for (unsigned i = 0; i < this->num_long_term_sps + this->num_long_term_pics; i++)
        {
          if (i < this->num_long_term_sps)
          {
            if (sps.num_long_term_ref_pics_sps > 1)
            {
              auto nrBits =
                  static_cast<unsigned>(std::ceil(std::log2(sps.num_long_term_ref_pics_sps)));
              this->lt_idx_sps[i] = reader.readBits(nrBits);
            }

            this->UsedByCurrPicLt.push_back(sps.used_by_curr_pic_lt_sps_flag[this->lt_idx_sps[i]]);
          }
          else
          {
            auto nrBits                       = sps.log2_max_pic_order_cnt_lsb_minus4 + 4;
            this->poc_lsb_lt[i]               = reader.readBits(nrBits);
            this->used_by_curr_pic_lt_flag[i] = reader.readFlag();

            this->UsedByCurrPicLt.push_back(this->used_by_curr_pic_lt_flag[i]);
          }

          this->delta_poc_msb_present_flag[i] = reader.readFlag();
          if (this->delta_poc_msb_present_flag[i])
            this->delta_poc_msb_cycle_lt[i] = reader.readUEV();
        }
      }
      if (sps.sps_temporal_mvp_enabled_flag)
        this->slice_temporal_mvp_enabled_flag = reader.readFlag();
    }

    if (sps.sample_adaptive_offset_enabled_flag)
    {
      this->slice_sao_luma_flag = reader.readFlag();
      if (sps.ChromaArrayType != 0)
        this->slice_sao_chroma_flag = reader.readFlag();
    }

    if (this->slice_type == SliceType::P || this->slice_type == SliceType::B)
    {
      // Infer if not present
      this->num_ref_idx_l0_active_minus1 = pps.num_ref_idx_l0_default_active_minus1;
      this->num_ref_idx_l1_active_minus1 = pps.num_ref_idx_l1_default_active_minus1;

      this->num_ref_idx_active_override_flag = reader.readFlag();
      if (this->num_ref_idx_active_override_flag)
      {
        this->num_ref_idx_l0_active_minus1 = reader.readUEV();
        if (this->slice_type == SliceType::B)
          this->num_ref_idx_l1_active_minus1 = reader.readUEV();
      }

      auto CurrRpsIdx = (this->short_term_ref_pic_set_sps_flag) ? this->short_term_ref_pic_set_idx
                                                                : sps.num_short_term_ref_pic_sets;
      auto NumPicTotalCurr = this->stRefPicSet.NumPicTotalCurr(CurrRpsIdx, this);
      if (pps.lists_modification_present_flag && NumPicTotalCurr > 1)
        this->refPicListsModification.parse(reader, NumPicTotalCurr, this);

      if (this->slice_type == SliceType::B)
        this->mvd_l1_zero_flag = reader.readFlag();
      if (pps.cabac_init_present_flag)
        this->cabac_init_flag = reader.readFlag();
      if (this->slice_temporal_mvp_enabled_flag)
      {
        if (this->slice_type == SliceType::B)
          this->collocated_from_l0_flag = reader.readFlag();
        if ((this->collocated_from_l0_flag && this->num_ref_idx_l0_active_minus1 > 0) ||
            (!this->collocated_from_l0_flag && this->num_ref_idx_l1_active_minus1 > 0))
          this->collocated_ref_idx = reader.readUEV();
      }
      if ((pps.weighted_pred_flag && this->slice_type == SliceType::P) ||
          (pps.weighted_bipred_flag && this->slice_type == SliceType::B))
        this->predWeightTable.parse(reader, sps, this);

      this->five_minus_max_num_merge_cand = reader.readUEV();
    }
    this->slice_qp_delta = reader.readSEV();
    if (pps.pps_slice_chroma_qp_offsets_present_flag)
    {
      this->slice_cb_qp_offset = reader.readSEV();
      this->slice_cr_qp_offset = reader.readSEV();
    }
    if (pps.ppsRangeExtension.chroma_qp_offset_list_enabled_flag)
      this->cu_chroma_qp_offset_enabled_flag = reader.readFlag();
    if (pps.deblocking_filter_override_enabled_flag)
      this->deblocking_filter_override_flag = reader.readFlag();
    if (this->deblocking_filter_override_flag)
    {
      this->slice_deblocking_filter_disabled_flag = reader.readFlag();
      if (!this->slice_deblocking_filter_disabled_flag)
      {
        this->slice_beta_offset_div2 = reader.readSEV();
        this->slice_tc_offset_div2   = reader.readSEV();
      }
    }
    else
      this->slice_deblocking_filter_disabled_flag = pps.pps_deblocking_filter_disabled_flag;

    if (pps.pps_loop_filter_across_slices_enabled_flag &&
        (this->slice_sao_luma_flag || this->slice_sao_chroma_flag ||
         !this->slice_deblocking_filter_disabled_flag))
      this->slice_loop_filter_across_slices_enabled_flag = reader.readFlag();
  }
  else // dependent_slice_segment_flag is true -- inferr values from firstSliceInSegment
  {
    if (!firstSliceInSegmentPicOrderCntLsb)
      throw std::logic_error("Dependent slice without a first slice in the segment.");

    this->slice_pic_order_cnt_lsb = firstSliceInSegmentPicOrderCntLsb.value();
  }

  if (pps.tiles_enabled_flag || pps.entropy_coding_sync_enabled_flag)
  {
    this->num_entry_point_offsets = reader.readUEV();
    if (this->num_entry_point_offsets > 0)
    {
      this->offset_len_minus1 = reader.readUEV();

      for (unsigned i = 0; i < this->num_entry_point_offsets; i++)
      {
        auto nrBits = offset_len_minus1 + 1;
        this->entry_point_offset_minus1.push_back(reader.readBits(nrBits));
      }
    }
  }

  if (pps.slice_segment_header_extension_present_flag)
  {
    this->slice_segment_header_extension_length = reader.readUEV();
    for (unsigned i = 0; i < this->slice_segment_header_extension_length; i++)
      this->slice_segment_header_extension_data_byte.push_back(reader.readBits(8));
  }

  // End of the slice header - byte_alignment()

  // Calculate the picture order count
  auto MaxPicOrderCntLsb = 1u << (sps.log2_max_pic_order_cnt_lsb_minus4 + 4);

  // If the current picture is an IDR picture, a BLA picture, the first picture in the bitstream in
  // decoding order, or the first picture that follows an end of sequence NAL unit in decoding
  // order, the variable NoRaslOutputFlag is set equal to 1.
  this->NoRaslOutputFlag = false;
  if (nalUnitHeader.nal_unit_type == NalType::IDR_W_RADL ||
      nalUnitHeader.nal_unit_type == NalType::IDR_N_LP ||
      nalUnitHeader.nal_unit_type == NalType::BLA_W_LP)
    this->NoRaslOutputFlag = true;
  else if (firstAUInDecodingOrder)
  {
    this->NoRaslOutputFlag = true;
  }

  // T-REC-H.265-201410 - 8.3.1 Decoding process for picture order count

  int prevPicOrderCntLsb = 0;
  int prevPicOrderCntMsb = 0;
  // When the current picture is not an IRAP picture with NoRaslOutputFlag equal to 1, ...
  if (!(nalUnitHeader.isIRAP() && this->NoRaslOutputFlag))
  {
    // the variables prevPicOrderCntLsb and prevPicOrderCntMsb are derived as follows:
    prevPicOrderCntLsb = static_cast<int>(prevTid0PicSlicePicOrderCntLsb);
    prevPicOrderCntMsb = prevTid0PicPicOrderCntMsb;
  }

  // The variable PicOrderCntMsb of the current picture is derived as follows:
  if (nalUnitHeader.isIRAP() && this->NoRaslOutputFlag)
  {
    // If the current picture is an IRAP picture with NoRaslOutputFlag equal to 1, PicOrderCntMsb is
    // set equal to 0.
    PicOrderCntMsb = 0;
  }
  else
  {
    // Otherwise, PicOrderCntMsb is derived as follows: (8-1)
    if (((int)slice_pic_order_cnt_lsb < prevPicOrderCntLsb) &&
        (((int)prevPicOrderCntLsb - (int)slice_pic_order_cnt_lsb) >= ((int)MaxPicOrderCntLsb / 2)))
      PicOrderCntMsb = prevPicOrderCntMsb + MaxPicOrderCntLsb;
    else if (((int)slice_pic_order_cnt_lsb > prevPicOrderCntLsb) &&
             (((int)slice_pic_order_cnt_lsb - prevPicOrderCntLsb) > ((int)MaxPicOrderCntLsb / 2)))
      PicOrderCntMsb = prevPicOrderCntMsb - MaxPicOrderCntLsb;
    else
      PicOrderCntMsb = prevPicOrderCntMsb;
  }

  // PicOrderCntVal is derived as follows: (8-2)
  PicOrderCntVal = PicOrderCntMsb + static_cast<int>(slice_pic_order_cnt_lsb);

  byte_alignment::parse(reader);
}

void slice_segment_header::write(SubByteWriter &        writer,
                                 const nal_unit_header &nalUnitHeader,
                                 const SPSMap &         spsMap,
                                 const PPSMap &         ppsMap) const
{
  writer.writeFlag(this->first_slice_segment_in_pic_flag);

  if (nalUnitHeader.isIRAP())
    writer.writeFlag(this->no_output_of_prior_pics_flag);

  writer.writeUEV(this->slice_pic_parameter_set_id);

  if (ppsMap.count(this->slice_pic_parameter_set_id) == 0)
    throw std::logic_error("PPS with given slice_pic_parameter_set_id not found.");
  const auto &pps = ppsMap.at(this->slice_pic_parameter_set_id);

  if (spsMap.count(pps.pps_seq_parameter_set_id) == 0)
    throw std::logic_error("SPS with given pps_seq_parameter_set_id not found.");
  const auto &sps = spsMap.at(pps.pps_seq_parameter_set_id);

  if (!this->first_slice_segment_in_pic_flag)
  {
    if (pps.dependent_slice_segments_enabled_flag)
      writer.writeFlag(this->dependent_slice_segment_flag);
    auto nrBits = static_cast<int>(std::ceil(std::log2(sps.PicSizeInCtbsY))); // 7.4.7.1
    writer.writeBits(this->slice_segment_address, nrBits);
  }

  if (!this->dependent_slice_segment_flag)
  {
    for (unsigned int i = 0; i < pps.num_extra_slice_header_bits; i++)
      writer.writeFlag(this->slice_reserved_flag.at(i));

    const auto sliceTypeIdx = sliceTypeCoding.getCode(this->slice_type);
    writer.writeUEV(sliceTypeIdx);
    if (pps.output_flag_present_flag)
      writer.writeFlag(this->pic_output_flag);

    if (sps.separate_colour_plane_flag)
      writer.writeBits(this->colour_plane_id, 2);

    if (nalUnitHeader.nal_unit_type != NalType::IDR_W_RADL &&
        nalUnitHeader.nal_unit_type != NalType::IDR_N_LP)
    {
      writer.writeBits(this->slice_pic_order_cnt_lsb, sps.log2_max_pic_order_cnt_lsb_minus4 + 4);
      writer.writeFlag(this->short_term_ref_pic_set_sps_flag);

      if (!this->short_term_ref_pic_set_sps_flag)
      {
        this->stRefPicSet.write(
            writer, sps.num_short_term_ref_pic_sets, sps.num_short_term_ref_pic_sets);
      }
      else if (sps.num_short_term_ref_pic_sets > 1)
      {
        auto nrBits = static_cast<unsigned>(std::ceil(std::log2(sps.num_short_term_ref_pic_sets)));
        writer.writeBits(this->short_term_ref_pic_set_idx, nrBits);

        // The short term ref pic set is the one with the given index from the SPS
        if (this->short_term_ref_pic_set_idx >= sps.stRefPicSets.size())
          throw std::logic_error(
              "Error writing slice header. The specified short term ref pic list could not be "
              "found in the SPS.");
      }
      if (sps.long_term_ref_pics_present_flag)
      {
        if (sps.num_long_term_ref_pics_sps > 0)
          writer.writeUEV(this->num_long_term_sps);
        writer.writeUEV(this->num_long_term_pics);
        for (unsigned i = 0; i < this->num_long_term_sps + this->num_long_term_pics; i++)
        {
          if (i < this->num_long_term_sps)
          {
            if (sps.num_long_term_ref_pics_sps > 1)
            {
              const auto nrBits =
                  static_cast<unsigned>(std::ceil(std::log2(sps.num_long_term_ref_pics_sps)));
              writer.writeBits(this->lt_idx_sps.at(i), nrBits);
            }
          }
          else
          {
            const auto nrBits = sps.log2_max_pic_order_cnt_lsb_minus4 + 4;
            writer.writeBits(this->poc_lsb_lt.at(i), nrBits);
            writer.writeFlag(this->used_by_curr_pic_lt_flag.at(i));
          }

          writer.writeFlag(this->delta_poc_msb_present_flag.at(i));
          if (this->delta_poc_msb_present_flag.at(i))
            writer.writeUEV(this->delta_poc_msb_cycle_lt.at(i));
        }
      }
      if (sps.sps_temporal_mvp_enabled_flag)
        writer.writeFlag(this->slice_temporal_mvp_enabled_flag);
    }

    if (sps.sample_adaptive_offset_enabled_flag)
    {
      writer.writeFlag(this->slice_sao_luma_flag);
      if (sps.ChromaArrayType != 0)
        writer.writeFlag(this->slice_sao_chroma_flag);
    }

    if (this->slice_type == SliceType::P || this->slice_type == SliceType::B)
    {
      writer.writeFlag(this->num_ref_idx_active_override_flag);
      if (this->num_ref_idx_active_override_flag)
      {
        writer.writeUEV(this->num_ref_idx_l0_active_minus1);
        if (this->slice_type == SliceType::B)
          writer.writeUEV(this->num_ref_idx_l1_active_minus1);
      }

      const auto CurrRpsIdx      = (this->short_term_ref_pic_set_sps_flag)
                                       ? this->short_term_ref_pic_set_idx
                                       : sps.num_short_term_ref_pic_sets;
      const auto NumPicTotalCurr = this->stRefPicSet.NumPicTotalCurr(CurrRpsIdx, this);
      if (pps.lists_modification_present_flag && NumPicTotalCurr > 1)
        this->refPicListsModification.write(writer, NumPicTotalCurr, this);

      if (this->slice_type == SliceType::B)
        writer.writeFlag(this->mvd_l1_zero_flag);
      if (pps.cabac_init_present_flag)
        writer.writeFlag(this->cabac_init_flag);
      if (this->slice_temporal_mvp_enabled_flag)
      {
        if (this->slice_type == SliceType::B)
          writer.writeFlag(this->collocated_from_l0_flag);
        if ((this->collocated_from_l0_flag && this->num_ref_idx_l0_active_minus1 > 0) ||
            (!this->collocated_from_l0_flag && this->num_ref_idx_l1_active_minus1 > 0))
          writer.writeUEV(this->collocated_ref_idx);
      }
      if ((pps.weighted_pred_flag && this->slice_type == SliceType::P) ||
          (pps.weighted_bipred_flag && this->slice_type == SliceType::B))
        this->predWeightTable.write(writer, sps, this);

      writer.writeUEV(this->five_minus_max_num_merge_cand);
    }
    writer.writeSEV(this->slice_qp_delta);
    if (pps.pps_slice_chroma_qp_offsets_present_flag)
    {
      writer.writeSEV(this->slice_cb_qp_offset);
      writer.writeSEV(this->slice_cr_qp_offset);
    }
    if (pps.ppsRangeExtension.chroma_qp_offset_list_enabled_flag)
      writer.writeFlag(this->cu_chroma_qp_offset_enabled_flag);
    if (pps.deblocking_filter_override_enabled_flag)
      writer.writeFlag(this->deblocking_filter_override_flag);
    if (this->deblocking_filter_override_flag)
    {
      writer.writeFlag(this->slice_deblocking_filter_disabled_flag);
      if (!this->slice_deblocking_filter_disabled_flag)
      {
        writer.writeSEV(this->slice_beta_offset_div2);
        writer.writeSEV(this->slice_tc_offset_div2);
      }
    }

    if (pps.pps_loop_filter_across_slices_enabled_flag &&
        (this->slice_sao_luma_flag || this->slice_sao_chroma_flag ||
         !this->slice_deblocking_filter_disabled_flag))
      writer.writeFlag(this->slice_loop_filter_across_slices_enabled_flag);
  }
  else
  {
    // dependent_slice_segment_flag is true -- inferr values from firstSliceInSegment
  }

  if (pps.tiles_enabled_flag || pps.entropy_coding_sync_enabled_flag)
  {
    writer.writeUEV(this->num_entry_point_offsets);
    if (this->num_entry_point_offsets > 0)
    {
      writer.writeUEV(this->offset_len_minus1);

      for (unsigned i = 0; i < this->num_entry_point_offsets; i++)
      {
        const auto nrBits = offset_len_minus1 + 1;
        writer.writeBits(this->entry_point_offset_minus1.at(i), nrBits);
      }
    }
  }

  if (pps.slice_segment_header_extension_present_flag)
  {
    writer.writeUEV(this->slice_segment_header_extension_length);
    for (unsigned i = 0; i < this->slice_segment_header_extension_length; i++)
      writer.writeBits(this->slice_segment_header_extension_data_byte.at(i), 8);
  }

  byte_alignment::write(writer);
}

} // namespace combiner::parser::hevc
