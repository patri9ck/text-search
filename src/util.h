#ifndef UTIL_H
#define UTIL_H

#include <map>
#include <optional>
#include <string>

std::optional<std::string> read_file(const std::string &path,
                                     bool silent = false);
std::map<std::string, std::string> read_directory(const std::string &path,
                                                  bool silent = false);

#endif
