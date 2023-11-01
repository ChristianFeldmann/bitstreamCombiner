/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "st_ref_pic_set.h"

#include <stdexcept>

#include "slice_segment_header.h"

namespace combiner::parser::hevc
{

uint64_t st_ref_pic_set::st_ref_pic_set::NumNegativePics[65]{};
uint64_t st_ref_pic_set::st_ref_pic_set::NumPositivePics[65]{};
int      st_ref_pic_set::st_ref_pic_set::DeltaPocS0[65][16]{};
int      st_ref_pic_set::st_ref_pic_set::DeltaPocS1[65][16]{};
bool     st_ref_pic_set::st_ref_pic_set::UsedByCurrPicS0[65][16]{};
bool     st_ref_pic_set::st_ref_pic_set::UsedByCurrPicS1[65][16]{};
uint64_t st_ref_pic_set::st_ref_pic_set::NumDeltaPocs[65]{};

void st_ref_pic_set::parse(SubByteReader &reader,
                           const uint64_t stRpsIdx,
                           const uint64_t num_short_term_ref_pic_sets)
{
  if (stRpsIdx > 64)
    throw std::logic_error(
        "Error while parsing short term ref pic set. The stRpsIdx must be in the range [0..64].");

  this->inter_ref_pic_set_prediction_flag = false;
  if (stRpsIdx != 0)
    this->inter_ref_pic_set_prediction_flag = reader.readFlag();

  if (this->inter_ref_pic_set_prediction_flag)
  {
    this->delta_idx_minus1 = 0;
    if (stRpsIdx == num_short_term_ref_pic_sets)
      this->delta_idx_minus1 = reader.readUEV();
    this->delta_rps_sign       = reader.readFlag();
    this->abs_delta_rps_minus1 = reader.readUEV();

    const auto RefRpsIdx =
        stRpsIdx - (this->delta_idx_minus1 + 1); // Rec. ITU-T H.265 v3 (04/2015) (7-57)
    const auto deltaRps =
        (1 - 2 * this->delta_rps_sign) *
        static_cast<int>(this->abs_delta_rps_minus1 + 1); // Rec. ITU-T H.265 v3 (04/2015) (7-58)

    for (uint64_t j = 0; j <= NumDeltaPocs[RefRpsIdx]; j++)
    {
      this->used_by_curr_pic_flag.push_back(reader.readFlag());
      if (!this->used_by_curr_pic_flag.back())
        this->use_delta_flag.push_back(reader.readFlag());
      else
        this->use_delta_flag.push_back(true);
    }

    // Derive NumNegativePics Rec. ITU-T H.265 v3 (04/2015) (7-59)
    int i = 0;
    for (int j = int(NumPositivePics[RefRpsIdx]) - 1; j >= 0; j--)
    {
      const auto dPoc = DeltaPocS1[RefRpsIdx][j] + deltaRps;
      if (dPoc < 0 && this->use_delta_flag[NumNegativePics[RefRpsIdx] + j])
      {
        DeltaPocS0[stRpsIdx][i] = dPoc;
        UsedByCurrPicS0[stRpsIdx][i++] =
            this->used_by_curr_pic_flag[NumNegativePics[RefRpsIdx] + j];
      }
    }
    if (deltaRps < 0 && this->use_delta_flag[NumDeltaPocs[RefRpsIdx]])
    {
      DeltaPocS0[stRpsIdx][i]        = static_cast<int>(deltaRps);
      UsedByCurrPicS0[stRpsIdx][i++] = this->used_by_curr_pic_flag[NumDeltaPocs[RefRpsIdx]];
    }
    for (uint64_t j = 0; j < NumNegativePics[RefRpsIdx]; j++)
    {
      const auto dPoc = DeltaPocS0[RefRpsIdx][j] + deltaRps;
      if (dPoc < 0 && this->use_delta_flag[j])
      {
        DeltaPocS0[stRpsIdx][i]        = dPoc;
        UsedByCurrPicS0[stRpsIdx][i++] = this->used_by_curr_pic_flag[j];
      }
    }
    NumNegativePics[stRpsIdx] = i;

    // Derive NumPositivePics Rec. ITU-T H.265 v3 (04/2015) (7-60)
    i = 0;
    for (int j = int(NumNegativePics[RefRpsIdx]) - 1; j >= 0; j--)
    {
      auto dPoc = DeltaPocS0[RefRpsIdx][j] + deltaRps;
      if (dPoc > 0 && this->use_delta_flag[j])
      {
        DeltaPocS1[stRpsIdx][i]        = dPoc;
        UsedByCurrPicS1[stRpsIdx][i++] = this->used_by_curr_pic_flag[j];
      }
    }
    if (deltaRps > 0 && this->use_delta_flag[NumDeltaPocs[RefRpsIdx]])
    {
      DeltaPocS1[stRpsIdx][i]        = deltaRps;
      UsedByCurrPicS1[stRpsIdx][i++] = this->used_by_curr_pic_flag[NumDeltaPocs[RefRpsIdx]];
    }
    for (unsigned j = 0; j < NumPositivePics[RefRpsIdx]; j++)
    {
      int dPoc = DeltaPocS1[RefRpsIdx][j] + deltaRps;
      if (dPoc > 0 && this->use_delta_flag[NumNegativePics[RefRpsIdx] + j])
      {
        DeltaPocS1[stRpsIdx][i] = dPoc;
        UsedByCurrPicS1[stRpsIdx][i++] =
            this->used_by_curr_pic_flag[NumNegativePics[RefRpsIdx] + j];
      }
    }
    NumPositivePics[stRpsIdx] = i;
  }
  else
  {
    this->num_negative_pics = reader.readUEV();
    this->num_positive_pics = reader.readUEV();
    for (unsigned i = 0; i < num_negative_pics; i++)
    {
      this->delta_poc_s0_minus1.push_back(reader.readUEV());
      this->used_by_curr_pic_s0_flag.push_back(reader.readFlag());

      if (i == 0)
        DeltaPocS0[stRpsIdx][i] = -(int(this->delta_poc_s0_minus1.back()) + 1); // (7-65)
      else
        DeltaPocS0[stRpsIdx][i] = DeltaPocS0[stRpsIdx][i - 1] -
                                  static_cast<int>(this->delta_poc_s0_minus1.back() + 1); // (7-67)
      UsedByCurrPicS0[stRpsIdx][i] = used_by_curr_pic_s0_flag[i];
    }
    for (unsigned i = 0; i < this->num_positive_pics; i++)
    {
      this->delta_poc_s1_minus1.push_back(reader.readUEV());
      this->used_by_curr_pic_s1_flag.push_back(reader.readFlag());

      if (i == 0)
        DeltaPocS1[stRpsIdx][i] = static_cast<int>(this->delta_poc_s1_minus1.back() + 1); // (7-66)
      else
        DeltaPocS1[stRpsIdx][i] = DeltaPocS1[stRpsIdx][i - 1] +
                                  static_cast<int>(this->delta_poc_s1_minus1.back() + 1); // (7-68)
      UsedByCurrPicS1[stRpsIdx][i] = used_by_curr_pic_s1_flag[i];
    }

    NumNegativePics[stRpsIdx] = num_negative_pics;
    NumPositivePics[stRpsIdx] = num_positive_pics;
  }

  NumDeltaPocs[stRpsIdx] = NumNegativePics[stRpsIdx] + NumPositivePics[stRpsIdx]; // (7-69)
}

void st_ref_pic_set::write(SubByteWriter &writer,
                           const uint64_t stRpsIdx,
                           const uint64_t num_short_term_ref_pic_sets) const
{
  if (stRpsIdx > 64)
    throw std::logic_error(
        "Error while writing short term ref pic set. The stRpsIdx must be in the range [0..64].");

  if (stRpsIdx != 0)
    writer.writeFlag(this->inter_ref_pic_set_prediction_flag);

  if (this->inter_ref_pic_set_prediction_flag)
  {
    if (stRpsIdx == num_short_term_ref_pic_sets)
      writer.writeUEV(this->delta_idx_minus1);
    writer.writeFlag(this->delta_rps_sign);
    writer.writeUEV(this->abs_delta_rps_minus1);

    const auto RefRpsIdx =
        stRpsIdx - (this->delta_idx_minus1 + 1); // Rec. ITU-T H.265 v3 (04/2015) (7-57)

    for (uint64_t j = 0; j <= NumDeltaPocs[RefRpsIdx]; j++)
    {
      writer.writeFlag(this->used_by_curr_pic_flag.at(j));
      if (!this->used_by_curr_pic_flag.at(j))
        writer.writeFlag(this->use_delta_flag.at(j));
    }
  }
  else
  {
    writer.writeUEV(this->num_negative_pics);
    writer.writeUEV(this->num_positive_pics);
    for (unsigned i = 0; i < this->num_negative_pics; i++)
    {
      writer.writeUEV(this->delta_poc_s0_minus1.at(i));
      writer.writeFlag(this->used_by_curr_pic_s0_flag.at(i));
    }
    for (unsigned i = 0; i < num_positive_pics; i++)
    {
      writer.writeUEV(this->delta_poc_s1_minus1.at(i));
      writer.writeFlag(this->used_by_curr_pic_s1_flag.at(i));
    }
  }
}

// (7-55)
unsigned st_ref_pic_set::NumPicTotalCurr(const uint64_t              CurrRpsIdx,
                                         const slice_segment_header *slice)
{
  int NumPicTotalCurr = 0;
  for (unsigned int i = 0; i < NumNegativePics[CurrRpsIdx]; i++)
    if (UsedByCurrPicS0[CurrRpsIdx][i])
      NumPicTotalCurr++;
  for (unsigned int i = 0; i < NumPositivePics[CurrRpsIdx]; i++)
    if (UsedByCurrPicS1[CurrRpsIdx][i])
      NumPicTotalCurr++;
  for (unsigned int i = 0; i < slice->num_long_term_sps + slice->num_long_term_pics; i++)
    if (slice->UsedByCurrPicLt[i])
      NumPicTotalCurr++;
  return NumPicTotalCurr;
}

} // namespace combiner::parser::hevc