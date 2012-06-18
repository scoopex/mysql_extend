#
# Makefile for mysql_extend
#
# vi:set ts=4 nu nowrap:
#

INSTALLDIR=/etc/zabbix/externalscripts

MYSQL_CFLAGS=$(shell mysql_config --cflags)
MYSQL_LDFLAGS=
MYSQL_LDADD=$(shell mysql_config --libs)

# working around SLES11
MYSQL_LDFLAGS+=$(sort $(patsubst -L%/,-L%,$(dir $(filter -L%,$(MYSQL_LDADD)))))

CC=gcc
CFLAGS=	-O2 -fno-strict-aliasing -pipe -march=native \
	-W -Wall -ansi -pedantic -Wbad-function-cast -Wcast-align \
	-Wcast-qual -Wchar-subscripts -Winline \
	-Wmissing-prototypes -Wnested-externs -Wpointer-arith \
	-Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings \
	-std=c99 -D_GNU_SOURCE
LDFLAGS=
LDADD=-lrt

ifeq ($(MAKECMDGOALS),debug)
CFLAGS+=-g
endif

OBJECTS=main.o options.o sql.o mod_maxmemory.o mod_qcachehitrate.o

%.d: %.c
	@echo generating $@
	@$(CC) -MM $< | sed -e 's,:, $@:,' >$@

%.o: %.c
	$(CC) -c $(CFLAGS) $(MYSQL_CFLAGS) -o $@ $<

.PHONY: default all debug
default: mysql_extend
all: default
debug: mysql_extend


mysql_extend: $(OBJECTS)
	$(CC) $(LDFLAGS) $(MYSQL_LDFLAGS) -o $@ $^ $(LDADD) $(MYSQL_LDADD)

ifneq ($(MAKECMDGOALS),clean)
-include $(OBJECTS:.o=.d)
endif

install: mysql_extend
	install -g root -m 0755 -o root -d $(INSTALLDIR)
	install -s -g root -m 0755 -o root mysql_extend $(INSTALLDIR)

.PHONY: indent
indent:
	indent *.[ch]

.PHONY: clean
clean:
	rm -f mysql_extend $(OBJECTS) core *.core *.[ch]~ *.d

