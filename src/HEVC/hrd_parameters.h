/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>
#include <common/SubByteWriter.h>

#include "sub_layer_hrd_parameters.h"

namespace combiner::parser::hevc
{

// E.2.2 HRD parameters syntax
class hrd_parameters
{
public:
  hrd_parameters() {}

  void parse(SubByteReader &reader,
             const bool     commonInfPresentFlag,
             const uint64_t maxNumSubLayersMinus1);

  void write(SubByteWriter &writer,
             const bool     commonInfPresentFlag,
             const uint64_t maxNumSubLayersMinus1) const;

  bool nal_hrd_parameters_present_flag{};
  bool vcl_hrd_parameters_present_flag{};
  bool sub_pic_hrd_params_present_flag{};

  uint64_t tick_divisor_minus2;
  uint64_t du_cpb_removal_delay_increment_length_minus1{};
  bool     sub_pic_cpb_params_in_pic_timing_sei_flag{};
  uint64_t dpb_output_delay_du_length_minus1{};

  uint64_t bit_rate_scale{};
  uint64_t cpb_size_scale{};
  uint64_t cpb_size_du_scale{};
  uint64_t initial_cpb_removal_delay_length_minus1{23};
  uint64_t au_cpb_removal_delay_length_minus1{23};
  uint64_t dpb_output_delay_length_minus1{23};

  bool SubPicHrdPreferredFlag{};
  bool SubPicHrdFlag{};

  boolArray<8>            fixed_pic_rate_general_flag{};
  boolArray<8>            fixed_pic_rate_within_cvs_flag{};
  std::array<uint64_t, 8> elemental_duration_in_tc_minus1{};
  boolArray<8>            low_delay_hrd_flag{};
  std::array<uint64_t, 8> cpb_cnt_minus1{};

  std::array<sub_layer_hrd_parameters, 8> nal_sub_hrd;
  std::array<sub_layer_hrd_parameters, 8> vcl_sub_hrd;
};

} // namespace combiner::parser::hevc
