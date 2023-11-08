/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "NalUnitHEVC.h"
#include "commonMaps.h"
#include "slice_segment_layer_rbsp.h"

#include <FileSourceAnnexB.h>
#include <optional>

namespace combiner::parser::hevc
{

class ParserAnnexBHEVC
{
public:
  ParserAnnexBHEVC(combiner::FileSourceAnnexB &&file);

  NalUnitHEVC parseNextNalFromFile();

  const ActiveParameterSets &getActiveParameterSets() const;

private:
  FileSourceAnnexB fileSource;

  ActiveParameterSets activeParameterSets;

  bool     firstAUInDecodingOrder{true};
  uint64_t prevTid0PicSlicePicOrderCntLsb{};
  int      prevTid0PicPicOrderCntMsb{};

  std::optional<uint64_t> firstSliceInSegmentPicOrderCntLsb{};
};

} // namespace combiner::parser::hevc
