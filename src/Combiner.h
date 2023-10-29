/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <FileSourceAnnexB.h>
#include <HEVC/video_parameter_set_rbsp.h>

#include <vector>

namespace combiner
{

class Combiner
{
public:
  Combiner(std::vector<combiner::FileSourceAnnexB> &&inputFiles);

private:
  void parseHeadersFromFiles();
  void parseHeadersFromFile(const int fileIndex);

  std::map<int, std::shared_ptr<parser::hevc::NalUnitHEVC>> vpsPerFile;

  std::vector<combiner::FileSourceAnnexB> inputFiles;
};

} // namespace combiner
