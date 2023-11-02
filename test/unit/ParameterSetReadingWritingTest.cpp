/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include <gtest/gtest.h>

#include <HEVC/seq_parameter_set_rbsp.h>
#include <HEVC/video_parameter_set_rbsp.h>
#include <common/SubByteReader.h>
#include <common/SubByteWriter.h>
#include <common/Typedef.h>

namespace combiner
{

template <std::size_t N>
bool checkForSetFlags(const std::array<bool, N> &flagArray, std::initializer_list<int> setIndices)
{
  for (int i = 0; i < N; ++i)
  {
    auto expectedFlag = false;
    if (std::find(setIndices.begin(), setIndices.end(), i) != setIndices.end())
      expectedFlag = true;
    if (flagArray.at(i) != expectedFlag)
      return false;
  }
  return true;
}

TEST(ParameterSet, TestReadingAndWriting_VPS)
{
  const ByteVector rawVPSTestData = {0x0C, 0x01, 0xFF, 0xFF, 0x01, 0x60, 0x00, 0x00,
                                     0x03, 0x00, 0xB0, 0x00, 0x00, 0x03, 0x00, 0x00,
                                     0x03, 0x00, 0x5A, 0x11, 0x40, 0xC0, 0x00, 0x00,
                                     0x03, 0x00, 0x40, 0x00, 0x00, 0x07, 0xBA};

  parser::SubByteReader reader(rawVPSTestData);

  parser::hevc::video_parameter_set_rbsp vps;
  vps.parse(reader);

  EXPECT_EQ(vps.vps_video_parameter_set_id, 0);
  EXPECT_EQ(vps.vps_base_layer_internal_flag, true);
  EXPECT_EQ(vps.vps_base_layer_available_flag, true);
  EXPECT_EQ(vps.vps_max_layers_minus1, 0);
  EXPECT_EQ(vps.vps_max_sub_layers_minus1, 0);
  EXPECT_EQ(vps.vps_temporal_id_nesting_flag, 1);

  const auto &ptl = vps.profileTierLevel;
  EXPECT_EQ(ptl.general_profile_space, 0);
  EXPECT_EQ(ptl.general_tier_flag, 0);
  EXPECT_EQ(ptl.general_profile_idc, 1);
  EXPECT_TRUE(checkForSetFlags(ptl.general_profile_compatibility_flag, {1, 2}));
  EXPECT_EQ(ptl.general_progressive_source_flag, true);
  EXPECT_EQ(ptl.general_interlaced_source_flag, false);
  EXPECT_EQ(ptl.general_non_packed_constraint_flag, true);
  EXPECT_EQ(ptl.general_frame_only_constraint_flag, true);
  EXPECT_EQ(ptl.general_inbld_flag, false);
  EXPECT_EQ(ptl.general_level_idc, 90);

  EXPECT_EQ(vps.vps_sub_layer_ordering_info_present_flag, false);
  EXPECT_EQ(vps.vps_max_dec_pic_buffering_minus1[0], 3);
  EXPECT_EQ(vps.vps_max_num_reorder_pics[0], 1);
  EXPECT_EQ(vps.vps_max_latency_increase_plus1[0], 0);
  EXPECT_EQ(vps.vps_max_layer_id, 0);
  EXPECT_EQ(vps.vps_timing_info_present_flag, true);
  EXPECT_EQ(vps.vps_num_units_in_tick, 1);
  EXPECT_EQ(vps.vps_time_scale, 30);
  EXPECT_EQ(vps.vps_poc_proportional_to_timing_flag, true);
  EXPECT_EQ(vps.vps_num_ticks_poc_diff_one_minus1, 0);
  EXPECT_EQ(vps.vps_num_hrd_parameters, 0);
  EXPECT_EQ(vps.vps_extension_flag, false);

  parser::SubByteWriter writer;
  vps.write(writer);

  const auto writtenData = writer.finishWritingAndGetData();

  EXPECT_EQ(rawVPSTestData.size(), writtenData.size());
  EXPECT_TRUE(std::equal(rawVPSTestData.begin(), rawVPSTestData.end(), writtenData.begin()));
}

TEST(ParameterSet, TestReadingAndWriting_SPS)
{
  const ByteVector rawSPSTestData = {
      0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0xB0, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
      0x00, 0x5A, 0xA0, 0x07, 0x82, 0x00, 0x88, 0x7D, 0xE5, 0x11, 0x64, 0x91, 0x4A, 0x70,
      0x16, 0xA0, 0x20, 0x20, 0x22, 0x80, 0x00, 0x00, 0x03, 0x00, 0x80, 0x00, 0x00, 0x0F,
      0x78, 0x25, 0x7B, 0x9F, 0x80, 0x02, 0x49, 0xF0, 0x00, 0x36, 0xEE, 0xC8};

  parser::SubByteReader reader(rawSPSTestData);

  parser::hevc::seq_parameter_set_rbsp sps;
  sps.parse(reader);

  EXPECT_EQ(sps.sps_video_parameter_set_id, 0);
  EXPECT_EQ(sps.sps_max_sub_layers_minus1, 0);
  EXPECT_EQ(sps.sps_temporal_id_nesting_flag, true);

  const auto &ptl = sps.profileTierLevel;
  EXPECT_EQ(ptl.general_profile_space, 0);
  EXPECT_EQ(ptl.general_tier_flag, 0);
  EXPECT_EQ(ptl.general_profile_idc, 1);
  EXPECT_TRUE(checkForSetFlags(ptl.general_profile_compatibility_flag, {1, 2}));
  EXPECT_EQ(ptl.general_progressive_source_flag, true);
  EXPECT_EQ(ptl.general_interlaced_source_flag, false);
  EXPECT_EQ(ptl.general_non_packed_constraint_flag, true);
  EXPECT_EQ(ptl.general_frame_only_constraint_flag, true);
  EXPECT_EQ(ptl.general_inbld_flag, false);
  EXPECT_EQ(ptl.general_level_idc, 90);

  EXPECT_EQ(sps.sps_seq_parameter_set_id, 0);
  EXPECT_EQ(sps.chroma_format_idc, 1);
  EXPECT_EQ(sps.pic_width_in_luma_samples, 960);
  EXPECT_EQ(sps.pic_height_in_luma_samples, 544);
  EXPECT_EQ(sps.conformance_window_flag, true);
  EXPECT_EQ(sps.conf_win_left_offset, 0);
  EXPECT_EQ(sps.conf_win_right_offset, 0);
  EXPECT_EQ(sps.conf_win_top_offset, 0);
  EXPECT_EQ(sps.conf_win_bottom_offset, 2);
  EXPECT_EQ(sps.bit_depth_luma_minus8, 0);
  EXPECT_EQ(sps.bit_depth_chroma_minus8, 0);
  EXPECT_EQ(sps.log2_max_pic_order_cnt_lsb_minus4, 4);
  EXPECT_EQ(sps.sps_sub_layer_ordering_info_present_flag, false);
  EXPECT_EQ(sps.sps_max_dec_pic_buffering_minus1[0], 3);
  EXPECT_EQ(sps.sps_max_num_reorder_pics[0], 1);
  EXPECT_EQ(sps.sps_max_latency_increase_plus1[0], 0);
  EXPECT_EQ(sps.log2_min_luma_coding_block_size_minus3, 0);
  EXPECT_EQ(sps.log2_diff_max_min_luma_coding_block_size, 3);
  EXPECT_EQ(sps.log2_min_luma_transform_block_size_minus2, 0);
  EXPECT_EQ(sps.log2_diff_max_min_luma_transform_block_size, 3);
  EXPECT_EQ(sps.max_transform_hierarchy_depth_inter, 1);
  EXPECT_EQ(sps.max_transform_hierarchy_depth_intra, 0);
  EXPECT_EQ(sps.scaling_list_enabled_flag, false);
  EXPECT_EQ(sps.amp_enabled_flag, 0);
  EXPECT_EQ(sps.sample_adaptive_offset_enabled_flag, true);
  EXPECT_EQ(sps.pcm_enabled_flag, false);
  EXPECT_EQ(sps.num_short_term_ref_pic_sets, 0);
  EXPECT_EQ(sps.long_term_ref_pics_present_flag, false);
  EXPECT_EQ(sps.sps_temporal_mvp_enabled_flag, false);
  EXPECT_EQ(sps.strong_intra_smoothing_enabled_flag, true);
  EXPECT_EQ(sps.vui_parameters_present_flag, true);

  const auto &vui = sps.vuiParameters;
  EXPECT_EQ(vui.aspect_ratio_info_present_flag, true);
  EXPECT_EQ(vui.aspect_ratio_idc, 1);
  EXPECT_EQ(vui.overscan_info_present_flag, false);
  EXPECT_EQ(vui.video_signal_type_present_flag, true);
  EXPECT_EQ(vui.video_format, 5);
  EXPECT_EQ(vui.video_full_range_flag, false);
  EXPECT_EQ(vui.colour_description_present_flag, true);
  EXPECT_EQ(vui.colour_primaries, 1);
  EXPECT_EQ(vui.transfer_characteristics, 1);
  EXPECT_EQ(vui.matrix_coeffs, 1);
  EXPECT_EQ(vui.chroma_loc_info_present_flag, false);
  EXPECT_EQ(vui.neutral_chroma_indication_flag, false);
  EXPECT_EQ(vui.field_seq_flag, false);
  EXPECT_EQ(vui.frame_field_info_present_flag, true);
  EXPECT_EQ(vui.default_display_window_flag, false);
  EXPECT_EQ(vui.vui_timing_info_present_flag, true);
  EXPECT_EQ(vui.vui_time_scale, 30);
  EXPECT_EQ(vui.vui_poc_proportional_to_timing_flag, true);
  EXPECT_EQ(vui.vui_num_ticks_poc_diff_one_minus1, 0);
  EXPECT_EQ(vui.vui_hrd_parameters_present_flag, true);

  const auto &hrd = vui.hrdParameters;
  EXPECT_EQ(hrd.nal_hrd_parameters_present_flag, true);
  EXPECT_EQ(hrd.vcl_hrd_parameters_present_flag, false);
  EXPECT_EQ(hrd.sub_pic_hrd_params_present_flag, false);
  EXPECT_EQ(hrd.bit_rate_scale, 1);
  EXPECT_EQ(hrd.cpb_size_scale, 2);
  EXPECT_EQ(hrd.initial_cpb_removal_delay_length_minus1, 23);
  EXPECT_EQ(hrd.au_cpb_removal_delay_length_minus1, 23);
  EXPECT_EQ(hrd.dpb_output_delay_length_minus1, 7);
  EXPECT_EQ(hrd.fixed_pic_rate_general_flag[0], true);
  EXPECT_EQ(hrd.elemental_duration_in_tc_minus1[0], 0);
  EXPECT_EQ(hrd.cpb_cnt_minus1[0], 0);

  const auto &subLayerHrd = hrd.nal_sub_hrd[0];
  EXPECT_EQ(subLayerHrd.bit_rate_value_minus1[0], 9374);
  EXPECT_EQ(subLayerHrd.cpb_size_value_minus1[0], 28124);

  EXPECT_EQ(vui.bitstream_restriction_flag, false);

  EXPECT_EQ(sps.sps_extension_present_flag, false);

  parser::SubByteWriter writer;
  sps.write(writer);

  const auto writtenData = writer.finishWritingAndGetData();

  EXPECT_EQ(rawSPSTestData.size(), writtenData.size());
  EXPECT_TRUE(std::equal(rawSPSTestData.begin(), rawSPSTestData.end(), writtenData.begin()));
}

} // namespace combiner
