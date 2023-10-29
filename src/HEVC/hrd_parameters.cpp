/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "hrd_parameters.h"

#include <stdexcept>

namespace combiner::parser::hevc
{

void hrd_parameters::parse(SubByteReader &reader,
                           const bool     commonInfPresentFlag,
                           const uint64_t maxNumSubLayersMinus1)
{
  if (commonInfPresentFlag)
  {
    this->nal_hrd_parameters_present_flag = reader.readFlag();
    this->vcl_hrd_parameters_present_flag = reader.readFlag();

    if (this->nal_hrd_parameters_present_flag || this->vcl_hrd_parameters_present_flag)
    {
      this->sub_pic_hrd_params_present_flag = reader.readFlag();
      if (this->sub_pic_hrd_params_present_flag)
      {
        this->tick_divisor_minus2                          = reader.readBits(8);
        this->du_cpb_removal_delay_increment_length_minus1 = reader.readBits(5);
        this->sub_pic_cpb_params_in_pic_timing_sei_flag    = reader.readFlag();
        this->dpb_output_delay_du_length_minus1            = reader.readBits(5);
      }
      this->bit_rate_scale = reader.readBits(4);
      this->cpb_size_scale = reader.readBits(4);
      if (this->sub_pic_hrd_params_present_flag)
        this->cpb_size_du_scale = reader.readBits(4);
      this->initial_cpb_removal_delay_length_minus1 = reader.readBits(5);
      this->au_cpb_removal_delay_length_minus1      = reader.readBits(5);
      this->dpb_output_delay_length_minus1          = reader.readBits(5);
    }
  }

  this->SubPicHrdFlag = (this->SubPicHrdPreferredFlag && this->sub_pic_hrd_params_present_flag);

  if (maxNumSubLayersMinus1 >= 8)
    throw std::logic_error("The value of maxNumSubLayersMinus1 must be in the range of 0 to 7");

  for (unsigned i = 0; i <= maxNumSubLayersMinus1; i++)
  {
    this->fixed_pic_rate_general_flag[i] = reader.readFlag();
    if (!this->fixed_pic_rate_general_flag[i])
      this->fixed_pic_rate_within_cvs_flag[i] = reader.readFlag();
    else
      this->fixed_pic_rate_within_cvs_flag[i] = 1;
    if (this->fixed_pic_rate_within_cvs_flag[i])
    {
      this->elemental_duration_in_tc_minus1[i] = reader.readUEV();
      this->low_delay_hrd_flag[i]              = false;
    }
    else
      this->low_delay_hrd_flag[i] = reader.readFlag();
    this->cpb_cnt_minus1[i] = 0;
    if (!this->low_delay_hrd_flag[i])
      this->cpb_cnt_minus1[i] = reader.readUEV();

    if (this->nal_hrd_parameters_present_flag)
      this->nal_sub_hrd[i].parse(reader,
                                 this->cpb_cnt_minus1[i],
                                 this->sub_pic_hrd_params_present_flag,
                                 SubPicHrdFlag,
                                 this->bit_rate_scale,
                                 this->cpb_size_scale,
                                 this->cpb_size_du_scale);
    if (this->vcl_hrd_parameters_present_flag)
      vcl_sub_hrd[i].parse(reader,
                           this->cpb_cnt_minus1[i],
                           this->sub_pic_hrd_params_present_flag,
                           SubPicHrdFlag,
                           this->bit_rate_scale,
                           this->cpb_size_scale,
                           this->cpb_size_du_scale);
  }
}

} // namespace combiner::parser::hevc