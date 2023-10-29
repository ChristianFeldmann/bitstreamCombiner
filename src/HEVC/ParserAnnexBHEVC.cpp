/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "ParserAnnexBHEVC.h"

#include <HEVC/pic_parameter_set_rbsp.h>
#include <HEVC/seq_parameter_set_rbsp.h>
#include <HEVC/slice_segment_layer_rbsp.h>
#include <HEVC/video_parameter_set_rbsp.h>

#include <iostream>

namespace combiner::parser::hevc
{

namespace
{

} // namespace

ParserAnnexBHEVC::ParserAnnexBHEVC(combiner::FileSourceAnnexB &&fileSource)
    : fileSource(std::move(fileSource))
{
}

void ParserAnnexBHEVC::parseHeaders()
{
  int  nalID    = 0;
  bool foundVPS = false;
  bool foundSPS = false;
  bool foundPPS = false;
  while (!foundVPS || !foundSPS || !foundPPS)
  {
    const auto nal = this->parseNextNalFromFile();
    if (!nal)
      throw std::logic_error("File ended while we were looking for parameter sets.");

    switch (nal->header.nal_unit_type)
    {
    case NalType::VPS_NUT:
      foundVPS = true;
      break;
    case NalType::SPS_NUT:
      foundSPS = true;
      break;
    case NalType::PPS_NUT:
      foundPPS = true;
      break;

    default:
      break;
    }

    std::cout << "  Parsed " << NalTypeMapper.getName(nal->header.nal_unit_type) << "\n";
  }
}

std::shared_ptr<NalUnitHEVC> ParserAnnexBHEVC::getNextSlice()
{
  while (true)
  {
    const auto nal = this->parseNextNalFromFile();
    if (!nal)
      return {};

    if (nal->header.isSlice())
      return nal;
  }
}

std::shared_ptr<NalUnitHEVC> ParserAnnexBHEVC::parseNextNalFromFile()
{
  const auto nalData = this->fileSource.getNextNALUnit();
  if (nalData.size() == 0)
    return {};

  auto                  nalHEVC = std::make_shared<NalUnitHEVC>();
  parser::SubByteReader reader(nalData);
  nalHEVC->header.parse(reader);

  if (nalHEVC->header.nal_unit_type == NalType::VPS_NUT)
  {
    auto newVPS = std::make_shared<video_parameter_set_rbsp>();
    newVPS->parse(reader);
    nalHEVC->rbsp = newVPS;

    this->activeParameterSets.vpsMap[newVPS->vps_video_parameter_set_id] = newVPS;
  }
  else if (nalHEVC->header.nal_unit_type == NalType::SPS_NUT)
  {
    auto newSPS = std::make_shared<seq_parameter_set_rbsp>();
    newSPS->parse(reader);
    nalHEVC->rbsp = newSPS;

    this->activeParameterSets.spsMap[newSPS->sps_seq_parameter_set_id] = newSPS;
  }
  else if (nalHEVC->header.nal_unit_type == NalType::PPS_NUT)
  {
    auto newPPS = std::make_shared<pic_parameter_set_rbsp>();
    newPPS->parse(reader);
    nalHEVC->rbsp = newPPS;

    this->activeParameterSets.ppsMap[newPPS->pps_pic_parameter_set_id] = newPPS;
  }
  if (nalHEVC->header.isSlice())
  {
    auto newSlice = std::make_shared<slice_segment_layer_rbsp>();
    newSlice->parse(reader,
                    this->firstAUInDecodingOrder,
                    this->prevTid0PicSlicePicOrderCntLsb,
                    this->prevTid0PicPicOrderCntMsb,
                    nalHEVC->header,
                    this->activeParameterSets.spsMap,
                    this->activeParameterSets.ppsMap,
                    this->lastFirstSliceSegmentInPic);
    nalHEVC->rbsp = newSlice;

    this->firstAUInDecodingOrder = false;
    auto TemporalId              = nalHEVC->header.nuh_temporal_id_plus1 - 1;
    if (TemporalId == 0 && !nalHEVC->header.isRASL() && !nalHEVC->header.isRADL())
    {
      // Let prevTid0Pic be the previous picture in decoding order that has TemporalId
      // equal to 0 and that is not a RASL picture, a RADL picture or an SLNR picture.
      // Set these for the next slice
      this->prevTid0PicSlicePicOrderCntLsb = newSlice->sliceSegmentHeader.slice_pic_order_cnt_lsb;
      this->prevTid0PicPicOrderCntMsb      = newSlice->sliceSegmentHeader.PicOrderCntMsb;
    }
    if (newSlice->sliceSegmentHeader.first_slice_segment_in_pic_flag)
      this->lastFirstSliceSegmentInPic = newSlice;
  }

  return nalHEVC;
}

} // namespace combiner::parser::hevc