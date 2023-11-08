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

  void parse(SubByteReader &               reader,
             const bool                    firstAUInDecodingOrder,
             const uint64_t                prevTid0PicSlicePicOrderCntLsb,
             const int                     prevTid0PicPicOrderCntMsb,
             const nal_unit_header &       nalUnitHeader,
             const ActiveParameterSets &   activeParameterSets,
             const std::optional<uint64_t> firstSliceInSegmentPicOrderCntLsb)
  {
    this->sliceSegmentHeader.parse(reader,
                                   firstAUInDecodingOrder,
                                   prevTid0PicSlicePicOrderCntLsb,
                                   prevTid0PicPicOrderCntMsb,
                                   nalUnitHeader,
                                   activeParameterSets,
                                   firstSliceInSegmentPicOrderCntLsb);
  }

  void write(SubByteWriter &            writer,
             const nal_unit_header &    nalUnitHeader,
             const ActiveParameterSets &activeParameterSets) const
  {
    this->sliceSegmentHeader.write(writer, nalUnitHeader, activeParameterSets);
  }

  slice_segment_header sliceSegmentHeader;
};

} // namespace combiner::parser::hevc
