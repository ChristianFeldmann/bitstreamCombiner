/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "Combiner.h"

#include <HEVC/NalUnitHEVC.h>
#include <HEVC/seq_parameter_set_rbsp.h>
#include <HEVC/slice_segment_layer_rbsp.h>
#include <HEVC/video_parameter_set_rbsp.h>
#include <common/SubByteWriter.h>

#include <iostream>

namespace combiner
{

using namespace parser::hevc;
using NalUnitVector = std::vector<NalUnitHEVC>;

namespace
{

bool anyNalUnitsEmpty(const NalUnitVector &nals)
{
  return std::any_of(
      nals.begin(), nals.end(), [](const NalUnitHEVC &nal) { return nal.rawData.empty(); });
}

seq_parameter_set_rbsp layoutTilesInSPS(const NalUnitVector &nalUnits)
{
  seq_parameter_set_rbsp sps;

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
  {
    const auto secondSPS = dynamic_cast<seq_parameter_set_rbsp *>(nalUnits.at(1).rbsp.get());

    if (firstSPS->pic_width_in_luma_samples % firstSPS->CtbSizeY != 0)
      throw std::runtime_error("In a side by side layout, the width of the first input must be a "
                               "multiple of CtbSizeY (max CTU size)");

    sps = *firstSPS;
    sps.pic_width_in_luma_samples =
        firstSPS->pic_width_in_luma_samples + secondSPS->pic_width_in_luma_samples;

    if (firstSPS->pic_height_in_luma_samples != secondSPS->pic_height_in_luma_samples)
      throw std::runtime_error(
          "The frame height must be identical to layout the tiles side by side.");
  }
  else if (nalUnits.size() == 4)
  {
    const auto secondSPS = dynamic_cast<seq_parameter_set_rbsp *>(nalUnits.at(1).rbsp.get());
    const auto thirdSPS  = dynamic_cast<seq_parameter_set_rbsp *>(nalUnits.at(2).rbsp.get());
    const auto fourthSPS = dynamic_cast<seq_parameter_set_rbsp *>(nalUnits.at(3).rbsp.get());

    if (firstSPS->pic_width_in_luma_samples % firstSPS->CtbSizeY != 0 ||
        firstSPS->pic_height_in_luma_samples % firstSPS->CtbSizeY != 0)
      throw std::runtime_error("In a 2x2 layout, the width and height of the first input must be a "
                               "multiple of CtbSizeY (max CTU size)");
    if (secondSPS->pic_height_in_luma_samples % firstSPS->CtbSizeY != 0)
      throw std::runtime_error("In a 2x2 layout, the height of the second input must be a "
                               "multiple of CtbSizeY (max CTU size)");
    if (thirdSPS->pic_width_in_luma_samples % firstSPS->CtbSizeY != 0)
      throw std::runtime_error("In a 2x2 layout, the width of the third input must be a "
                               "multiple of CtbSizeY (max CTU size)");

    sps = *firstSPS;
    sps.pic_width_in_luma_samples =
        firstSPS->pic_width_in_luma_samples + secondSPS->pic_width_in_luma_samples;

    if (thirdSPS->pic_width_in_luma_samples != firstSPS->pic_width_in_luma_samples ||
        fourthSPS->pic_width_in_luma_samples != secondSPS->pic_width_in_luma_samples)
      throw std::runtime_error("The frame widths of the 4 input files do not match in a way that "
                               "they can be laid out in a 2x2 grid.");

    sps.pic_height_in_luma_samples =
        firstSPS->pic_height_in_luma_samples + thirdSPS->pic_height_in_luma_samples;

    if (secondSPS->pic_height_in_luma_samples != firstSPS->pic_height_in_luma_samples ||
        fourthSPS->pic_height_in_luma_samples != thirdSPS->pic_height_in_luma_samples)
      throw std::runtime_error("The frame height of the 4 input files do not match in a way that "
                               "they can be laid out in a 2x2 grid.");
  }
  else
    throw std::runtime_error("Only layout of 2 (side by side) and 4 (grid) files implemented.");

  sps.updateCalculatedValues();

  return sps;
}

pic_parameter_set_rbsp enableUniformTiles(const NalUnitVector &nalUnits)
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

void checkSlices(const NalUnitVector &nalUnits)
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

} // namespace

using namespace parser::hevc;

Combiner::Combiner(std::vector<FileSourceAnnexB> &&inputFiles, FileSinkAnnexB &&outputFile)
    : outputFile(std::move(outputFile))
{
  for (auto &file : inputFiles)
    this->parsers.emplace_back(std::move(file));

  this->combineFiles();
}

void Combiner::combineFiles()
{
  while (true)
  {
    NalUnitVector nalPerFile;

    for (auto &parser : this->parsers)
      nalPerFile.push_back(parser.parseNextNalFromFile());

    if (anyNalUnitsEmpty(nalPerFile))
      return;

    const auto firstNalType = nalPerFile.at(0).header.nal_unit_type;
    for (const auto &nal : nalPerFile)
    {
      if (nal.header.nal_unit_type != firstNalType)
        throw std::runtime_error("Nal type of all files must be identical");
    }

    const auto &firstNal = nalPerFile.at(0);
    if (firstNalType == NalType::VPS_NUT)
    {
      const auto vps = dynamic_cast<video_parameter_set_rbsp *>(firstNal.rbsp.get());

      parser::SubByteWriter writer;
      firstNal.header.write(writer);
      vps->write(writer);
      const auto data = writer.finishWritingAndGetData();
      this->outputFile.writeNALUnit(data);

      this->activeWritingParameterSets.vpsMap[vps->vps_video_parameter_set_id] = *vps;

      std::cout << "Pass through VPS from file 0.\n";
    }
    else if (firstNalType == NalType::SPS_NUT)
    {
      const auto firstSPS = dynamic_cast<seq_parameter_set_rbsp *>(nalPerFile.at(0).rbsp.get());
      this->frameSize     = firstSPS->getFrameSize();

      const auto newSPS = layoutTilesInSPS(nalPerFile);

      parser::SubByteWriter writer;
      firstNal.header.write(writer);
      newSPS.write(writer);
      const auto data = writer.finishWritingAndGetData();
      this->outputFile.writeNALUnit(data);

      this->activeWritingParameterSets.spsMap[newSPS.sps_seq_parameter_set_id] = newSPS;
      this->CtbSizeY                                                           = newSPS.CtbSizeY;

      std::cout << "SPS -> New frame size " << newSPS.getFrameSize().toString() << "\n";
    }
    else if (firstNalType == NalType::PPS_NUT)
    {
      const auto newPPS = enableUniformTiles(nalPerFile);

      parser::SubByteWriter writer;
      firstNal.header.write(writer);
      newPPS.write(writer);
      const auto data = writer.finishWritingAndGetData();
      this->outputFile.writeNALUnit(data);

      this->activeWritingParameterSets.ppsMap[newPPS.pps_pic_parameter_set_id] = newPPS;

      std::cout << "PPS -> Enabled tiles\n";
    }
    else if (nalPerFile.at(0).header.isSlice())
    {
      checkSlices(nalPerFile);
      this->writeOutSlices(nalPerFile);

      const auto firstSlice = dynamic_cast<slice_segment_layer_rbsp *>(nalPerFile.at(0).rbsp.get());
      std::cout << "Combined POC " << firstSlice->sliceSegmentHeader.PicOrderCntVal << "\n";
    }
    else
    {
      this->outputFile.writeNALUnit(firstNal.rawData);
      std::cout << "Pass through " << NalTypeMapper.getName(firstNalType) << " NAL.\n";
    }
  }
}

void Combiner::writeOutSlices(const NalUnitVector &nalUnits)
{
  if (this->CtbSizeY != 16 && this->CtbSizeY != 32 && this->CtbSizeY != 64)
    throw std::logic_error("Invalid CtbSizeY of " + std::to_string(this->CtbSizeY));

  for (size_t i = 0; i < nalUnits.size(); ++i)
  {
    const auto &nal         = nalUnits.at(i);
    const auto  slice       = dynamic_cast<slice_segment_layer_rbsp *>(nal.rbsp.get());
    auto        sliceHeader = slice->sliceSegmentHeader;

    sliceHeader.first_slice_segment_in_pic_flag = (i == 0);
    const auto widthInCTU                       = (this->frameSize.width / this->CtbSizeY);
    const auto heightInCTU                      = (this->frameSize.height / this->CtbSizeY);
    switch (i)
    {
    case 0:
      sliceHeader.slice_segment_address = 0;
      break;
    case 1:
      sliceHeader.slice_segment_address = widthInCTU;
      break;
    case 2:
      sliceHeader.slice_segment_address = widthInCTU * 2 * heightInCTU;
      break;
    case 3:
      sliceHeader.slice_segment_address = widthInCTU * 2 * heightInCTU + widthInCTU;

    default:
      break;
    }

    parser::SubByteWriter writer;
    nal.header.write(writer);
    sliceHeader.write(writer, nal.header, this->activeWritingParameterSets);
    auto data = writer.finishWritingAndGetData();

    data.insert(data.end(),
                nal.rawData.begin() + slice->sliceSegmentHeader.nrBytesInHeader,
                nal.rawData.end());

    this->outputFile.writeNALUnit(data);
  }
}

} // namespace combiner
