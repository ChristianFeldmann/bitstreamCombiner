/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "sub_layer_hrd_parameters.h"

#include <stdexcept>

namespace combiner::parser::hevc
{

void sub_layer_hrd_parameters::parse(SubByteReader &reader,
                                     const uint64_t CpbCnt,
                                     const bool     sub_pic_hrd_params_present_flag,
                                     const bool     SubPicHrdFlag,
                                     const uint64_t bit_rate_scale,
                                     const uint64_t cpb_size_scale,
                                     const uint64_t cpb_size_du_scale)
{
  if (CpbCnt >= 32)
    throw std::logic_error("The value of CpbCnt must be in the range of 0 to 31");

  for (int i = 0; i <= CpbCnt; i++)
  {
    this->bit_rate_value_minus1.push_back(reader.readUEV());
    if (i > 0 && this->bit_rate_value_minus1[i] <= this->bit_rate_value_minus1[i - 1])
      throw std::logic_error("For any i > 0, bit_rate_value_minus1[i] shall be greater than "
                             "bit_rate_value_minus1[i-1].");
    if (!SubPicHrdFlag)
    {
      const auto value =
          (this->bit_rate_value_minus1[i] + 1) * (uint64_t(1) << (6 + bit_rate_scale));
      this->BitRate.push_back(value);
    }

    this->cpb_size_value_minus1.push_back(reader.readUEV());
    if (i > 0 && this->cpb_size_value_minus1[i] > this->cpb_size_value_minus1[i - 1])
      throw std::logic_error("For any i greater than 0, cpb_size_value_minus1[i] shall be less "
                             "than or equal to cpb_size_value_minus1[i-1].");
    if (!SubPicHrdFlag)
    {
      const auto value =
          (this->cpb_size_value_minus1[i] + 1) * (uint64_t(1) << (4 + cpb_size_scale));
      this->CpbSize.push_back(value);
    }

    if (sub_pic_hrd_params_present_flag)
    {
      this->cpb_size_du_value_minus1.push_back(reader.readUEV());
      if (i > 0 && cpb_size_du_value_minus1[i] > cpb_size_du_value_minus1[i - 1])
        throw std::logic_error("For any i greater than 0, cpb_size_du_value_minus1[i] shall be "
                               "less than or equal to cpb_size_du_value_minus1[i-1].");
      if (SubPicHrdFlag)
      {
        const auto value =
            (this->cpb_size_du_value_minus1[i] + 1) * (uint64_t(1) << (4 + cpb_size_du_scale));
        this->CpbSize.push_back(value);
      }

      this->bit_rate_du_value_minus1.push_back(reader.readUEV());
      if (i > 0 && bit_rate_du_value_minus1[i] <= bit_rate_du_value_minus1[i - 1])
        throw std::logic_error("For any i > 0, bit_rate_du_value_minus1[i] shall be greater than "
                               "bit_rate_du_value_minus1[i-1].");
      if (SubPicHrdFlag)
      {
        const auto value =
            (this->bit_rate_du_value_minus1[i] + 1) * (uint64_t(1) << (6 + bit_rate_scale));
        this->BitRate.push_back(value);
      }
    }
    this->cbr_flag.push_back(reader.readFlag());
  }
}

} // namespace combiner::parser::hevc