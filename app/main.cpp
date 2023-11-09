/* Copyright (c) 2023 Christian Feldmann [christian.feldmann@gmx.de].
 * All rights reserved.
 * This work is licensed under the terms of the MIT license.
 * For a copy, see <https://opensource.org/licenses/MIT>.
 */

#include <Combiner.h>
#include <FileSourceAnnexB.h>

#include <filesystem>
#include <iostream>

void printHelp()
{
  std::cout << "BitstreamCombiner. Combine multiple HEVC input bitstreams into one\n";
  std::cout << "that are laid out side by side or in a grid using Tiles.\n";
  std::cout << "Usage:\n";
  std::cout << "  BitstreamCombiner InputFile1.hevc InputFile2.hevc OutputFile.hevc\n";
}
struct Settings
{
  std::vector<std::filesystem::path>   inputFiles;
  std::optional<std::filesystem::path> outputFile;
};

Settings parseCommandLineArguments(int argc, char const *argv[])
{
  Settings settings;
  for (int i = 1; i < argc; ++i)
    settings.inputFiles.push_back(std::filesystem::path(argv[i]));
  if (!settings.inputFiles.empty())
  {
    settings.outputFile = settings.inputFiles.back();
    settings.inputFiles.pop_back();
  }
  return settings;
}

int main(int argc, char const *argv[])
{
  const auto settings = parseCommandLineArguments(argc, argv);

  if (settings.inputFiles.empty() || !settings.outputFile)
  {
    std::cout << "No inputs or output files provided.\n\n";
    printHelp();
    return 1;
  }

  if (settings.inputFiles.size() != 1 && settings.inputFiles.size() != 2 &&
      settings.inputFiles.size() != 4)
  {
    std::cout << "Currently only 1/2/4 input files are supported.\n\n";
    printHelp();
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

  combiner::FileSinkAnnexB outputFile(settings.outputFile.value());

  try
  {
    combiner::Combiner combiner(std::move(fileSources), std::move(outputFile));
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error during combination: " << e.what() << '\n';
  }

  return 0;
}
