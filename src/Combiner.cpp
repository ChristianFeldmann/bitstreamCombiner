/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "Combiner.h"

#include <HEVC/NalUnitHEVC.h>
#include <HEVC/slice_segment_layer_rbsp.h>
#include <common/SubByteReader.h>

#include <iostream>

namespace combiner
{

using namespace parser::hevc;

Combiner::Combiner(std::vector<combiner::FileSourceAnnexB> &&inputFiles)
{
  for (auto &file : inputFiles)
    this->parsers.emplace_back(std::move(file));

  this->parseHeadersFromFiles();
  this->combineFiles();
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

} // namespace combiner
