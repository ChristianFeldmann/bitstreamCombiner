/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "slice_segment_layer_rbsp.h"

namespace combiner::parser::hevc
{

void slice_segment_layer_rbsp::parse(SubByteReader &        reader,
                                     const bool             firstAUInDecodingOrder,
                                     const uint64_t         prevTid0PicSlicePicOrderCntLsb,
                                     const int              prevTid0PicPicOrderCntMsb,
                                     const nal_unit_header &nalUnitHeader,
                                     const SPSMap &         spsMap,
                                     const PPSMap &         ppsMap,
                                     std::shared_ptr<slice_segment_layer_rbsp> firstSliceInSegment)
{
  this->sliceSegmentHeader.parse(reader,
                                 firstAUInDecodingOrder,
                                 prevTid0PicSlicePicOrderCntLsb,
                                 prevTid0PicPicOrderCntMsb,
                                 nalUnitHeader,
                                 spsMap,
                                 ppsMap,
                                 firstSliceInSegment);
}

} // namespace combiner::parser::hevc