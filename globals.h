/*
 * globals.h
 *
 * vi:set sw=4 ts=4 nu nowrap et:
 */

#ifndef GLOBALS_H
#define GLOBALS_H

/* some defaults */
#define MYSQL_DEFAULT_PORT 3306
#define MYSQL_DEFAULT_TIMEOUT 5

/* size of shared memory segment */
/* #define SHM_SIZE 32768 */
#define SHM_SIZE 65536

/* time to live in seconds for shared memory segment */
#define SHM_TIMETOLIVE 60

/* sql statements */
#define SQL_STMT_VARIABLES "SHOW GLOBAL VARIABLES"
#define SQL_STMT_STATUS "SHOW GLOBAL STATUS"
/* Fields needed: Slave_IO_Running,Slave_SQL_Running,Seconds_Behind_Master */
#define SQL_STMT_SLAVESTATUS "SHOW SLAVE STATUS"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/**********
 * main.c *
 **********/

/* saved basename of argv[0] */
extern const char *progname;

/*************
 * options.c *
 *************/

/* db credentials */
extern char *db_hostname;       /* mysql host */
extern char *db_user;           /* mysql username */
extern char *db_password;       /* mysql password */
extern char *db_config;         /* name of an extra my.cnf file to read */
extern char *db_section;        /* name of a section in any my.cnf to read */
extern unsigned short db_port;  /* mysql port */
extern unsigned int db_timeout; /* login timeout */
extern unsigned int shm_ttl;    /* shared memory ttl */

/* misc */
extern char *request;           /* the variable|Status|SPECIAL thing */
extern char *sem_name;          /* name for lockfile/semaphore */
extern char *shm_name;          /* name for shm segment (just a pointer into sem_name) */

/* parse_options()
 * parses command line options and arguments
 */
void parse_options(int argc, char *const argv[]);

/*********
 * sql.c *
 *********/

/* update_stats
 * gets status and variables from mysql server
 * and puts it into buffer (which is normally
 * the shared memory object
 */
void update_stats(char *buffer, size_t buflen);

/* get_var
 * searches the variable by name or value in the buffer
 * (which is of size buflen) and returns a pointer to
 * the NULL terminated value
 */
const char *get_var(const char *buffer, size_t buflen, const char *name);

/***********
 * modules *
 ***********/

struct module {
    const char *rp;
    const char *hs;
    void (*f) (const char *, size_t);
};

/* mod_maxmemory.c */
extern const char rp_maxmemory[];
extern const char hs_maxmemory[];
void f_maxmemory(const char *, size_t);

/* mod_qcachehitrate.c */
extern const char rp_qcachehitrate[];
extern const char hs_qcachehitrate[];
void f_qcachehitrate(const char *, size_t);

#endif                          /* GLOBALS_H */
