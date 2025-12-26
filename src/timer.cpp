#include "timer.h"

#include <iostream>
#include <utility>

Timer::Timer(std::string name) : name(std::move(name)) {}

void Timer::start_total() {
    total_start = std::chrono::high_resolution_clock::now();
}

void Timer::stop_total() {
    const auto now = std::chrono::high_resolution_clock::now();

    total_duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - total_start)
            .count();
}

void Timer::start_sequential_part(int index, const std::string &label) {
    const auto now = std::chrono::high_resolution_clock::now();

    if (sequential_parts.contains(index)) {
        sequential_parts.at(index).start = now;
    } else {
        sequential_parts.emplace(index, Part{label, now, 0});
    }
}

void Timer::stop_sequential_part(int index) {
    const auto now = std::chrono::high_resolution_clock::now();

    auto &part = sequential_parts.at(index);

    part.duration +=
        std::chrono::duration_cast<std::chrono::milliseconds>(now - part.start)
            .count();
}

void Timer::start_parallel_part(int index, const std::string &label) {
    const auto now = std::chrono::high_resolution_clock::now();

    if (parallel_parts.contains(index)) {
        parallel_parts.at(index).start = now;
    } else {
        parallel_parts.emplace(index, Part{label, now, 0});
    }
}

void Timer::stop_parallel_part(int index) {
    const auto now = std::chrono::high_resolution_clock::now();

    auto &part = parallel_parts.at(index);

    part.duration +=
        std::chrono::duration_cast<std::chrono::milliseconds>(now - part.start)
            .count();
}

long Timer::get_total_time() const { return total_duration; }

std::map<int, long> Timer::get_sequential_parts() const {
    std::map<int, long> results;

    for (const auto &[index, part] : sequential_parts) {
        results.emplace(index, part.duration);
    }

    return results;
}

std::map<int, long> Timer::get_parallel_parts() const {
    std::map<int, long> results;

    for (const auto &[index, part] : parallel_parts) {
        results.emplace(index, part.duration);
    }

    return results;
}

void Timer::print() const {
    if (!name.empty()) {
        std::cout << "----- " << name << " -----" << std::endl;
    }

    if (!sequential_parts.empty()) {
        for (const auto &[index, part] : sequential_parts) {
            std::cout << "SEQ " << index;

            if (!part.label.empty()) {
                std::cout << " " << part.label;
            }

            std::cout << ": " << part.duration << std::endl;
        }
    }

    if (!parallel_parts.empty()) {
        for (const auto &[index, part] : parallel_parts) {
            std::cout << "PAR " << index;

            if (!part.label.empty()) {
                std::cout << " " << part.label;
            }

            std::cout << ": " << part.duration << std::endl;
        }
    }

    std::cout << "TOT: " << get_total_time() << std::endl;
}
