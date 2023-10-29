/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "Combiner.h"

#include <HEVC/NalUnitHEVC.h>
#include <common/SubByteReader.h>

#include <iostream>

namespace combiner
{

namespace
{

std::shared_ptr<parser::hevc::NalUnitHEVC> parseNextNalFromFile(combiner::FileSourceAnnexB &file,
                                                                const int                   nalID)
{
  using namespace parser::hevc;

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
  for (int fileIndex = 0; fileIndex < static_cast<int>(inputFiles.size()); ++fileIndex)
    this->parseHeadersFromFile(fileIndex);
}

void Combiner::parseHeadersFromFile(const int fileIndex)
{
  auto &file = this->inputFiles[fileIndex];

  std::cout << "File " << fileIndex << ":\n";

  int  nalID    = 0;
  bool foundVPS = false;

  while (!foundVPS)
  {
    const auto nal = parseNextNalFromFile(file, nalID);
    if (nal->header.nal_unit_type == parser::hevc::NalType::VPS_NUT)
    {
      this->vpsPerFile[fileIndex] = nal;
      foundVPS                    = true;
    }

    std::cout << "  Parsed " << parser::hevc::NalTypeMapper.getName(nal->header.nal_unit_type)
              << "\n";
  }
}

} // namespace combiner
