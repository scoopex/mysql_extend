/*
 * options.c
 *
 * vi:set sw=4 ts=4 nu nowrap et:
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <err.h>

#include <unistd.h>
#include <sys/types.h>

#include "globals.h"

char *db_hostname = NULL;
char *db_user = NULL;
char *db_password = NULL;
char *db_config = NULL;
char *db_section = NULL;
unsigned short db_port = MYSQL_DEFAULT_PORT;
unsigned int db_timeout = MYSQL_DEFAULT_TIMEOUT;
unsigned int shm_ttl = SHM_TIMETOLIVE;
char *request = NULL;
char *shm_name = NULL;
char *sem_name = NULL;

static void usage(void);
static void help(void);

static struct option longopts[] = {
    {"config", required_argument, NULL, 'c'},
    {"help", no_argument, NULL, 'h'},
    {"password", required_argument, NULL, 'p'},
    {"port", required_argument, NULL, 'P'},
    {"section", required_argument, NULL, 's'},
    {"timeout", required_argument, NULL, 't'},
    {"maxttl", required_argument, NULL, 'm'},
    {"user", required_argument, NULL, 'u'},
    {"version", no_argument, NULL, 1},
    {NULL, 0, NULL, 0}
};

void parse_options(int argc, char *const argv[]) {
    int ch;
    size_t len;

    while((ch = getopt_long(argc, argv, "c:hp:P:s:t:m:u:", longopts, NULL)) != -1) {
        switch (ch) {
            case 0:
                break;

            case 1:            /* version */
                printf("%s v%s\n", progname, MYSQL_EXTEND_VERSION);
                exit(EXIT_SUCCESS);
                /* NOT REACHED */

            case 'c':          /* config */
                free(db_config);
                if((db_config = strdup(optarg)) == NULL)
                    err(EXIT_FAILURE, NULL);
                break;

            case 'h':
                help();
                /* NOT REACHED */

            case 'p':          /* password */
                free(db_password);
                if((db_password = strdup(optarg)) == NULL)
                    err(EXIT_FAILURE, NULL);
                break;

            case 'P':          /* port */
                db_port = strtoul(optarg, NULL, 10);
                if(db_port == 0) {
                    warnx("Parameter port is malformed (1-65535).");
                    usage();
                }
                break;

            case 's':          /* section */
                free(db_section);
                if((db_section = strdup(optarg)) == NULL)
                    err(EXIT_FAILURE, NULL);
                break;

            case 't':          /* timeout */
                db_timeout = strtoul(optarg, NULL, 10);
                break;

            case 'm':          /* shared memory ttl */
                shm_ttl = strtoul(optarg, NULL, 10);
                break;

            case 'u':          /* user */
                free(db_user);
                if((db_user = strdup(optarg)) == NULL)
                    err(EXIT_FAILURE, NULL);
                break;

            default:
                usage();
                /* NOT REACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if(argc != 2) {
        warnx("Wrong number of arguments.");
        usage();
        /* NOT REACHED */
    }

    if((db_hostname = strdup(argv[0])) == NULL)
        err(EXIT_FAILURE, NULL);
    if((request = strdup(argv[1])) == NULL)
        err(EXIT_FAILURE, NULL);

    /* '/var/lock/' + progname + '-' + hostname + ':' + port + '\0' */
    len = 10 + strlen(progname) + 1 + strlen(db_hostname) + 1 + 5 + 1;
    if(len > PATH_MAX)
        errx(EXIT_FAILURE, "hostname too long");

    sem_name = malloc(len);
    if(!sem_name)
        err(EXIT_FAILURE, NULL);
    snprintf(sem_name, len, "/var/lock/%s-%s:%0u", progname, db_hostname, db_port);
    for(len = 10; sem_name[len]; ++len) {
        if(sem_name[len] == '/')
            sem_name[len] = '.';
    }
    shm_name = sem_name + 9;
}

static void usage(void) {
    fprintf(stderr, "Usage: %s [OPTIONS] <HOST> <variable|Status|SPECIAL>\n", progname);
    fprintf(stderr, "Try `%s --help' for more information.\n", progname);
    exit(EXIT_FAILURE);
}

static void help(void) {
    printf("%s v%s\n", progname, MYSQL_EXTEND_VERSION);
    puts("Copyright (c) 2011 dmc digital media center GmbH");
    printf("Usage: %s [OPTIONS] <HOST> variable|Status|SPECIAL\n", progname);
    puts("Options:");
    puts("  -c, --config=name   Name of an extra my.cnf file to read.");
    puts("  -h, --help          Display this help and exit.");
    puts("  -p, --password=name Password to use when connecting to server.");
    puts("                      WARNING: maybe read by `ps auxww'!!!");
    puts("                      Use the my.cnf mechanism (see -c & -s).");
    printf("  -P, --port=#        Port number to use for connection. (default: %d)\n", MYSQL_DEFAULT_PORT);
    puts("  -s, --section=name  Section to read for login credentials. (my.cnf)");
    printf("  -t, --timeout=#     Timeout in seconds for login. (default: %d sec)\n", MYSQL_DEFAULT_TIMEOUT);
    printf("  -m, --maxttl=#     Max lifetime of cached values. (default: %d sec)\n", SHM_TIMETOLIVE);
    puts("  -u, --user=name     User for login if not current user.");
    puts("      --version       Display version information an exit.");
    puts("Arguments:");
    puts("  <HOST>              Connect to host <HOST>.");
    puts("  variable            Name of the variable as given in `SHOW GLOBAL VARIABLES'");
    puts("                      (e.g.: \"innodb_buffer_pool_size\")");
    puts("  Status              Name of the status item as given in `SHOW GLOBAL STATUS'");
    puts("                      (e.g.: \"Connections\" or \"Table_locks_waited\")");
    puts("  SPECIAL             One of these:");
    puts("                      MAXMEMORY: Maximum possible memory usage");
    puts("                      QCACHEHITRATE: Query cache hit rate");
/*
	puts("                      SLOWQUERIES: Percentage of slow queries");
*/
    exit(EXIT_SUCCESS);
}
