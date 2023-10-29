/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>

namespace combiner::parser::hevc
{

class slice_segment_header;

class ref_pic_lists_modification
{
public:
  ref_pic_lists_modification() {}

  void
  parse(SubByteReader &reader, const uint64_t NumPicTotalCurr, const slice_segment_header *slice);

  bool             ref_pic_list_modification_flag_l0{};
  vector<uint64_t> list_entry_l0;
  bool             ref_pic_list_modification_flag_l1{};
  vector<uint64_t> list_entry_l1;
};

} // namespace combiner::parser::hevc