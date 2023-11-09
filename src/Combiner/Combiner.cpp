/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "Combiner.h"

#include <HEVC/NalUnitHEVC.h>
#include <common/SubByteWriter.h>

#include "ParameterSetsModifiers.h"

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

uint64_t roundToCTUSize(uint64_t size, uint64_t CtbSizeY)
{
  return (size + CtbSizeY - 1) / CtbSizeY;
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
      for (int i = 0; i < nalPerFile.size(); ++i)
      {
        const auto sps = dynamic_cast<seq_parameter_set_rbsp *>(nalPerFile.at(i).rbsp.get());
        this->frameSizePerInput[i] = sps->getFrameSize();
      }

      const auto newSPS = generateSPSWithNewFrameSize(nalPerFile);

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
      const auto newPPS = generatePPSWithUniformTiles(nalPerFile);

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
      checkForMathingSlices(nalPerFile);
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

    sliceHeader.slice_segment_address = 0;
    const auto isOneOfRightInputs     = (i == 1 || i == 3);
    if (isOneOfRightInputs)
    {
      const auto leftInputWidthInCTU =
          roundToCTUSize(this->frameSizePerInput[0].width, this->CtbSizeY);
      sliceHeader.slice_segment_address += leftInputWidthInCTU;
    }
    const auto isOneOfLowerInputs = (i == 2 || i == 3);
    if (isOneOfLowerInputs)
    {
      const auto fullWidth = this->frameSizePerInput[0].width + this->frameSizePerInput[1].width;
      const auto fullWidthInCTU = roundToCTUSize(fullWidth, this->CtbSizeY);
      const auto upperRowHeightInCTU =
          roundToCTUSize(this->frameSizePerInput[0].height, this->CtbSizeY);

      const auto offsetToThirdInputInCTU = fullWidthInCTU * upperRowHeightInCTU;
      sliceHeader.slice_segment_address += offsetToThirdInputInCTU;
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
