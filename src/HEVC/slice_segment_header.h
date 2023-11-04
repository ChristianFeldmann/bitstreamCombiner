/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>
#include <common/SubByteWriter.h>
#include <common/Typedef.h>

#include "commonMaps.h"
#include "nal_unit_header.h"
#include "pred_weight_table.h"
#include "ref_pic_lists_modification.h"
#include "st_ref_pic_set.h"

namespace combiner::parser::hevc
{

enum class SliceType
{
  I,
  B,
  P
};

class slice_segment_layer_rbsp;

std::string to_string(SliceType sliceType);

// T-REC-H.265-201410 - 7.3.6.1 slice_segment_header()
class slice_segment_header
{
public:
  slice_segment_header() {}

  void parse(SubByteReader &               reader,
             const bool                    firstAUInDecodingOrder,
             const uint64_t                prevTid0PicSlicePicOrderCntLsb,
             const int                     prevTid0PicPicOrderCntMsb,
             const nal_unit_header &       nalUnitHeader,
             const SPSMap &                spsMap,
             const PPSMap &                ppsMap,
             const std::optional<uint64_t> firstSliceInSegmentPicOrderCntLsb);

  void write(SubByteWriter &        writer,
             const bool             firstAUInDecodingOrder,
             const uint64_t         prevTid0PicSlicePicOrderCntLsb,
             const int              prevTid0PicPicOrderCntMsb,
             const nal_unit_header &nalUnitHeader,
             const SPSMap &         spsMap,
             const PPSMap &         ppsMap) const;

  bool              first_slice_segment_in_pic_flag{};
  bool              no_output_of_prior_pics_flag{};
  bool              dependent_slice_segment_flag{};
  uint64_t          slice_pic_parameter_set_id{};
  uint64_t          slice_segment_address{};
  vector<bool>      slice_reserved_flag;
  SliceType         slice_type{};
  bool              pic_output_flag{true};
  uint64_t          colour_plane_id{};
  uint64_t          slice_pic_order_cnt_lsb{};
  bool              short_term_ref_pic_set_sps_flag{};
  st_ref_pic_set    stRefPicSet{};
  uint64_t          short_term_ref_pic_set_idx{};
  uint64_t          num_long_term_sps{};
  uint64_t          num_long_term_pics{};
  umap_1d<uint64_t> lt_idx_sps;
  umap_1d<uint64_t> poc_lsb_lt;
  umap_1d<bool>     used_by_curr_pic_lt_flag;
  umap_1d<bool>     delta_poc_msb_present_flag;
  umap_1d<uint64_t> delta_poc_msb_cycle_lt;
  bool              slice_temporal_mvp_enabled_flag{};
  bool              slice_sao_luma_flag{};
  bool              slice_sao_chroma_flag{};
  bool              num_ref_idx_active_override_flag{};
  uint64_t          num_ref_idx_l0_active_minus1{};
  uint64_t          num_ref_idx_l1_active_minus1{};

  ref_pic_lists_modification refPicListsModification;

  bool              mvd_l1_zero_flag{};
  bool              cabac_init_flag{};
  bool              collocated_from_l0_flag{true};
  uint64_t          collocated_ref_idx{};
  pred_weight_table predWeightTable;
  uint64_t          five_minus_max_num_merge_cand{};
  int64_t           slice_qp_delta{};
  int64_t           slice_cb_qp_offset{};
  int64_t           slice_cr_qp_offset{};
  bool              cu_chroma_qp_offset_enabled_flag{};
  bool              deblocking_filter_override_flag{};
  bool              slice_deblocking_filter_disabled_flag{};
  int64_t           slice_beta_offset_div2{};
  int64_t           slice_tc_offset_div2{};
  bool              slice_loop_filter_across_slices_enabled_flag{};

  uint64_t         num_entry_point_offsets{};
  uint64_t         offset_len_minus1{};
  vector<uint64_t> entry_point_offset_minus1;

  uint64_t         slice_segment_header_extension_length{};
  vector<uint64_t> slice_segment_header_extension_data_byte;

  // Calculated values
  int         PicOrderCntVal{-1}; // The slice POC
  int         PicOrderCntMsb{-1};
  vector<int> UsedByCurrPicLt;
  bool        NoRaslOutputFlag{};

  int globalPOC{-1};
};

} // namespace combiner::parser::hevc