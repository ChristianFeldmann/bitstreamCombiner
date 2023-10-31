/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include "video_parameter_set_rbsp.h"

#include "rbsp_trailing_bits.h"

namespace combiner::parser::hevc
{

void video_parameter_set_rbsp::parse(SubByteReader &reader)
{
  this->vps_video_parameter_set_id      = reader.readBits(4);
  this->vps_base_layer_internal_flag    = reader.readFlag();
  this->vps_base_layer_available_flag   = reader.readFlag();
  this->vps_max_layers_minus1           = reader.readBits(6);
  this->vps_max_sub_layers_minus1       = reader.readBits(3);
  this->vps_temporal_id_nesting_flag    = reader.readFlag();
  const auto vps_reserved_0xffff_16bits = reader.readBits(16);
  if (vps_reserved_0xffff_16bits != 0xffff)
    throw std::logic_error("The vps_reserved_0xffff_16bits is not equal to 0xffff");

  this->profileTierLevel.parse(reader, true, this->vps_max_sub_layers_minus1);

  this->vps_sub_layer_ordering_info_present_flag = reader.readFlag();
  for (auto i =
           (this->vps_sub_layer_ordering_info_present_flag ? 0 : this->vps_max_sub_layers_minus1);
       i <= this->vps_max_sub_layers_minus1;
       i++)
  {
    this->vps_max_dec_pic_buffering_minus1[i] = reader.readUEV();
    this->vps_max_num_reorder_pics[i]         = reader.readUEV();
    this->vps_max_latency_increase_plus1[i]   = reader.readUEV();
  }

  this->vps_max_layer_id          = reader.readBits(6);
  this->vps_num_layer_sets_minus1 = reader.readUEV();

  for (unsigned int i = 1; i <= this->vps_num_layer_sets_minus1; i++)
    for (unsigned int j = 0; j <= this->vps_max_layer_id; j++)
      this->layer_id_included_flag[i].push_back(reader.readFlag());

  this->vps_timing_info_present_flag = reader.readFlag();
  if (this->vps_timing_info_present_flag)
  {
    // The VPS can provide timing info (the time between two POCs. So the refresh rate)
    this->vps_num_units_in_tick               = reader.readBits(32);
    this->vps_time_scale                      = reader.readBits(32);
    this->vps_poc_proportional_to_timing_flag = reader.readFlag();
    if (this->vps_poc_proportional_to_timing_flag)
      this->vps_num_ticks_poc_diff_one_minus1 = reader.readUEV();

    // HRD parameters ...
    this->vps_num_hrd_parameters = reader.readUEV();
    for (unsigned int i = 0; i < vps_num_hrd_parameters; i++)
    {
      this->hrd_layer_set_idx.push_back(reader.readUEV());

      if (i == 0)
        this->cprms_present_flag.push_back(1);
      else
        this->cprms_present_flag.push_back(reader.readFlag());

      hrd_parameters hrdParameters;
      hrdParameters.parse(reader, cprms_present_flag[i], vps_max_sub_layers_minus1);
      this->vps_hrd_parameters.push_back(hrdParameters);
    }

    this->frameRate = (double)vps_time_scale / (double)vps_num_units_in_tick;
  }

  this->vps_extension_flag = reader.readFlag();
  if (this->vps_extension_flag)
    throw std::runtime_error("Not implemented yet");

  // TODO:
  // Here comes the VPS extension.
  // This is specified in the annex F, multilayer and stuff.
  // This could be added and is definitely interesting.
  // ... later

  rbsp_trailing_bits::parse(reader);
}

void video_parameter_set_rbsp::write(SubByteWriter &writer) const
{
  writer.writeBits(this->vps_video_parameter_set_id, 4);
  writer.writeFlag(this->vps_base_layer_internal_flag);
  writer.writeFlag(this->vps_base_layer_available_flag);
  writer.writeBits(this->vps_max_layers_minus1, 6);
  writer.writeBits(this->vps_max_sub_layers_minus1, 3);
  writer.writeFlag(this->vps_temporal_id_nesting_flag);

  const auto vps_reserved_0xffff_16bits = 0xffff;
  writer.writeBits(vps_reserved_0xffff_16bits, 16);

  this->profileTierLevel.write(writer, true, this->vps_max_sub_layers_minus1);

  writer.writeFlag(this->vps_sub_layer_ordering_info_present_flag);
  for (auto i =
           (this->vps_sub_layer_ordering_info_present_flag ? 0 : this->vps_max_sub_layers_minus1);
       i <= this->vps_max_sub_layers_minus1;
       i++)
  {
    writer.writeUEV(this->vps_max_dec_pic_buffering_minus1[i]);
    writer.writeUEV(this->vps_max_num_reorder_pics[i]);
    writer.writeUEV(this->vps_max_latency_increase_plus1[i]);
  }

  writer.writeBits(this->vps_max_layer_id, 6);
  writer.writeUEV(this->vps_num_layer_sets_minus1);

  for (unsigned int i = 1; i <= this->vps_num_layer_sets_minus1; i++)
    for (unsigned int j = 0; j <= this->vps_max_layer_id; j++)
      writer.writeFlag(this->layer_id_included_flag[i].at(j));

  writer.writeFlag(this->vps_timing_info_present_flag);
  if (this->vps_timing_info_present_flag)
  {
    // The VPS can provide timing info (the time between two POCs. So the refresh rate)
    writer.writeBits(this->vps_num_units_in_tick, 32);
    writer.writeBits(this->vps_time_scale, 32);
    writer.writeFlag(this->vps_poc_proportional_to_timing_flag);
    if (this->vps_poc_proportional_to_timing_flag)
      writer.writeUEV(this->vps_num_ticks_poc_diff_one_minus1);

    // HRD parameters ...
    writer.writeUEV(this->vps_num_hrd_parameters);
    for (unsigned int i = 0; i < vps_num_hrd_parameters; i++)
    {
      writer.writeUEV(this->hrd_layer_set_idx.at(i));

      this->vps_hrd_parameters.at(i).write(
          writer, this->cprms_present_flag.at(i), this->vps_max_sub_layers_minus1);
    }
  }

  writer.writeFlag(this->vps_extension_flag);
  if (this->vps_extension_flag)
    throw std::runtime_error("Not implemented yet");

  rbsp_trailing_bits::write(writer);
}

} // namespace combiner::parser::hevc