/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include <gtest/gtest.h>

#include <HEVC/pic_parameter_set_rbsp.h>
#include <HEVC/seq_parameter_set_rbsp.h>
#include <HEVC/slice_segment_layer_rbsp.h>

#include "Functions.h"
#include "TestFileData.h"

namespace combiner
{

namespace
{

using namespace parser::hevc;

ActiveParameterSets parseActiveParameterSetsFromData()
{
  ActiveParameterSets activeParameterSets;
  activeParameterSets.spsMap[0] =
      parserParameterSetFromData<parser::hevc::seq_parameter_set_rbsp>(RAW_SPS_DATA);
  activeParameterSets.ppsMap[0] = parserParameterSetFromData<pic_parameter_set_rbsp>(RAW_PPS_DATA);
  return activeParameterSets;
}

} // namespace

using FirstAUInDecodingOrder                      = bool;
constexpr uint64_t prevTid0PicSlicePicOrderCntLsb = 0;
constexpr int      prevTid0PicPicOrderCntMsb      = 0;

TEST(SliceHeader, TestParsingAndWritingOfSliceHeaderFrame0)
{
  const auto activeParameterSets = parseActiveParameterSetsFromData();

  parser::SubByteReader reader(RAW_SLICE_HEADER_DATA_SLICE_0);

  slice_segment_layer_rbsp sliceHeader;
  sliceHeader.parse(reader,
                    FirstAUInDecodingOrder(true),
                    prevTid0PicSlicePicOrderCntLsb,
                    prevTid0PicPicOrderCntMsb,
                    nal_unit_header(NalType::IDR_N_LP),
                    activeParameterSets,
                    {});

  const auto &ssh = sliceHeader.sliceSegmentHeader;

  EXPECT_EQ(ssh.first_slice_segment_in_pic_flag, true);
  EXPECT_EQ(ssh.no_output_of_prior_pics_flag, false);
  EXPECT_EQ(ssh.slice_pic_parameter_set_id, 0);
  EXPECT_EQ(ssh.slice_type, SliceType::I);
  EXPECT_EQ(ssh.slice_sao_luma_flag, true);
  EXPECT_EQ(ssh.slice_sao_chroma_flag, true);
  EXPECT_EQ(ssh.slice_qp_delta, -4);
  EXPECT_EQ(ssh.slice_cb_qp_offset, -6);
  EXPECT_EQ(ssh.slice_cr_qp_offset, -6);
  EXPECT_EQ(ssh.deblocking_filter_override_flag, true);
  EXPECT_EQ(ssh.slice_deblocking_filter_disabled_flag, false);
  EXPECT_EQ(ssh.slice_beta_offset_div2, 0);
  EXPECT_EQ(ssh.slice_tc_offset_div2, 0);
  EXPECT_EQ(ssh.slice_loop_filter_across_slices_enabled_flag, true);

  parser::SubByteWriter writer;
  ssh.write(writer, nal_unit_header(NalType::IDR_N_LP), activeParameterSets);
  const auto writtenData = writer.finishWritingAndGetData();

  EXPECT_EQ(RAW_SLICE_HEADER_DATA_SLICE_0.size(), writtenData.size());
  EXPECT_TRUE(std::equal(RAW_SLICE_HEADER_DATA_SLICE_0.begin(),
                         RAW_SLICE_HEADER_DATA_SLICE_0.end(),
                         writtenData.begin()));
}

TEST(SliceHeader, TestParsingAndWritingOfSliceHeaderFrame1)
{
  const auto activeParameterSets = parseActiveParameterSetsFromData();

  constexpr uint64_t firstSliceInSegmentPicOrderCntLsb = 0;

  parser::SubByteReader reader(RAW_SLICE_HEADER_DATA_SLICE_1);

  slice_segment_layer_rbsp sliceHeader;
  sliceHeader.parse(reader,
                    FirstAUInDecodingOrder(false),
                    prevTid0PicSlicePicOrderCntLsb,
                    prevTid0PicPicOrderCntMsb,
                    nal_unit_header(NalType::TRAIL_R),
                    activeParameterSets,
                    firstSliceInSegmentPicOrderCntLsb);

  const auto &ssh = sliceHeader.sliceSegmentHeader;

  EXPECT_EQ(ssh.first_slice_segment_in_pic_flag, true);
  EXPECT_EQ(ssh.slice_pic_parameter_set_id, 0);
  EXPECT_EQ(ssh.slice_type, SliceType::P);
  EXPECT_EQ(ssh.slice_pic_order_cnt_lsb, 4);
  EXPECT_EQ(ssh.short_term_ref_pic_set_sps_flag, false);

  EXPECT_EQ(ssh.stRefPicSet.num_negative_pics, 1);
  EXPECT_EQ(ssh.stRefPicSet.num_positive_pics, 0);
  EXPECT_EQ(ssh.stRefPicSet.delta_poc_s0_minus1.at(0), 3);
  EXPECT_EQ(ssh.stRefPicSet.used_by_curr_pic_s0_flag.at(0), true);

  EXPECT_EQ(ssh.slice_sao_luma_flag, true);
  EXPECT_EQ(ssh.slice_sao_chroma_flag, true);
  EXPECT_EQ(ssh.num_ref_idx_active_override_flag, true);
  EXPECT_EQ(ssh.num_ref_idx_l0_active_minus1, 0);
  EXPECT_EQ(ssh.cabac_init_flag, false);
  EXPECT_EQ(ssh.five_minus_max_num_merge_cand, 3);
  EXPECT_EQ(ssh.slice_qp_delta, -2);
  EXPECT_EQ(ssh.slice_cb_qp_offset, 0);
  EXPECT_EQ(ssh.slice_cr_qp_offset, 0);
  EXPECT_EQ(ssh.deblocking_filter_override_flag, true);
  EXPECT_EQ(ssh.slice_deblocking_filter_disabled_flag, false);

  parser::SubByteWriter writer;
  ssh.write(writer, nal_unit_header(NalType::TRAIL_R), activeParameterSets);
  const auto writtenData = writer.finishWritingAndGetData();

  EXPECT_EQ(RAW_SLICE_HEADER_DATA_SLICE_1.size(), writtenData.size());
  EXPECT_TRUE(std::equal(RAW_SLICE_HEADER_DATA_SLICE_1.begin(),
                         RAW_SLICE_HEADER_DATA_SLICE_1.end(),
                         writtenData.begin()));
}

TEST(SliceHeader, TestParsingAndWritingOfSliceHeaderFrame2)
{
  const auto activeParameterSets = parseActiveParameterSetsFromData();

  constexpr uint64_t firstSliceInSegmentPicOrderCntLsb = 0;

  parser::SubByteReader reader(RAW_SLICE_HEADER_DATA_SLICE_2);

  slice_segment_layer_rbsp sliceHeader;
  sliceHeader.parse(reader,
                    FirstAUInDecodingOrder(false),
                    prevTid0PicSlicePicOrderCntLsb,
                    prevTid0PicPicOrderCntMsb,
                    nal_unit_header(NalType::TRAIL_N),
                    activeParameterSets,
                    firstSliceInSegmentPicOrderCntLsb);

  const auto &ssh = sliceHeader.sliceSegmentHeader;

  EXPECT_EQ(ssh.first_slice_segment_in_pic_flag, true);
  EXPECT_EQ(ssh.slice_pic_parameter_set_id, 0);
  EXPECT_EQ(ssh.slice_type, SliceType::B);
  EXPECT_EQ(ssh.slice_pic_order_cnt_lsb, 1);
  EXPECT_EQ(ssh.short_term_ref_pic_set_sps_flag, false);

  EXPECT_EQ(ssh.stRefPicSet.num_negative_pics, 1);
  EXPECT_EQ(ssh.stRefPicSet.num_positive_pics, 1);
  EXPECT_EQ(ssh.stRefPicSet.delta_poc_s0_minus1.at(0), 0);
  EXPECT_EQ(ssh.stRefPicSet.used_by_curr_pic_s0_flag.at(0), true);
  EXPECT_EQ(ssh.stRefPicSet.delta_poc_s1_minus1.at(0), 2);
  EXPECT_EQ(ssh.stRefPicSet.used_by_curr_pic_s1_flag.at(0), true);

  EXPECT_EQ(ssh.slice_sao_luma_flag, true);
  EXPECT_EQ(ssh.slice_sao_chroma_flag, true);
  EXPECT_EQ(ssh.num_ref_idx_active_override_flag, true);
  EXPECT_EQ(ssh.num_ref_idx_l0_active_minus1, 0);
  EXPECT_EQ(ssh.num_ref_idx_l1_active_minus1, 0);
  EXPECT_EQ(ssh.mvd_l1_zero_flag, false);
  EXPECT_EQ(ssh.cabac_init_flag, false);
  EXPECT_EQ(ssh.five_minus_max_num_merge_cand, 3);
  EXPECT_EQ(ssh.slice_qp_delta, 1);
  EXPECT_EQ(ssh.slice_cb_qp_offset, 0);
  EXPECT_EQ(ssh.slice_cr_qp_offset, 0);
  EXPECT_EQ(ssh.deblocking_filter_override_flag, true);
  EXPECT_EQ(ssh.slice_deblocking_filter_disabled_flag, false);
  EXPECT_EQ(ssh.slice_beta_offset_div2, 1);
  EXPECT_EQ(ssh.slice_tc_offset_div2, 1);
  EXPECT_EQ(ssh.slice_loop_filter_across_slices_enabled_flag, true);

  parser::SubByteWriter writer;
  ssh.write(writer, nal_unit_header(NalType::TRAIL_N), activeParameterSets);
  const auto writtenData = writer.finishWritingAndGetData();

  EXPECT_EQ(RAW_SLICE_HEADER_DATA_SLICE_2.size(), writtenData.size());
  EXPECT_TRUE(std::equal(RAW_SLICE_HEADER_DATA_SLICE_2.begin(),
                         RAW_SLICE_HEADER_DATA_SLICE_2.end(),
                         writtenData.begin()));
}

} // namespace combiner