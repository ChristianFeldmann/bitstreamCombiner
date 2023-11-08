/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <FileSinkAnnexB.h>
#include <FileSourceAnnexB.h>
#include <HEVC/NalUnitHEVC.h>
#include <HEVC/ParserAnnexBHEVC.h>
#include <HEVC/commonMaps.h>

#include <vector>

namespace combiner
{

class Combiner
{
public:
  Combiner(std::vector<FileSourceAnnexB> &&inputFiles, FileSinkAnnexB &&outputFile);

private:
  void combineFiles();
  void writeOutSlices(const std::vector<parser::hevc::NalUnitHEVC> &nalUnits);

  std::map<int, parser::hevc::NalUnitHEVC> vpsPerFile;
  std::map<int, parser::hevc::NalUnitHEVC> spsPerFile;
  std::map<int, parser::hevc::NalUnitHEVC> ppsPerFile;

  std::vector<parser::hevc::ParserAnnexBHEVC> parsers;

  FrameSize frameSize{};

  FileSinkAnnexB                    outputFile;
  parser::hevc::ActiveParameterSets activeWritingParameterSets{};
};

} // namespace combiner
