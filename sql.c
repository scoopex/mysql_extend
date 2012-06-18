/*
 * sql.c
 *
 * vi:set sw=4 ts=4 nu nowrap et:
 */

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include <string.h>
#include <time.h>
#include <sys/types.h>

#include <mysql.h>

#include "globals.h"

#if MYSQL_VERSION_ID < 51000
#define CAST_OPTIONS_ARG (char *)
#else
#define CAST_OPTIONS_ARG
#endif


MYSQL mysql;

static const char _stmt_status[] = SQL_STMT_STATUS;
static const char _stmt_variables[] = SQL_STMT_VARIABLES;
static const char _stmt_slavestatus[] = SQL_STMT_SLAVESTATUS;

static char *append_str(char *dst, const char *src, size_t len, size_t * bytes_left);

void update_stats(char *buffer, size_t buflen) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    char *ptr;
    size_t bytes_left;

    ptr = buffer;
    bytes_left = buflen;

    if(!mysql_init(&mysql))
        err(EXIT_FAILURE, "Could not initialize MySQL client.");

    mysql_options(&mysql, MYSQL_OPT_COMPRESS, 0);
    mysql_options(&mysql, MYSQL_OPT_CONNECT_TIMEOUT, CAST_OPTIONS_ARG & db_timeout);
    mysql_options(&mysql, MYSQL_READ_DEFAULT_GROUP, progname);
    if(db_config)
        mysql_options(&mysql, MYSQL_READ_DEFAULT_FILE, db_config);
    if(db_section)
        mysql_options(&mysql, MYSQL_READ_DEFAULT_GROUP, db_section);

    if(!mysql_real_connect(&mysql, db_hostname, db_user, db_password, NULL, db_port, NULL, 0))
        errx(EXIT_FAILURE, "%s - Host = %s:%u, User = %s", mysql_error(&mysql), db_hostname, db_port, db_user);

    /* get the global status */
    if(mysql_real_query(&mysql, _stmt_status, sizeof(_stmt_status) - 1))
        errx(EXIT_FAILURE, "%s - Host = %s:%u, User = %s", mysql_error(&mysql), db_hostname, db_port, db_user);
    if((result = mysql_store_result(&mysql)) == NULL)
        errx(EXIT_FAILURE, "%s - Host = %s:%u, User = %s", mysql_error(&mysql), db_hostname, db_port, db_user);
    while((row = mysql_fetch_row(result))) {
        unsigned long *lengths;
        lengths = mysql_fetch_lengths(result);
        ptr = append_str(ptr, row[0], lengths[0], &bytes_left);
        ptr = append_str(ptr, row[1], lengths[1], &bytes_left);
    }
    mysql_free_result(result);

    /* get the global variables */
    if(mysql_real_query(&mysql, _stmt_variables, sizeof(_stmt_variables) - 1))
        errx(EXIT_FAILURE, "%s - Host = %s:%u, User = %s", mysql_error(&mysql), db_hostname, db_port, db_user);
    if((result = mysql_store_result(&mysql)) == NULL)
        errx(EXIT_FAILURE, "%s - Host = %s:%u, User = %s", mysql_error(&mysql), db_hostname, db_port, db_user);
    while((row = mysql_fetch_row(result))) {
        unsigned long *lengths;
        lengths = mysql_fetch_lengths(result);
        ptr = append_str(ptr, row[0], lengths[0], &bytes_left);
        ptr = append_str(ptr, row[1], lengths[1], &bytes_left);
    }
    mysql_free_result(result);

    /* get slave status */
    if(mysql_real_query(&mysql, _stmt_slavestatus, sizeof(_stmt_slavestatus) - 1))
        errx(EXIT_FAILURE, "%s - Host = %s:%u, User = %s", mysql_error(&mysql), db_hostname, db_port, db_user);
    if((result = mysql_store_result(&mysql)) == NULL)
        errx(EXIT_FAILURE, "%s - Host = %s:%u, User = %s", mysql_error(&mysql), db_hostname, db_port, db_user);
    while((row = mysql_fetch_row(result))) {
        unsigned long *lengths;
        unsigned int num_fields;
        unsigned int i;
        MYSQL_FIELD *fields;
        lengths = mysql_fetch_lengths(result);
        num_fields = mysql_num_fields(result);
        fields = mysql_fetch_fields(result);
        for(i = 0; i < num_fields; ++i) {
            if(!strncmp("Slave_IO_Running", fields[i].name, 16) || !strncmp("Slave_SQL_Running", fields[i].name, 17) || !strncmp("Seconds_Behind_Master", fields[i].name, 21)) {
                ptr = append_str(ptr, fields[i].name, strlen(fields[i].name), &bytes_left);
                ptr = append_str(ptr, row[i], lengths[i], &bytes_left);
            }
        }
    }
    mysql_free_result(result);

    mysql_close(&mysql);
}

/*
 * 0x00: 2 bytes len (u_int16_t) (without trailing \0)
 * 0x02: src
 * 0x02 + len: \0
 */
static char *append_str(char *dst, const char *src, size_t len, size_t * bytes_left) {
    u_int16_t *lenptr;
    size_t bytes_needed;

    if(!len || !src) {
        src = "n/a";
        len = 3;
    }

    bytes_needed = sizeof(u_int16_t) + len + 1;

    if(*bytes_left < bytes_needed) {
        errx(EXIT_FAILURE, "No space left in shared memory object");
    }

    *bytes_left -= bytes_needed;

    lenptr = (u_int16_t *) dst;
    *lenptr = len;

    dst += 2;
    memcpy(dst, src, len);

    dst += len;
    *dst = '\0';

    return ++dst;
}

const char *get_var(const char *buffer, size_t buflen, const char *name) {
    size_t offset;
    u_int16_t len;
    const char *str;

    offset = 0;
    len = (u_int16_t) * (buffer + offset);
    str = buffer + offset + 2;
    while(offset < buflen && len) {
        if(!strncmp(str, name, len + 1))
            return (buffer + offset + 5 + len);

        offset += len + 3;
        len = (u_int16_t) * (buffer + offset);
        str = buffer + offset + 2;
    }

    return NULL;
}
