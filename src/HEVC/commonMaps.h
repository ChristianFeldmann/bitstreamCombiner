/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "pic_parameter_set_rbsp.h"
#include "seq_parameter_set_rbsp.h"
#include "video_parameter_set_rbsp.h"

#include <map>
#include <memory>

namespace combiner::parser::hevc
{

class video_parameter_set_rbsp;
class seq_parameter_set_rbsp;
class pic_parameter_set_rbsp;

using VPSMap = std::map<uint64_t, hevc::video_parameter_set_rbsp>;
using SPSMap = std::map<uint64_t, hevc::seq_parameter_set_rbsp>;
using PPSMap = std::map<uint64_t, hevc::pic_parameter_set_rbsp>;

struct ActiveParameterSets
{
  VPSMap vpsMap;
  SPSMap spsMap;
  PPSMap ppsMap;
};

} // namespace combiner::parser::hevc
