/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "scaling_list_data.h"

namespace combiner::parser::hevc
{

void scaling_list_data::parse(SubByteReader &reader)
{
  for (unsigned sizeId = 0; sizeId < 4; sizeId++)
  {
    for (unsigned matrixId = 0; matrixId < 6u; matrixId += (sizeId == 3) ? 3 : 1)
    {

      this->scaling_list_pred_mode_flag[sizeId][matrixId] = reader.readFlag();
      if (!this->scaling_list_pred_mode_flag[sizeId][matrixId])
      {
        this->scaling_list_pred_matrix_id_delta[sizeId][matrixId] = reader.readUEV();
      }
      else
      {
        int64_t nextCoef = 8;
        auto    coefNum  = std::min(64u, (1u << (4u + (sizeId << 1u))));
        if (sizeId > 1)
        {
          this->scaling_list_dc_coef_minus8[sizeId - 2][matrixId] = reader.readSEV();
          nextCoef = this->scaling_list_dc_coef_minus8[sizeId - 2][matrixId] + 8;
        }
        for (unsigned i = 0; i < coefNum; i++)
        {
          this->scaling_list_delta_coef[sizeId][matrixId] = reader.readSEV();
          nextCoef = (nextCoef + this->scaling_list_delta_coef[sizeId][matrixId] + 256) % 256;
        }
      }
    }
  }
}

void scaling_list_data::write(SubByteWriter &writer) const
{
  for (int sizeId = 0; sizeId < 4; sizeId++)
  {
    for (int matrixId = 0; matrixId < 6; matrixId += (sizeId == 3) ? 3 : 1)
    {
      writer.writeFlag(this->scaling_list_pred_mode_flag[sizeId][matrixId]);
      if (!this->scaling_list_pred_mode_flag[sizeId][matrixId])
      {
        writer.writeUEV(this->scaling_list_pred_matrix_id_delta[sizeId][matrixId]);
      }
      else
      {
        auto coefNum = std::min(64u, (1u << (4u + (sizeId << 1u))));
        if (sizeId > 1)
        {
          writer.writeSEV(this->scaling_list_dc_coef_minus8[sizeId - 2][matrixId]);
        }
        for (unsigned i = 0; i < coefNum; i++)
        {
          writer.writeSEV(this->scaling_list_delta_coef[sizeId][matrixId]);
        }
      }
    }
  }
}

} // namespace combiner::parser::hevc