/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>
#include <common/SubByteWriter.h>

namespace combiner::parser::hevc
{

class profile_tier_level
{
public:
  profile_tier_level() {}

  void
  parse(SubByteReader &reader, const bool profilePresentFlag, const uint64_t maxNumSubLayersMinus1);
  void write(SubByteWriter &writer,
             const bool     profilePresentFlag,
             const uint64_t maxNumSubLayersMinus1) const;

  uint64_t      general_profile_space{};
  bool          general_tier_flag{};
  uint64_t      general_profile_idc{};
  boolArray<32> general_profile_compatibility_flag{};
  bool          general_progressive_source_flag{};
  bool          general_interlaced_source_flag{};
  bool          general_non_packed_constraint_flag{};
  bool          general_frame_only_constraint_flag{};

  bool general_max_12bit_constraint_flag{};
  bool general_max_10bit_constraint_flag{};
  bool general_max_8bit_constraint_flag{};
  bool general_max_422chroma_constraint_flag{};
  bool general_max_420chroma_constraint_flag{};
  bool general_max_monochrome_constraint_flag{};
  bool general_intra_constraint_flag{};
  bool general_one_picture_only_constraint_flag{};
  bool general_lower_bit_rate_constraint_flag{};
  bool general_inbld_flag{};

  uint64_t general_level_idc{};

  // A maximum of 8 sub-layer are allowed
  boolArray<8>            sub_layer_profile_present_flag{};
  boolArray<8>            sub_layer_level_present_flag{};
  std::array<uint64_t, 8> sub_layer_profile_space{};
  boolArray<8>            sub_layer_tier_flag{};
  std::array<uint64_t, 8> sub_layer_profile_idc{};
  boolArray2D<8, 32>      sub_layer_profile_compatibility_flag{{}};
  boolArray<8>            sub_layer_progressive_source_flag{};
  boolArray<8>            sub_layer_interlaced_source_flag{};
  boolArray<8>            sub_layer_non_packed_constraint_flag{};
  boolArray<8>            sub_layer_frame_only_constraint_flag{};
  boolArray<8>            sub_layer_max_12bit_constraint_flag{};
  boolArray<8>            sub_layer_max_10bit_constraint_flag{};
  boolArray<8>            sub_layer_max_8bit_constraint_flag{};
  boolArray<8>            sub_layer_max_422chroma_constraint_flag{};
  boolArray<8>            sub_layer_max_420chroma_constraint_flag{};
  boolArray<8>            sub_layer_max_monochrome_constraint_flag{};
  boolArray<8>            sub_layer_intra_constraint_flag{};
  boolArray<8>            sub_layer_one_picture_only_constraint_flag{};
  boolArray<8>            sub_layer_lower_bit_rate_constraint_flag{};
  boolArray<8>            sub_layer_inbld_flag{};
  std::array<uint64_t, 8> sub_layer_level_idc{};
};

} // namespace combiner::parser::hevc