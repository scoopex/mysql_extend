/*
 * mod_qcachehitrate.c
 *
 * QCACHEHITRATE
 *
 * vi:set sw=4 ts=4 nu nowrap et:
 */

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"

const char rp_qcachehitrate[] = "QCACHEHITRATE";
const char hs_qcachehitrate[] =
    "Qcache_hits * 100 / (Qcache_hits + Qcache_inserts):\nA high rate indicates that queries in cache are being reused by other threads; a low rate shows either not enough memory allocated to query cache or identical queries are not repeatedly issued to the server.";

void f_qcachehitrate(const char *buffer, size_t len) {
    double qcache_hits, qcache_inserts;

#define VALUE(x) strtoull(get_var(buffer, len, x), NULL, 10)

    qcache_hits = VALUE("Qcache_hits");
    qcache_inserts = VALUE("Qcache_inserts");

    printf("%0.2f", (qcache_hits * 100.0 / (qcache_hits + qcache_inserts)));
}
