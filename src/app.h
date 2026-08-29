#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <options-parser.h>

/// @brief App wrapper class
class CoderApp {
 public:
  /// @brief Constructor
  /// @param params - general generation configuration
  CoderApp(const OptionsParser::GenOptions& params) : Params(params) {}

  /// @brief Main generation process
  /// @return process exit code: 0 on success, 1 if generation was refused
  int Run();

 private:
  bool AreParamsValid();
  int GenerateCode();
  void PrintHelp();

  const OptionsParser::GenOptions& Params;
};
