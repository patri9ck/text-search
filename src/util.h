#ifndef UTIL_H
#define UTIL_H

#include <map>
#include <optional>
#include <string>

std::optional<std::string> read_file(const std::string &path,
                                     bool silent = false);

std::map<std::string, std::string> read_directory(const std::string &path,
                                                  bool silent = false);

template <typename T> int countr_zero(T v) {
    static_assert(std::is_unsigned_v<T>);

    if (v == 0) {
        return static_cast<int>(sizeof(T) * 8);
    }

#if defined(_MSC_VER)
    unsigned long index;

    if constexpr (sizeof(T) <= 4) {
        _BitScanForward(&index, static_cast<unsigned long>(v));
    } else {
        _BitScanForward64(&index, static_cast<unsigned __int64>(v));
    }

    return static_cast<int>(index);
#elif defined(__GNUC__) || defined(__clang__)
    if constexpr (sizeof(T) <= sizeof(unsigned int)) {
        return __builtin_ctz(static_cast<unsigned int>(v));
    } else {
        return __builtin_ctzll(static_cast<unsigned long long>(v));
    }
#endif
}

#endif
