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

Combiner::Combiner(std::vector<combiner::FileSourceAnnexB> &&inputFiles)
    : inputFiles(std::move(inputFiles))
{
  this->parseHeadersFromFiles();
}

void Combiner::parseHeadersFromFiles()
{
  int nalID = 0;
  while (true)
  {
    int fileIndex = 0;
    for (auto &file : this->inputFiles)
    {
      const auto nalData = file.getNextNALUnit();
      if (nalData.size() == 0)
        return;

      auto                  nalHEVC = std::make_shared<parser::hevc::NalUnitHEVC>(nalID);
      parser::SubByteReader reader(nalData);
      nalHEVC->header.parse(reader);

      std::cout << "File " << fileIndex++ << " NAL "
                << parser::hevc::NalTypeMapper.getName(nalHEVC->header.nal_unit_type) << " ";
    }
    std::cout << "\n";

    nalID++;
  }
}

} // namespace combiner
