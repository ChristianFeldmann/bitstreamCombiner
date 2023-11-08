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

NalUnitHEVC ParserAnnexBHEVC::parseNextNalFromFile()
{
  const auto nalData = this->fileSource.getNextNALUnit();
  if (nalData.size() == 0)
    return {};

  NalUnitHEVC           nal(nalData);
  parser::SubByteReader reader(nalData);
  nal.header.parse(reader);

  if (nal.header.nal_unit_type == NalType::VPS_NUT)
  {
    video_parameter_set_rbsp vps;
    vps.parse(reader);
    nal.rbsp = std::make_unique<video_parameter_set_rbsp>(vps);

    this->activeParameterSets.vpsMap[vps.vps_video_parameter_set_id] = vps;
  }
  else if (nal.header.nal_unit_type == NalType::SPS_NUT)
  {
    seq_parameter_set_rbsp sps;
    sps.parse(reader);
    nal.rbsp = std::make_unique<seq_parameter_set_rbsp>(sps);

    this->activeParameterSets.spsMap[sps.sps_seq_parameter_set_id] = sps;
  }
  else if (nal.header.nal_unit_type == NalType::PPS_NUT)
  {
    pic_parameter_set_rbsp pps;
    pps.parse(reader);
    nal.rbsp = std::make_unique<pic_parameter_set_rbsp>(pps);

    this->activeParameterSets.ppsMap[pps.pps_pic_parameter_set_id] = pps;
  }
  if (nal.header.isSlice())
  {
    slice_segment_layer_rbsp slice;
    slice.parse(reader,
                this->firstAUInDecodingOrder,
                this->prevTid0PicSlicePicOrderCntLsb,
                this->prevTid0PicPicOrderCntMsb,
                nal.header,
                this->activeParameterSets,
                this->firstSliceInSegmentPicOrderCntLsb);

    this->firstAUInDecodingOrder = false;
    auto TemporalId              = nal.header.nuh_temporal_id_plus1 - 1;
    if (TemporalId == 0 && !nal.header.isRASL() && !nal.header.isRADL())
    {
      // Let prevTid0Pic be the previous picture in decoding order that has TemporalId
      // equal to 0 and that is not a RASL picture, a RADL picture or an SLNR picture.
      // Set these for the next slice
      this->prevTid0PicSlicePicOrderCntLsb = slice.sliceSegmentHeader.slice_pic_order_cnt_lsb;
      this->prevTid0PicPicOrderCntMsb      = slice.sliceSegmentHeader.PicOrderCntMsb;
    }
    if (slice.sliceSegmentHeader.first_slice_segment_in_pic_flag)
      this->firstSliceInSegmentPicOrderCntLsb = slice.sliceSegmentHeader.slice_pic_order_cnt_lsb;

    nal.rbsp = std::make_unique<slice_segment_layer_rbsp>(slice);
  }

  return nal;
}

const ActiveParameterSets &ParserAnnexBHEVC::getActiveParameterSets() const
{
  return this->activeParameterSets;
}

} // namespace combiner::parser::hevc
