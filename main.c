/*
 * main.c
 *
 * vi:set sw=4 ts=4 nu nowrap et:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "globals.h"

static struct module modules[] = {
    {rp_maxmemory, hs_maxmemory, f_maxmemory},
    {rp_qcachehitrate, hs_qcachehitrate, f_qcachehitrate},
    {NULL, NULL, NULL}
};

const char *progname;

static void alrm_handler(int s) {
    s = s;                      /* quell compiler warnings */
    errx(EXIT_FAILURE, "Timed out while waiting for lock");
}

int main(int argc, char *argv[]) {
    char *ptr;
    int sem_fd;                 /* our "semaphore" */
    int shm_fd;                 /* fd for shared memory segment */
    char *shm_ptr;
    int shm_initialized;        /* flag: 0 not ok, 1 ok */
    time_t t;
    int is_special;             /* flag: 0 no, 1 yes */
    struct sigaction sa;

    /* save basename of argv[0] for later (global) use */
    ptr = argv[0];
    while(*ptr)
        ++ptr;
    while(ptr > argv[0] && *(ptr - 1) != '/')
        --ptr;
    progname = ptr;

    /* parse command line options */
    parse_options(argc, argv);

    /* init lockfile */
    sem_fd = open(sem_name, O_RDWR |    /* open for read and write access */
#ifdef O_CLOEXEC
                  O_CREAT |     /* create if not exist */
                  O_CLOEXEC,    /* close on execute */
#else
                  O_CREAT,
#endif
                  S_IRUSR |     /* user permission: read */
                  S_IWUSR);     /* user permission: write */
    if(sem_fd == -1)
        err(EXIT_FAILURE, "Could not open lockfile `%s'", sem_name);

    /* setup signal handler for timeout */
    sa.sa_handler = alrm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGALRM, &sa, NULL) == -1)
        err(EXIT_FAILURE, "Could not setup signal handler");
    alarm(db_timeout);

    /* get lock (blocking) */
    if(lockf(sem_fd, F_LOCK, 0) == -1)
        err(EXIT_FAILURE, "Error occured while requesting lock");

    /* remove signal handler */
    alarm(0);
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGALRM, &sa, NULL) == -1)
        err(EXIT_FAILURE, "Could not reset signal handler");

    /* get valid fd */
    shm_initialized = FALSE;
    shm_fd = shm_open(shm_name, O_RDWR, 0);
    if(shm_fd < 0) {
        /* got none, maybe error */
        if(errno != ENOENT) {
            /* error condition */
            err(EXIT_FAILURE, "Could not open shared memory object");
        }

        /* object did not exist, so create one */
        shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if(shm_fd < 0) {
            err(EXIT_FAILURE, "Could not create shared memory object");
        }

        /* set size */
        if(ftruncate(shm_fd, SHM_SIZE) < 0) {
            err(EXIT_FAILURE, "Could not set size of shared memory object");
        }

        /* shm is not initialized here */
    }
    else {
        struct stat sb;

        /* success, check for size now */
        if(fstat(shm_fd, &sb) < 0) {
            err(EXIT_FAILURE, "Could not get size of shared memory object");
        }

        if(sb.st_size != SHM_SIZE) {
            /* size mismatch, reinit */
            if(ftruncate(shm_fd, SHM_SIZE) < 0) {
                err(EXIT_FAILURE, "Could not reset size of shared memory object");
            }
        }
        else {
            shm_initialized = TRUE;
        }
    }

    /* map to memory */
    shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shm_ptr == MAP_FAILED)
        err(EXIT_FAILURE, "Could not map shared object to memory");

    /* initialize/update data if needed */
    t = time(NULL) - *(time_t *) shm_ptr;
    if(!shm_initialized || memcmp(shm_ptr + (SHM_SIZE - sizeof(time_t)), shm_ptr, sizeof(time_t)) || t > shm_ttl || t < 0) {
        memset(shm_ptr, 0x00, SHM_SIZE);
        time((time_t *) shm_ptr);
        update_stats(shm_ptr + sizeof(time_t), SHM_SIZE - (2 * sizeof(time_t)));
        memcpy(shm_ptr + (SHM_SIZE - sizeof(time_t)), shm_ptr, sizeof(time_t));
    }

    /* check if variable/Status or SPECIAL */
    is_special = 1;
    for(ptr = request; *ptr; ptr++) {
        if(islower(*ptr)) {
            is_special = 0;
            break;
        }
    }

    if(is_special) {
        struct module *m;

        /* find matching module */
        for(m = &modules[0]; m && m->rp; m++) {
            if(!strcmp(request, m->rp)) {
                break;
            }
        }

        if(m && m->rp) {
            m->f(shm_ptr + sizeof(time_t), SHM_SIZE - (2 * sizeof(time_t)));
        }
        else {
            fputs("unknown", stdout);
        }
    }
    else {
        const char *v;

        v = get_var(shm_ptr + sizeof(time_t), SHM_SIZE - (2 * sizeof(time_t)), request);
        if(!v) {
            fputs("unknown", stdout);
        }
        else {
            fputs(v, stdout);
        }
    }

    /* unmap */
    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);

    /* release lock */
    lockf(sem_fd, F_ULOCK, 0);

    /* cleanup */
    close(sem_fd);

    exit(EXIT_SUCCESS);
}
