#include "timer.h"

#include <iostream>
#include <utility>

Timer::Timer(std::string name) : name(std::move(name)) {}

void Timer::start_total() {
    total_start = std::chrono::high_resolution_clock::now();
}

void Timer::stop_total() {
    total_end = std::chrono::high_resolution_clock::now();
}

void Timer::start_sequential_part(int index, const std::string &label) {
    const auto now = std::chrono::high_resolution_clock::now();

    sequential_parts.emplace(index, Part{label, now, now});
}

void Timer::stop_sequential_part(int index) {
    sequential_parts.at(index).end = std::chrono::high_resolution_clock::now();
}

void Timer::start_parallel_part(int index, const std::string &label) {
    const auto now = std::chrono::high_resolution_clock::now();

    parallel_parts.emplace(index, Part{label, now, now});
}

void Timer::stop_parallel_part(int index) {
    parallel_parts.at(index).end = std::chrono::high_resolution_clock::now();
}

long Timer::get_total_time() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start).count();
}

std::map<int, long> Timer::get_sequential_parts() const {
    std::map<int, long> results;

    for (const auto& [index, part] : sequential_parts) {
        results.emplace(index, std::chrono::duration_cast<std::chrono::milliseconds>(part.end - part.start).count());
    }

    return results;
}

std::map<int, long> Timer::get_parallel_parts() const {
    std::map<int, long> results;

    for (const auto& [index, part] : parallel_parts) {
        results.emplace(index, std::chrono::duration_cast<std::chrono::milliseconds>(part.end - part.start).count());
    }

    return results;
}

void Timer::print() const {
    if (!name.empty()) {
        std::cout << "----- " << name << " -----" << std::endl;
    }

    if (!sequential_parts.empty()) {
        for (const auto& [index, part] : sequential_parts) {
            std::cout << "SEQ " << index;

            if (!part.label.empty()) {
                std::cout << " " << part.label;
            }

            std::cout << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(part.end - part.start).count() << std::endl;
        }

        std::cout << std::endl;
    }

    if (!parallel_parts.empty()) {
        for (const auto& [index, part] : parallel_parts) {
            std::cout << "PAR " << index;

            if (!part.label.empty()) {
                std::cout << " " << part.label;
            }

            std::cout << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(part.end - part.start).count() << std::endl;
        }

        std::cout << std::endl;
    }

    std::cout << "TOT: " << get_total_time() << std::endl;
}


