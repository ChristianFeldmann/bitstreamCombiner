/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "Combiner.h"

#include <HEVC/NalUnitHEVC.h>
#include <HEVC/pic_parameter_set_rbsp.h>
#include <HEVC/seq_parameter_set_rbsp.h>
#include <HEVC/video_parameter_set_rbsp.h>
#include <common/SubByteReader.h>

#include <iostream>

namespace combiner
{

using namespace parser::hevc;

namespace
{

std::shared_ptr<parser::hevc::NalUnitHEVC> parseNextNalFromFile(combiner::FileSourceAnnexB &file,
                                                                const int                   nalID)
{
  const auto nalData = file.getNextNALUnit();
  if (nalData.size() == 0)
    return {};

  auto                  nalHEVC = std::make_shared<NalUnitHEVC>(nalID);
  parser::SubByteReader reader(nalData);
  nalHEVC->header.parse(reader);

  if (nalHEVC->header.nal_unit_type == NalType::VPS_NUT)
  {
    auto newVPS = std::make_shared<video_parameter_set_rbsp>();
    newVPS->parse(reader);
    nalHEVC->rbsp = newVPS;
  }
  else if (nalHEVC->header.nal_unit_type == NalType::SPS_NUT)
  {
    auto newSPS = std::make_shared<seq_parameter_set_rbsp>();
    newSPS->parse(reader);
    nalHEVC->rbsp = newSPS;
  }
  else if (nalHEVC->header.nal_unit_type == NalType::PPS_NUT)
  {
    auto newPPS = std::make_shared<pic_parameter_set_rbsp>();
    newPPS->parse(reader);
    nalHEVC->rbsp = newPPS;
  }

  return nalHEVC;
}

} // namespace

Combiner::Combiner(std::vector<combiner::FileSourceAnnexB> &&inputFiles)
    : inputFiles(std::move(inputFiles))
{
  this->parseHeadersFromFiles();
}

void Combiner::parseHeadersFromFiles()
{
  std::cout << "Getting headers from files...\n";

  for (int fileIndex = 0; fileIndex < static_cast<int>(inputFiles.size()); ++fileIndex)
    this->parseHeadersFromFile(fileIndex);
}

void Combiner::parseHeadersFromFile(const int fileIndex)
{
  auto &file = this->inputFiles[fileIndex];

  std::cout << "File " << fileIndex << ":\n";

  int  nalID    = 0;
  bool foundVPS = false;
  bool foundSPS = false;
  bool foundPPS = false;
  while (!foundVPS || !foundSPS || !foundPPS)
  {
    const auto nal = parseNextNalFromFile(file, nalID);

    switch (nal->header.nal_unit_type)
    {
    case NalType::VPS_NUT:
      this->vpsPerFile[fileIndex] = nal;
      foundVPS                    = true;
      break;
    case NalType::SPS_NUT:
      this->spsPerFile[fileIndex] = nal;
      foundSPS                    = true;
      break;
    case NalType::PPS_NUT:
      this->ppsPerFile[fileIndex] = nal;
      foundPPS                    = true;
      break;

    default:
      break;
    }

    std::cout << "  Parsed " << NalTypeMapper.getName(nal->header.nal_unit_type) << "\n";
  }
}

} // namespace combiner
