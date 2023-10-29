/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/Typedef.h>

#include "nal_unit_header.h"

#include <memory>
#include <optional>

namespace combiner::parser::hevc
{

class NalRBSP
{
public:
  NalRBSP()          = default;
  virtual ~NalRBSP() = default;
};

class NalUnitHEVC
{
public:
  NalUnitHEVC() = default;

  nal_unit_header          header{};
  std::shared_ptr<NalRBSP> rbsp{};

  ByteVector rawData{};
};

using NalMap = std::map<unsigned, std::shared_ptr<NalUnitHEVC>>;

} // namespace combiner::parser::hevc
