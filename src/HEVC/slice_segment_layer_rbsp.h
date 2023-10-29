/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/SubByteReader.h>

#include "NalUnitHEVC.h"
#include "commonMaps.h"
#include "nal_unit_header.h"
#include "slice_segment_header.h"

namespace combiner::parser::hevc
{

class slice_segment_layer_rbsp : public NalRBSP
{
public:
  slice_segment_layer_rbsp() {}

  void parse(SubByteReader &                           reader,
             const bool                                firstAUInDecodingOrder,
             const uint64_t                            prevTid0PicSlicePicOrderCntLsb,
             const int                                 prevTid0PicPicOrderCntMsb,
             const nal_unit_header &                   nalUnitHeader,
             const SPSMap &                            spsMap,
             const PPSMap &                            ppsMap,
             std::shared_ptr<slice_segment_layer_rbsp> firstSliceInSegment);

  slice_segment_header sliceSegmentHeader;
};

} // namespace combiner::parser::hevc