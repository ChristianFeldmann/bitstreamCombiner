/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <HEVC/seq_parameter_set_rbsp.h>
#include <HEVC/slice_segment_layer_rbsp.h>
#include <HEVC/video_parameter_set_rbsp.h>

namespace combiner
{

using NalUnitVector = std::vector<parser::hevc::NalUnitHEVC>;

parser::hevc::seq_parameter_set_rbsp generateSPSWithNewFrameSize(const NalUnitVector &nalUnits);
parser::hevc::pic_parameter_set_rbsp generatePPSWithUniformTiles(const NalUnitVector &nalUnits);
void                                 checkForMathingSlices(const NalUnitVector &nalUnits);

} // namespace combiner
