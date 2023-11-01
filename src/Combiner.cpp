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

Combiner::Combiner(std::vector<combiner::FileSourceAnnexB> &&inputFiles)
{
  for (auto &file : inputFiles)
    this->parsers.emplace_back(std::move(file));

  FileSinkAnnexB outputFile("debugOutputFile.hevc");
  this->testPassThroughOfBitstream(outputFile);

  // this->parseHeadersFromFiles();
  // this->combineFiles();
}

void Combiner::parseHeadersFromFiles()
{
  std::cout << "Getting headers from files...\n";

  for (auto &parser : this->parsers)
    parser.parseHeaders();
}

void Combiner::combineFiles()
{
  while (true)
  {
    std::vector<std::shared_ptr<parser::hevc::NalUnitHEVC>> slicePerFile;

    for (auto &parser : this->parsers)
    {
      const auto slice = parser.getNextSlice();
      if (!slice)
        return;
      slicePerFile.push_back(std::move(slice));
    }

    if (slicePerFile.size() != this->parsers.size())
      return;

    auto firstFileSlice =
        dynamic_cast<parser::hevc::slice_segment_layer_rbsp *>(slicePerFile.at(0)->rbsp.get());

    if (!firstFileSlice)
      throw std::runtime_error("Unable to cast slice");

    std::cout << "Combine POC " << firstFileSlice->sliceSegmentHeader.PicOrderCntVal << "\n";
  }
}

void Combiner::testPassThroughOfBitstream(FileSinkAnnexB &outputFile)
{
  auto &parser = this->parsers.at(0);
  while (const auto &nal = parser.parseNextNalFromFile())
  {
    if (nal->header.nal_unit_type == NalType::VPS_NUT)
    {
      auto vps = dynamic_cast<parser::hevc::video_parameter_set_rbsp *>(nal->rbsp.get());

      parser::SubByteWriter writer;
      nal->header.write(writer);
      vps->write(writer);
      const auto data = writer.finishWritingAndGetData();

      outputFile.writeNALUnit(data);
    }
    else if (nal->header.nal_unit_type == NalType::SPS_NUT)
    {
      auto sps = dynamic_cast<parser::hevc::seq_parameter_set_rbsp *>(nal->rbsp.get());

      parser::SubByteWriter writer;
      nal->header.write(writer);
      sps->write(writer);
      const auto data = writer.finishWritingAndGetData();

      outputFile.writeNALUnit(data);
    }
    else
      outputFile.writeNALUnit(nal->rawData);
  }
}

} // namespace combiner
