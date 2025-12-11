#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <map>
#include <string>

class Timer {
    public:
        explicit Timer(std::string name);

        void start_total();
        void stop_total();
        void start_sequential_part(int index, const std::string &label = "");
        void stop_sequential_part(int index);
        void start_parallel_part(int index, const std::string &label = "");
        void stop_parallel_part(int index);

        long get_total_time() const;

        std::map<int, long> get_sequential_parts() const;
        std::map<int, long> get_parallel_parts() const;

        void print() const;
    private:
        struct Part {
            std::string label;
            std::chrono::time_point<std::chrono::high_resolution_clock> start;
            std::chrono::time_point<std::chrono::high_resolution_clock> end;
        };

        const std::string name;

        std::chrono::time_point<std::chrono::high_resolution_clock> total_start;
        std::chrono::time_point<std::chrono::high_resolution_clock> total_end;

        std::map<int, Part> sequential_parts;
        std::map<int, Part> parallel_parts;
};

#endif // TIMER_H
