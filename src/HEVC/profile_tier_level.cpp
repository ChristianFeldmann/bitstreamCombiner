/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "profile_tier_level.h"

#include <stdexcept>

namespace combiner::parser::hevc
{

void profile_tier_level::parse(SubByteReader &reader,
                               const bool     profilePresentFlag,
                               const uint64_t maxNumSubLayersMinus1)
{
  if (profilePresentFlag)
  {
    this->general_profile_space = reader.readBits(2);
    this->general_tier_flag     = reader.readFlag();
    this->general_profile_idc   = reader.readBits(5);

    for (unsigned j = 0; j < 32; j++)
      this->general_profile_compatibility_flag[j] = reader.readFlag();
    this->general_progressive_source_flag    = reader.readFlag();
    this->general_interlaced_source_flag     = reader.readFlag();
    this->general_non_packed_constraint_flag = reader.readFlag();
    this->general_frame_only_constraint_flag = reader.readFlag();

    if (this->general_profile_idc == 4 || this->general_profile_compatibility_flag[4] ||
        this->general_profile_idc == 5 || this->general_profile_compatibility_flag[5] ||
        this->general_profile_idc == 6 || this->general_profile_compatibility_flag[6] ||
        this->general_profile_idc == 7 || this->general_profile_compatibility_flag[7])
    {
      this->general_max_12bit_constraint_flag        = reader.readFlag();
      this->general_max_10bit_constraint_flag        = reader.readFlag();
      this->general_max_8bit_constraint_flag         = reader.readFlag();
      this->general_max_422chroma_constraint_flag    = reader.readFlag();
      this->general_max_420chroma_constraint_flag    = reader.readFlag();
      this->general_max_monochrome_constraint_flag   = reader.readFlag();
      this->general_intra_constraint_flag            = reader.readFlag();
      this->general_one_picture_only_constraint_flag = reader.readFlag();
      this->general_lower_bit_rate_constraint_flag   = reader.readFlag();

      const auto general_reserved_zero_bits = reader.readBits(34);
      if (general_reserved_zero_bits)
        throw std::logic_error("general_reserved_zero_bits must be zero");
    }
    else
    {
      const auto general_reserved_zero_bits = reader.readBits(43);
      if (general_reserved_zero_bits)
        throw std::logic_error("general_reserved_zero_bits must be zero");
    }

    if ((this->general_profile_idc >= 1 && this->general_profile_idc <= 5) ||
        this->general_profile_compatibility_flag[1] ||
        this->general_profile_compatibility_flag[2] ||
        this->general_profile_compatibility_flag[3] ||
        this->general_profile_compatibility_flag[4] || this->general_profile_compatibility_flag[5])
      this->general_inbld_flag = reader.readFlag();
    else
    {
      const auto general_reserved_zero_bit = reader.readFlag();
      if (general_reserved_zero_bit)
        throw std::logic_error("general_reserved_zero_bit must be zero");
    }
  }

  this->general_level_idc = reader.readBits(8);

  for (unsigned i = 0; i < maxNumSubLayersMinus1; i++)
  {
    this->sub_layer_profile_present_flag[i] = reader.readFlag();
    this->sub_layer_level_present_flag[i]   = reader.readFlag();
  }

  if (maxNumSubLayersMinus1 > 0)
    for (auto i = maxNumSubLayersMinus1; i < 8; i++)
    {
      const auto reserved_zero_2bits = reader.readBits(2);
      if (reserved_zero_2bits)
        throw std::logic_error("reserved_zero_2bits must be zero");
    }

  for (unsigned i = 0; i < maxNumSubLayersMinus1; i++)
  {
    if (this->sub_layer_profile_present_flag[i])
    {
      this->sub_layer_profile_space[i] = reader.readBits(2);
      this->sub_layer_tier_flag[i]     = reader.readFlag();

      this->sub_layer_profile_idc[i] = reader.readBits(5);

      for (unsigned j = 0; j < 32; j++)
        this->sub_layer_profile_compatibility_flag[i][j] = reader.readFlag();

      this->sub_layer_progressive_source_flag[i]    = reader.readFlag();
      this->sub_layer_interlaced_source_flag[i]     = reader.readFlag();
      this->sub_layer_non_packed_constraint_flag[i] = reader.readFlag();
      this->sub_layer_frame_only_constraint_flag[i] = reader.readFlag();

      if (sub_layer_profile_idc[i] == 4 || sub_layer_profile_compatibility_flag[i][4] ||
          sub_layer_profile_idc[i] == 5 || sub_layer_profile_compatibility_flag[i][5] ||
          sub_layer_profile_idc[i] == 6 || sub_layer_profile_compatibility_flag[i][6] ||
          sub_layer_profile_idc[i] == 7 || sub_layer_profile_compatibility_flag[i][7])
      {
        this->sub_layer_max_12bit_constraint_flag[i]        = reader.readFlag();
        this->sub_layer_max_10bit_constraint_flag[i]        = reader.readFlag();
        this->sub_layer_max_8bit_constraint_flag[i]         = reader.readFlag();
        this->sub_layer_max_422chroma_constraint_flag[i]    = reader.readFlag();
        this->sub_layer_max_420chroma_constraint_flag[i]    = reader.readFlag();
        this->sub_layer_max_monochrome_constraint_flag[i]   = reader.readFlag();
        this->sub_layer_intra_constraint_flag[i]            = reader.readFlag();
        this->sub_layer_one_picture_only_constraint_flag[i] = reader.readFlag();
        this->sub_layer_lower_bit_rate_constraint_flag[i]   = reader.readFlag();

        const auto sub_layer_reserved_zero_bits = reader.readBits(34);
        if (sub_layer_reserved_zero_bits != 0)
          throw std::logic_error("sub_layer_reserved_zero_bits must all be zero");
      }
      else
      {
        const auto sub_layer_reserved_zero_bits = reader.readBits(43);
        if (sub_layer_reserved_zero_bits != 0)
          throw std::logic_error("sub_layer_reserved_zero_bits must be zero");
      }

      if ((this->sub_layer_profile_idc[i] >= 1 && this->sub_layer_profile_idc[i] <= 5) ||
          sub_layer_profile_compatibility_flag[i][1] ||
          sub_layer_profile_compatibility_flag[i][2] ||
          sub_layer_profile_compatibility_flag[i][3] ||
          sub_layer_profile_compatibility_flag[i][4] || sub_layer_profile_compatibility_flag[i][5])
        this->sub_layer_inbld_flag[i] = reader.readFlag();
      else
      {
        const auto sub_layer_reserved_zero_bit = reader.readFlag();
        if (sub_layer_reserved_zero_bit)
          throw std::logic_error("sub_layer_reserved_zero_bit must be zero");
      }
    }

    if (this->sub_layer_level_present_flag[i])
      this->sub_layer_level_idc[i] = reader.readBits(8);
  }
}

void profile_tier_level::write(SubByteWriter &writer,
                               const bool     profilePresentFlag,
                               const uint64_t maxNumSubLayersMinus1) const
{
  if (profilePresentFlag)
  {
    writer.writeBits(this->general_profile_space, 2);
    writer.writeFlag(this->general_tier_flag);
    writer.writeBits(this->general_profile_idc, 5);

    for (auto j = 0; j < 32; j++)
      writer.writeFlag(this->general_profile_compatibility_flag[j]);

    writer.writeFlag(this->general_progressive_source_flag);
    writer.writeFlag(this->general_interlaced_source_flag);
    writer.writeFlag(this->general_non_packed_constraint_flag);
    writer.writeFlag(this->general_frame_only_constraint_flag);

    if (this->general_profile_idc == 4 || this->general_profile_compatibility_flag[4] ||
        this->general_profile_idc == 5 || this->general_profile_compatibility_flag[5] ||
        this->general_profile_idc == 6 || this->general_profile_compatibility_flag[6] ||
        this->general_profile_idc == 7 || this->general_profile_compatibility_flag[7])
    {
      writer.writeFlag(this->general_max_12bit_constraint_flag);
      writer.writeFlag(this->general_max_10bit_constraint_flag);
      writer.writeFlag(this->general_max_8bit_constraint_flag);
      writer.writeFlag(this->general_max_422chroma_constraint_flag);
      writer.writeFlag(this->general_max_420chroma_constraint_flag);
      writer.writeFlag(this->general_max_monochrome_constraint_flag);
      writer.writeFlag(this->general_intra_constraint_flag);
      writer.writeFlag(this->general_one_picture_only_constraint_flag);
      writer.writeFlag(this->general_lower_bit_rate_constraint_flag);

      const auto general_reserved_zero_bits = 0;
      writer.writeBits(general_reserved_zero_bits, 34);
    }
    else
    {
      const auto general_reserved_zero_bits = 0;
      writer.writeBits(general_reserved_zero_bits, 43);
    }

    if ((this->general_profile_idc >= 1 && this->general_profile_idc <= 5) ||
        this->general_profile_compatibility_flag[1] ||
        this->general_profile_compatibility_flag[2] ||
        this->general_profile_compatibility_flag[3] ||
        this->general_profile_compatibility_flag[4] || this->general_profile_compatibility_flag[5])
      writer.writeFlag(this->general_inbld_flag);
    else
    {
      const auto general_reserved_zero_bit = false;
      writer.writeFlag(general_reserved_zero_bit);
    }
  }

  writer.writeBits(this->general_level_idc, 8);

  for (unsigned i = 0; i < maxNumSubLayersMinus1; i++)
  {
    writer.writeFlag(this->sub_layer_profile_present_flag[i]);
    writer.writeFlag(this->sub_layer_level_present_flag[i]);
  }

  if (maxNumSubLayersMinus1 > 0)
    for (auto i = maxNumSubLayersMinus1; i < 8; i++)
    {
      const auto reserved_zero_2bits = 0;
      writer.writeBits(reserved_zero_2bits, 2);
    }

  for (unsigned i = 0; i < maxNumSubLayersMinus1; i++)
  {
    if (this->sub_layer_profile_present_flag[i])
    {
      writer.writeBits(this->sub_layer_profile_space[i], 2);
      writer.writeFlag(this->sub_layer_tier_flag[i]);

      writer.writeBits(this->sub_layer_profile_idc[i], 5);

      for (unsigned j = 0; j < 32; j++)
        writer.writeFlag(this->sub_layer_profile_compatibility_flag[i][j]);

      writer.writeFlag(this->sub_layer_progressive_source_flag[i]);
      writer.writeFlag(this->sub_layer_interlaced_source_flag[i]);
      writer.writeFlag(this->sub_layer_non_packed_constraint_flag[i]);
      writer.writeFlag(this->sub_layer_frame_only_constraint_flag[i]);

      if (sub_layer_profile_idc[i] == 4 || sub_layer_profile_compatibility_flag[i][4] ||
          sub_layer_profile_idc[i] == 5 || sub_layer_profile_compatibility_flag[i][5] ||
          sub_layer_profile_idc[i] == 6 || sub_layer_profile_compatibility_flag[i][6] ||
          sub_layer_profile_idc[i] == 7 || sub_layer_profile_compatibility_flag[i][7])
      {
        writer.writeFlag(this->sub_layer_max_12bit_constraint_flag[i]);
        writer.writeFlag(this->sub_layer_max_10bit_constraint_flag[i]);
        writer.writeFlag(this->sub_layer_max_8bit_constraint_flag[i]);
        writer.writeFlag(this->sub_layer_max_422chroma_constraint_flag[i]);
        writer.writeFlag(this->sub_layer_max_420chroma_constraint_flag[i]);
        writer.writeFlag(this->sub_layer_max_monochrome_constraint_flag[i]);
        writer.writeFlag(this->sub_layer_intra_constraint_flag[i]);
        writer.writeFlag(this->sub_layer_one_picture_only_constraint_flag[i]);
        writer.writeFlag(this->sub_layer_lower_bit_rate_constraint_flag[i]);

        const auto sub_layer_reserved_zero_bits = 0;
        writer.writeBits(sub_layer_reserved_zero_bits, 34);
      }
      else
      {
        const auto sub_layer_reserved_zero_bits = 0;
        writer.writeBits(sub_layer_reserved_zero_bits, 43);
      }

      if ((this->sub_layer_profile_idc[i] >= 1 && this->sub_layer_profile_idc[i] <= 5) ||
          sub_layer_profile_compatibility_flag[i][1] ||
          sub_layer_profile_compatibility_flag[i][2] ||
          sub_layer_profile_compatibility_flag[i][3] ||
          sub_layer_profile_compatibility_flag[i][4] || sub_layer_profile_compatibility_flag[i][5])
        writer.writeFlag(this->sub_layer_inbld_flag[i]);
      else
      {
        const auto sub_layer_reserved_zero_bit = false;
        writer.writeFlag(sub_layer_reserved_zero_bit);
      }
    }

    if (this->sub_layer_level_present_flag[i])
      writer.writeBits(this->sub_layer_level_idc[i], 8);
  }
}

} // namespace combiner::parser::hevc
