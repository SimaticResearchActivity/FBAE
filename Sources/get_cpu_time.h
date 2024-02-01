#ifndef _GET_CPU_TIME_H
#define _GET_CPU_TIME_H

/**
 * @brief Returns CPU time (in microseconds).
 * Note: Under Windows, you must have a different an elapsed time of at least 100 milliseconds,
 *       Otherwise values returned by get_cpu_time() are not precise enough.
 * @return CPU time (in microseconds)
 */
unsigned long long get_cpu_time();

#endif /* _GET_CPU_TIME_H */
