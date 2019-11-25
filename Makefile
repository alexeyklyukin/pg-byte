EXTENSION = pg_byte
PG_CONFIG ?= pg_config
DATA = $(wildcard *--*.sql)
PGXS := $(shell $(PG_CONFIG) --pgxs)
MODULE_big = pg_byte
# XXX: use Makefile $(foo:*.o=%.c syntax instead)
OBJS = $(patsubst %.c,%.o,$(wildcard src/*.c))
REGRESS=basic
include $(PGXS)
