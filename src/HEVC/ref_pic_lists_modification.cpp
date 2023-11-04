/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "ref_pic_lists_modification.h"

#include "slice_segment_header.h"

#include <cmath>

namespace combiner::parser::hevc
{

void ref_pic_lists_modification::parse(SubByteReader &             reader,
                                       const uint64_t              NumPicTotalCurr,
                                       const slice_segment_header *slice)
{
  const auto nrBits = static_cast<int>(std::ceil(std::log2(NumPicTotalCurr)));

  this->ref_pic_list_modification_flag_l0 = reader.readFlag();
  if (this->ref_pic_list_modification_flag_l0)
  {
    for (unsigned int i = 0; i <= slice->num_ref_idx_l0_active_minus1; i++)
      this->list_entry_l0.push_back(reader.readBits(nrBits));
  }

  if (slice->slice_type == SliceType::B)
  {
    this->ref_pic_list_modification_flag_l1 = reader.readFlag();
    if (ref_pic_list_modification_flag_l1)
      for (unsigned int i = 0; i <= slice->num_ref_idx_l1_active_minus1; i++)
        this->list_entry_l1.push_back(reader.readBits(nrBits));
  }
}

void ref_pic_lists_modification::write(SubByteWriter &             writer,
                                       const uint64_t              NumPicTotalCurr,
                                       const slice_segment_header *slice) const
{
  const auto nrBits = static_cast<int>(std::ceil(std::log2(NumPicTotalCurr)));

  writer.writeFlag(this->ref_pic_list_modification_flag_l0);
  if (this->ref_pic_list_modification_flag_l0)
  {
    for (unsigned int i = 0; i <= slice->num_ref_idx_l0_active_minus1; i++)
      writer.writeBits(this->list_entry_l0.at(i), nrBits);
  }

  if (slice->slice_type == SliceType::B)
  {
    writer.writeFlag(this->ref_pic_list_modification_flag_l1);
    if (ref_pic_list_modification_flag_l1)
      for (unsigned int i = 0; i <= slice->num_ref_idx_l1_active_minus1; i++)
        writer.writeBits(this->list_entry_l1.at(i), nrBits);
  }
}

} // namespace combiner::parser::hevc
