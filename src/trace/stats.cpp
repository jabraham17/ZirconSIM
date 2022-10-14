#include "stats.h"
#include "cpu/cpu.h"
#include <cpu/isa/inst.h>
#include <functional>
#include <iomanip>
#include <sstream>

namespace internal {
static std::
    map<std::string, std::function<void(const cpu::HartState&, float& counter)>>
        counter_funcs;
static std::string counter_names[] = {
#define COUNTER(name, ...) name,
#include "stats.inc"
};
static size_t number_counters = 0
#define COUNTER(name, ...) +1
#include "stats.inc"
    ;

void initCounterMap(std::map<std::string, float>& m) {
#define COUNTER(name, expression)                                              \
    m[name] = 0;                                                               \
    counter_funcs[name] = []([[maybe_unused]] const cpu::HartState& hs,        \
                             [[maybe_unused]] float& counter) {                \
        do {                                                                   \
            expression;                                                        \
        } while(0);                                                            \
    };
#include "stats.inc"
}

static std::map<
    std::string,
    std::function<
        float(const cpu::HartState&, const std::map<std::string, float>&)>>
    computed_funcs;
static std::string computed_names[] = {
#define COMPUTED(name, ...) name,
#include "stats.inc"
};
static size_t number_computed = 0
#define COMPUTED(name, ...) +1
#include "stats.inc"
    ;

void initComputedMap(std::map<std::string, float>& m) {
#define COMPUTED(name, expression)                                             \
    m[name] = 0;                                                               \
    computed_funcs[name] =                                                     \
        []([[maybe_unused]] const cpu::HartState& hs,                          \
           [[maybe_unused]] std::map<std::string, float> counters) -> float {  \
        do {                                                                   \
            expression;                                                        \
        } while(0);                                                            \
        return 0;                                                              \
    };
#include "stats.inc"
}

// std::map<std::string, uint64_t> getComputed(
//     const std::map<std::string, uint64_t>& counters,
//     const cpu::HartState& hs) {
//     std::map<std::string, uint64_t> computed;
// #define COMPUTED(name, expression)
//     computed[name] =
//         []([[maybe_unused]] const std::map<std::string, uint64_t>& counters
//            [[maybe_unused]] const cpu::HartState& hs) {
//             do {
//                 expression;
//             } while(0);
//         }();
// #include "stats.inc"

//     return computed;
// }

} // namespace internal

Stats::Stats() : enabled(false) {
    internal::initCounterMap(counters);
    internal::initComputedMap(computed_counters);
}

void Stats::count(const cpu::HartState& hs) {
    if(!enabled) return;

    for(size_t idx = 0; idx < internal::number_counters; idx++) {
        const auto& key = internal::counter_names[idx];
        auto& counter_value = counters[key];
        auto func = internal::counter_funcs[key];
        func(hs, counter_value);
    }
    for(size_t idx = 0; idx < internal::number_computed; idx++) {
        const auto& key = internal::computed_names[idx];
        auto& computed_value = computed_counters[key];
        auto func = internal::computed_funcs[key];
        computed_value = func(hs, counters);
    }
}

std::string Stats::dump() {
    std::stringstream ss;

    ss << "Statistics\n";
    ss << " Raw Counts\n";
    for(size_t idx = 0; idx < internal::number_counters; idx++) {
        const auto& key = internal::counter_names[idx];
        const auto& counter_value = counters[key];

        ss << "  ";
        ss << std::setfill('.') << std::setw(70) << std::left << key;
        ss << std::setfill('.') << std::setw(8) << std::right << counter_value;
        ss << "\n";
    }
    ss << " Computed\n";
    for(size_t idx = 0; idx < internal::number_computed; idx++) {
        const auto& key = internal::computed_names[idx];
        const auto& computed_value = computed_counters[key];

        ss << "  ";
        ss << std::setfill('.') << std::setw(70) << std::left << key;
        ss << std::setfill('.') << std::setw(8) << std::right
           << std::setprecision(2) << computed_value;
        ss << "\n";
    }

    return ss.str();
}
