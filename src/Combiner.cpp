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
  for (auto &file : this->inputFiles)
  {
    const auto firstNAL = file.getNextNALUnit();
    std::cout << "First NAL: " << firstNAL.size() << "\n";
  }
}

} // namespace combiner
