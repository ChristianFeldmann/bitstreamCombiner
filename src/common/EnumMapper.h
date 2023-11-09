/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <common/Functions.h>

#include <algorithm>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{

std::string toLower(std::string str)
{
  std::transform(
      str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
  return str;
}

std::optional<unsigned long> toUnsigned(const std::string &text)
{
  try
  {
    auto index = std::stoul(text);
    return index;
  }
  catch (...)
  {
    return {};
  }
}

} // namespace

/* This class implement mapping of "enum class" values to and from names (string).
 */
template <typename T> class EnumMapper
{
public:
  enum class StringType
  {
    Name,
    Text,
    NameOrIndex
  };
  struct Entry
  {
    Entry(T value, std::string name) : value(value), name(name) {}
    Entry(T value, std::string name, std::string text) : value(value), name(name), text(text) {}
    T           value;
    std::string name;
    std::string text;
  };

  using EntryVector = std::vector<Entry>;

  EnumMapper() = default;
  EnumMapper(const EntryVector &entryVector) : entryVector(entryVector){};

  std::optional<T> getValue(std::string name, StringType stringType = StringType::Name) const
  {
    if (stringType == StringType::NameOrIndex)
      if (auto index = toUnsigned(name))
        return this->at(*index);

    for (const auto &entry : this->entryVector)
    {
      if ((stringType == StringType::Name && entry.name == name) ||
          (stringType == StringType::NameOrIndex && entry.text == name) ||
          (stringType == StringType::Text && entry.text == name))
        return entry.value;
    }
    return {};
  }

  std::optional<T> getValueCaseInsensitive(std::string name,
                                           StringType  stringType = StringType::Name) const
  {
    if (stringType == StringType::NameOrIndex)
      if (auto index = toUnsigned(name))
        return this->at(*index);

    name = toLower(name);
    for (const auto &entry : this->entryVector)
    {
      if ((stringType == StringType::Name && toLower(entry.name) == name) ||
          (stringType == StringType::NameOrIndex && toLower(entry.text) == name) ||
          (stringType == StringType::Text && toLower(entry.text) == name))
        return entry.value;
    }
    return {};
  }

  std::string getName(T value) const
  {
    for (const auto &entry : this->entryVector)
      if (entry.value == value)
        return entry.name;
    throw std::logic_error(
        "The given type T was not registered in the mapper. All possible enums must be mapped.");
  }

  std::string getText(T value) const
  {
    for (const auto &entry : this->entryVector)
      if (entry.value == value)
        return entry.text;
    throw std::logic_error(
        "The given type T was not registered in the mapper. All possible enums must be mapped.");
  }

  size_t indexOf(T value) const
  {
    for (size_t i = 0; i < this->entryVector.size(); i++)
      if (this->entryVector.at(i).value == value)
        return i;
    throw std::logic_error(
        "The given type T was not registered in the mapper. All possible enums must be mapped.");
  }

  std::optional<T> at(size_t index) const
  {
    if (index >= this->entryVector.size())
      return {};
    return this->entryVector.at(index).value;
  }

  std::vector<T> getEnums() const
  {
    std::vector<T> m;
    for (const auto &entry : this->entryVector)
      m.push_back(entry.value);
    return m;
  }

  std::vector<std::string> getNames() const
  {
    std::vector<std::string> l;
    for (const auto &entry : this->entryVector)
      l.push_back(entry.name);
    return l;
  }

  std::vector<std::string> getTextEntries() const
  {
    std::vector<std::string> l;
    for (const auto &entry : this->entryVector)
      l.push_back(entry.text);
    return l;
  }

  size_t size() const { return this->entryVector.size(); }

  const EntryVector &entries() const { return this->entryVector; }

private:
  EntryVector entryVector;
};
