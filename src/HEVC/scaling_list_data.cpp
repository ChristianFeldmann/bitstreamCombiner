/*  This file is part of YUView - The YUV player with advanced analytics toolset
 *   <https://github.com/IENT/YUView>
 *   Copyright (C) 2015  Institut f�r Nachrichtentechnik, RWTH Aachen University, GERMANY
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the
 *   OpenSSL library under certain conditions as described in each
 *   individual source file, and distribute linked combinations including
 *   the two.
 *
 *   You must obey the GNU General Public License in all respects for all
 *   of the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the
 *   file(s), but you are not obligated to do so. If you do not wish to do
 *   so, delete this exception statement from your version. If you delete
 *   this exception statement from all source files in the program, then
 *   also delete it here.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
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