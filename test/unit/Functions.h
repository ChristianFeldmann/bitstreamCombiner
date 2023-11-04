/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include <common/Typedef.h>

#include <HEVC/pic_parameter_set_rbsp.h>
#include <HEVC/seq_parameter_set_rbsp.h>
#include <HEVC/slice_segment_layer_rbsp.h>
#include <HEVC/video_parameter_set_rbsp.h>

namespace combiner
{

template <typename T> T parserParameterSetFromData(const ByteVector &data)
{
  parser::SubByteReader reader(data);

  T parameterSet;
  parameterSet.parse(reader);

  return parameterSet;
}

template <typename T>
void writeParameterSetAndCompareToReference(const T &parameterSet, const ByteVector &referenceData)
{
  parser::SubByteWriter writer;
  parameterSet.write(writer);

  const auto writtenData = writer.finishWritingAndGetData();

  EXPECT_EQ(referenceData.size(), writtenData.size());
  EXPECT_TRUE(std::equal(referenceData.begin(), referenceData.end(), writtenData.begin()));
}

} // namespace combiner
