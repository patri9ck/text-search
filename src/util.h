#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>

std::optional<std::string> read_file(const std::string &path,
                                     bool silent = false);
std::map<std::string, std::string> read_directory(const std::string &path,
                                                  bool silent = false);

template <typename T> int countr_zero(T v) {
    static_assert(std::is_unsigned_v<T>);

#if defined(_MSC_VER)
    unsigned long index;

    _BitScanForward(&index, v);

    return static_cast<int>(index);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_ctz(v);
#else
    #error "unsupported compiler"
#endif
}

#endif
