/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "Combiner.h"

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
  while (true)
  {
    int fileIndex = 0;
    for (auto &file : this->inputFiles)
    {
      const auto nalData = file.getNextNALUnit();
      if (nalData.size() == 0)
        return;
      std::cout << "File " << fileIndex++ << " NAL: " << nalData.size() << " ";
    }
    std::cout << "\n";
  }
}

} // namespace combiner
