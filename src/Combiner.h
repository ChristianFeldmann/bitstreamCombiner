/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <FileSourceAnnexB.h>

#include <vector>

namespace combiner
{

class Combiner
{
public:
  Combiner(std::vector<combiner::FileSourceAnnexB> &&inputFiles);

private:
  void parseHeadersFromFiles();

  std::vector<combiner::FileSourceAnnexB> inputFiles;
};

} // namespace combiner
