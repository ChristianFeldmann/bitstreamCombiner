/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <map>
#include <string>
#include <vector>

namespace combiner::parser
{

template <typename T> class CodingEnum
{
public:
  struct Entry
  {
    Entry(unsigned code, T value, std::string name, std::string meaning = "")
        : code(code), value(value), name(name), meaning(meaning)
    {
    }
    unsigned    code;
    T           value;
    std::string name;
    std::string meaning;
  };

  using EntryVector = std::vector<Entry>;

  CodingEnum() = default;
  CodingEnum(const EntryVector &entryVector, const T unknown)
      : entryVector(entryVector), unknown(unknown){};

  T getValue(unsigned code) const
  {
    for (const auto &entry : this->entryVector)
      if (entry.code == code)
        return entry.value;
    return this->unknown;
  }

  unsigned getCode(T value) const
  {
    for (const auto &entry : this->entryVector)
      if (entry.value == value)
        return entry.code;
    return {};
  }

  std::map<int, std::string> getMeaningMap() const
  {
    std::map<int, std::string> m;
    for (const auto &entry : this->entryVector)
    {
      if (entry.meaning.empty())
        m[int(entry.code)] = entry.name;
      else
        m[int(entry.code)] = entry.meaning;
    }
    return m;
  }

  std::string getMeaning(T value) const
  {
    for (const auto &entry : this->entryVector)
      if (entry.value == value)
        return entry.meaning;
    return {};
  }

private:
  EntryVector entryVector;
  T           unknown;
};

} // namespace combiner::parser
