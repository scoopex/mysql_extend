/*
 * mod_maxmemory.c
 *
 * MAXMEMORY
 *
 * vi:set sw=4 ts=4 nu nowrap et:
 */

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"

const char rp_maxmemory[] = "MAXMEMORY";
const char hs_maxmemory[] = "no help given yet";

void f_maxmemory(const char *buffer, size_t len) {
    unsigned long long tmp_table_size;
    unsigned long long max_heap_table_size;
    unsigned long long max_tmp_table_size;
    unsigned long long server_buffers;
    unsigned long long per_thread_buffers;
    unsigned long long total_per_thread_buffers;

#define VALUE(x) strtoull(get_var(buffer, len, x), NULL, 10)

    tmp_table_size = VALUE("tmp_table_size");
    max_heap_table_size = VALUE("max_heap_table_size");
    max_tmp_table_size = (tmp_table_size > max_heap_table_size) ? max_heap_table_size : tmp_table_size;

    server_buffers = VALUE("key_buffer_size") + max_tmp_table_size;
    server_buffers += VALUE("innodb_buffer_pool_size");
    server_buffers += VALUE("innodb_additional_mem_pool_size");
    server_buffers += VALUE("innodb_log_buffer_size");
    server_buffers += VALUE("query_cache_size");

    per_thread_buffers = VALUE("read_buffer_size");
    per_thread_buffers += VALUE("read_rnd_buffer_size");
    per_thread_buffers += VALUE("sort_buffer_size");
    per_thread_buffers += VALUE("thread_stack");
    per_thread_buffers += VALUE("join_buffer_size");
    total_per_thread_buffers = per_thread_buffers * VALUE("max_connections");

    printf("%llu", server_buffers + total_per_thread_buffers);
}
