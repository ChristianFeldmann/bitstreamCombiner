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

} // namespace

using namespace parser::hevc;

Combiner::Combiner(std::vector<combiner::FileSourceAnnexB> &&inputFiles)
{
  for (auto &file : inputFiles)
    this->parsers.emplace_back(std::move(file));

  FileSinkAnnexB outputFile("debugOutputFile.hevc");

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
      auto vps = dynamic_cast<video_parameter_set_rbsp *>(firstNal.rbsp.get());

      parser::SubByteWriter writer;
      vps->write(writer);
      const auto data = writer.finishWritingAndGetData();
      this->fileSink.writeNALUnit(data);
    }
    else if (firstNalType == NalType::SPS_NUT)
    {
      auto sps = dynamic_cast<seq_parameter_set_rbsp *>(firstNal.rbsp.get());

      parser::SubByteWriter writer;
      sps->write(writer);
      const auto data = writer.finishWritingAndGetData();
      this->fileSink.writeNALUnit(data);
    }
    else if (firstNalType == NalType::PPS_NUT)
    {
      auto pps = dynamic_cast<pic_parameter_set_rbsp *>(firstNal.rbsp.get());

      parser::SubByteWriter writer;
      pps->write(writer);
      const auto data = writer.finishWritingAndGetData();
      this->fileSink.writeNALUnit(data);
    }
    else if (nalPerFile.at(0).header.isSlice())
    {
      auto slice = dynamic_cast<slice_segment_layer_rbsp *>(firstNal.rbsp.get());
      std::cout << "Combine POC " << slice->sliceSegmentHeader.PicOrderCntVal << "\n";

      parser::SubByteWriter writer;
      const auto &          activeParameterSets = this->parsers.at(0).getActiveParameterSets();
      slice->write(
          writer, nalPerFile.at(0).header, activeParameterSets.spsMap, activeParameterSets.ppsMap);
      const auto data = writer.finishWritingAndGetData();
      this->fileSink.writeNALUnit(data);
    }
    else
    {
      this->fileSink.writeNALUnit(firstNal.rawData);
    }
  }
}

} // namespace combiner
