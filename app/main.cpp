/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include <Combiner.h>
#include <FileSourceAnnexB.h>

#include <filesystem>
#include <iostream>

struct Settings
{
  std::vector<std::filesystem::path> inputFiles;
};

Settings parseCommandLineArguments(int argc, char const *argv[])
{
  Settings settings;
  for (int i = 1; i < argc; ++i)
    settings.inputFiles.push_back(std::filesystem::path(argv[i]));
  return settings;
}

int main(int argc, char const *argv[])
{
  const auto settings = parseCommandLineArguments(argc, argv);

  if (settings.inputFiles.empty())
  {
    std::cout << "Please provide a list of input files to process.\n";
    return 1;
  }

  if (settings.inputFiles.size() != 2)
  {
    std::cout << "Currently only 2 input files are supported.\n";
    return 1;
  }

  std::vector<combiner::FileSourceAnnexB> fileSources;
  for (const auto &file : settings.inputFiles)
  {
    const auto fileStatus = std::filesystem::status(file);
    if (fileStatus.type() == std::filesystem::file_type::not_found)
    {
      std::cout << "Unable to find input file " << file << "\n";
      return 1;
    }
    fileSources.emplace_back(file);
  }

  combiner::FileSinkAnnexB outputFile("combinedFile.hevc");

  combiner::Combiner combiner(std::move(fileSources), std::move(outputFile));

  return 0;
}
