/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "ParameterSetsModifiers.h"

namespace combiner
{

using namespace parser::hevc;

template <size_t N>
std::array<seq_parameter_set_rbsp *, N> getParameterSetArrayFromNals(const NalUnitVector &nalUnits)
{
  std::array<seq_parameter_set_rbsp *, N> parameterSets;
  for (int i = 0; i < N; ++i)
    parameterSets[i] = dynamic_cast<seq_parameter_set_rbsp *>(nalUnits.at(i).rbsp.get());
  return parameterSets;
}

seq_parameter_set_rbsp
generateSPSFor2InputsSideBySide(const std::array<seq_parameter_set_rbsp *, 2> spsArray)
{
  if (spsArray[0]->pic_width_in_luma_samples % spsArray[0]->CtbSizeY != 0)
    throw std::runtime_error("In a side by side layout, the width of the first input must be a "
                             "multiple of CtbSizeY (max CTU size)");

  auto sps = *spsArray[0];
  sps.pic_width_in_luma_samples =
      spsArray[0]->pic_width_in_luma_samples + spsArray[1]->pic_width_in_luma_samples;

  if (spsArray[0]->pic_height_in_luma_samples != spsArray[1]->pic_height_in_luma_samples)
    throw std::runtime_error(
        "The frame height must be identical to layout the tiles side by side.");

  sps.updateCalculatedValues();
  return sps;
}

void checkParameterSetsFor2x2Layout(const std::array<seq_parameter_set_rbsp *, 4> &parameterSets)
{
  if (parameterSets[0]->pic_width_in_luma_samples % parameterSets[0]->CtbSizeY != 0 ||
      parameterSets[0]->pic_height_in_luma_samples % parameterSets[0]->CtbSizeY != 0)
    throw std::runtime_error("In a 2x2 layout, the width and height of the first input must be a "
                             "multiple of CtbSizeY (max CTU size)");
  if (parameterSets[1]->pic_height_in_luma_samples % parameterSets[0]->CtbSizeY != 0)
    throw std::runtime_error("In a 2x2 layout, the height of the second input must be a "
                             "multiple of CtbSizeY (max CTU size)");
  if (parameterSets[2]->pic_width_in_luma_samples % parameterSets[0]->CtbSizeY != 0)
    throw std::runtime_error("In a 2x2 layout, the width of the third input must be a "
                             "multiple of CtbSizeY (max CTU size)");

  if (parameterSets[0]->pic_width_in_luma_samples != parameterSets[2]->pic_width_in_luma_samples ||
      parameterSets[1]->pic_width_in_luma_samples != parameterSets[3]->pic_width_in_luma_samples)
    throw std::runtime_error("The frame widths of the 4 input files do not match in a way that "
                             "they can be laid out in a 2x2 grid.");

  if (parameterSets[0]->pic_height_in_luma_samples !=
          parameterSets[1]->pic_height_in_luma_samples ||
      parameterSets[2]->pic_height_in_luma_samples != parameterSets[3]->pic_height_in_luma_samples)
    throw std::runtime_error("The frame height of the 4 input files do not match in a way that "
                             "they can be laid out in a 2x2 grid.");
}

seq_parameter_set_rbsp
generateSPSFor4InputsIn2x2Layout(const std::array<seq_parameter_set_rbsp *, 4> &parameterSets)
{
  auto sps = *parameterSets[0];

  sps.pic_width_in_luma_samples =
      parameterSets[0]->pic_width_in_luma_samples + parameterSets[1]->pic_width_in_luma_samples;
  sps.pic_height_in_luma_samples =
      parameterSets[0]->pic_height_in_luma_samples + parameterSets[2]->pic_height_in_luma_samples;

  sps.updateCalculatedValues();
  return sps;
}

seq_parameter_set_rbsp generateSPSWithNewFrameSize(const NalUnitVector &nalUnits)
{
  const auto firstSPS = dynamic_cast<seq_parameter_set_rbsp *>(nalUnits.at(0).rbsp.get());
  for (const auto &nal : nalUnits)
  {
    const auto sps = dynamic_cast<seq_parameter_set_rbsp *>(nal.rbsp.get());
    if (sps->CtbSizeY != firstSPS->CtbSizeY)
      throw std::runtime_error("The CtbSizeY (max CTU size) must be identical for all inputs");
  }

  if (nalUnits.size() == 1)
    return *firstSPS;
  else if (nalUnits.size() == 2)
    return generateSPSFor2InputsSideBySide(getParameterSetArrayFromNals<2>(nalUnits));
  else if (nalUnits.size() == 4)
  {
    const auto parameterSets = getParameterSetArrayFromNals<4>(nalUnits);
    checkParameterSetsFor2x2Layout(parameterSets);
    return generateSPSFor4InputsIn2x2Layout(parameterSets);
  }

  throw std::runtime_error("Only layout of 2 (side by side) and 4 (grid) files implemented.");
}

pic_parameter_set_rbsp generatePPSWithUniformTiles(const NalUnitVector &nalUnits)
{
  const auto firstPPS = dynamic_cast<pic_parameter_set_rbsp *>(nalUnits.at(0).rbsp.get());

  for (const auto &nal : nalUnits)
  {
    const auto pps = dynamic_cast<pic_parameter_set_rbsp *>(nal.rbsp.get());
    if (pps->tiles_enabled_flag)
      throw std::runtime_error("Tiles already enabled in PPS. This is not allowed.");
  }

  if (nalUnits.size() == 1)
    return *firstPPS;

  pic_parameter_set_rbsp pps = *firstPPS;

  pps.tiles_enabled_flag      = true;
  pps.num_tile_columns_minus1 = 1;
  if (nalUnits.size() == 4)
    pps.num_tile_rows_minus1 = 1;
  pps.uniform_spacing_flag = true;

  return pps;
}

void checkForMathingSlices(const NalUnitVector &nalUnits)
{
  const auto firstSlice = dynamic_cast<slice_segment_layer_rbsp *>(nalUnits.at(0).rbsp.get());

  int fileIndex = 0;
  for (const auto &nal : nalUnits)
  {
    const auto slice = dynamic_cast<slice_segment_layer_rbsp *>(nal.rbsp.get());
    if (slice->sliceSegmentHeader.slice_type != firstSlice->sliceSegmentHeader.slice_type)
      throw std::runtime_error("All slices must have the same slice_type.");
    if (slice->sliceSegmentHeader.PicOrderCntVal != firstSlice->sliceSegmentHeader.PicOrderCntVal)
      throw std::runtime_error(
          "All slices must have the same PicOrderCntVal. First input file POC " +
          std::to_string(firstSlice->sliceSegmentHeader.PicOrderCntVal) + " File " +
          std::to_string(fileIndex) + " POC " +
          std::to_string(slice->sliceSegmentHeader.PicOrderCntVal));
  }
}

} // namespace combiner