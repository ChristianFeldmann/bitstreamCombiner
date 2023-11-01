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

class slice_segment_header;

// 7.3.7 Short-term reference picture set syntax
class st_ref_pic_set
{
public:
  st_ref_pic_set() {}

  void
  parse(SubByteReader &reader, const uint64_t stRpsIdx, const uint64_t num_short_term_ref_pic_sets);
  void write(SubByteWriter &writer,
             const uint64_t stRpsIdx,
             const uint64_t num_short_term_ref_pic_sets) const;

  unsigned NumPicTotalCurr(const uint64_t CurrRpsIdx, const slice_segment_header *slice);

  bool         inter_ref_pic_set_prediction_flag{};
  uint64_t     delta_idx_minus1{};
  bool         delta_rps_sign{};
  uint64_t     abs_delta_rps_minus1{};
  vector<bool> used_by_curr_pic_flag;
  vector<bool> use_delta_flag;

  uint64_t         num_negative_pics{};
  uint64_t         num_positive_pics{};
  vector<uint64_t> delta_poc_s0_minus1;
  vector<bool>     used_by_curr_pic_s0_flag;
  vector<uint64_t> delta_poc_s1_minus1;
  vector<bool>     used_by_curr_pic_s1_flag;

  // Calculated values. These are static. They are used for reference picture set prediction.
  static uint64_t NumNegativePics[65];
  static uint64_t NumPositivePics[65];
  static int      DeltaPocS0[65][16];
  static int      DeltaPocS1[65][16];
  static bool     UsedByCurrPicS0[65][16];
  static bool     UsedByCurrPicS1[65][16];
  static uint64_t NumDeltaPocs[65];
};

} // namespace combiner::parser::hevc