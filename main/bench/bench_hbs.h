/*
 * Hash-based signature benchmark (SLH-DSA-SHA2-128s).
 *
 * Measurement harness only -- compiled in solely when CONFIG_KERN_BENCH_HBS is
 * set, and never part of a release build. It touches no wallet state, no user
 * keys and no storage: it runs NIST ACVP known-answer tests, then times the
 * reference implementation over throwaway keys.
 */

#ifndef KERN_BENCH_HBS_H
#define KERN_BENCH_HBS_H

/*
 * Spawn the benchmark task. Returns immediately; the task prints its results
 * to the console and then deletes itself.
 */
void bench_hbs_start(void);

#endif // KERN_BENCH_HBS_H
