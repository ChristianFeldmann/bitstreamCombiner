/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>

#include "NalUnitHEVC.h"
#include "hrd_parameters.h"
#include "profile_tier_level.h"
#include "rbsp_trailing_bits.h"

namespace combiner::parser::hevc
{

// The video parameter set. 7.3.2.1
class video_parameter_set_rbsp : public NalRBSP
{
public:
  video_parameter_set_rbsp() {}

  void parse(SubByteReader &reader);

  uint64_t vps_video_parameter_set_id{};
  bool     vps_base_layer_internal_flag{};
  bool     vps_base_layer_available_flag{};
  uint64_t vps_max_layers_minus1{};
  uint64_t vps_max_sub_layers_minus1{};
  bool     vps_temporal_id_nesting_flag{};
  uint64_t vps_reserved_0xffff_16bits{};

  profile_tier_level profileTierLevel;

  bool         vps_sub_layer_ordering_info_present_flag{};
  uint64_t     vps_max_dec_pic_buffering_minus1[7]{};
  uint64_t     vps_max_num_reorder_pics[7]{};
  uint64_t     vps_max_latency_increase_plus1[7]{};
  uint64_t     vps_max_layer_id{};
  uint64_t     vps_num_layer_sets_minus1{};
  vector<bool> layer_id_included_flag[7]{};

  bool             vps_timing_info_present_flag{};
  uint64_t         vps_num_units_in_tick{};
  uint64_t         vps_time_scale{};
  bool             vps_poc_proportional_to_timing_flag{};
  uint64_t         vps_num_ticks_poc_diff_one_minus1{};
  uint64_t         vps_num_hrd_parameters{};
  vector<uint64_t> hrd_layer_set_idx;
  vector<bool>     cprms_present_flag;

  vector<hrd_parameters> vps_hrd_parameters;
  bool                   vps_extension_flag{};

  rbsp_trailing_bits rbspTrailingBits;

  // Calculated values
  double frameRate{};
};

} // namespace combiner::parser::hevc