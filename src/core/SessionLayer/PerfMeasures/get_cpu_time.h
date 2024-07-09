#ifndef _FBAE_GET_CPU_TIME_H
#define _FBAE_GET_CPU_TIME_H

namespace fbae::core::SessionLayer::PerfMeasures {

/**
 * @brief Returns CPU time (in microseconds).
 * Note: Under Windows, you must have a different an elapsed time of at least
 * 100 milliseconds, Otherwise values returned by get_cpu_time() are not precise
 * enough.
 * @return CPU time (in microseconds)
 */
unsigned long long get_cpu_time();

}  // namespace fbae::core::SessionLayer::PerfMeasures

#endif /* _FBAE_GET_CPU_TIME_H */