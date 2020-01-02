TEMPLATE := pg_byte
TEMPLATE_UPPER := $(shell echo ${EXTENSION} | tr '[:lower:]' '[:upper:]')
EXTENSION ?= ${TEMPLATE}
EXTENSION := $(shell echo ${EXTENSION} | tr '[:upper:]' '[:lower:]')
EXTENSION_UPPER := $(shell echo ${EXTENSION} | tr '[:lower:]' '[:upper:]')

PG_CONFIG ?= pg_config
DATA_built = $(patsubst ${TEMPLATE}--%.sql.in, ${EXTENSION}--%.sql, $(wildcard pg_byte--*.sql.in))
TESTS_built=$(patsubst %.sql.in,%.sql,$(wildcard sql/*.sql.in))
EXPECTED_built=$(patsubst %.out.in,%.out,$(wildcard expected/*.out.in))
PGXS := $(shell $(PG_CONFIG) --pgxs)
MODULE_big := ${EXTENSION}
OBJS := $(patsubst %.c.in,%.o,$(wildcard src/*.c.in))
REGRESS=create basic operators btree drop

HEADERS := $(patsubst %.h.in,%.h,$(wildcard src/*.h.in))
SOURCES := $(patsubst %.c.in,%.c,$(wildcard src/*.c.in))
include $(PGXS)

$(EXTENSION).control: ${TEMPLATE}.control.in
	sed -e 's/pg_byte/${EXTENSION}/g' -e 's/PG_BYTE/${EXTENSION_UPPER}/g'  $< > $@

$(DATA_built): ${EXTENSION}--%.sql: ${TEMPLATE}--%.sql.in
	sed -e 's/pg_byte/${EXTENSION}/g' -e 's/PG_BYTE/${EXTENSION_UPPER}/g'  $< > $@

$(DATA_built): $(TESTS_built) $(EXPECTED_built)

$(OBJS): $(HEADERS) $(SOURCES)

$(HEADERS): %.h: %.h.in
	sed -e 's/pg_byte/${EXTENSION}/g' -e 's/PG_BYTE/${EXTENSION_UPPER}/g'  $< > $@

$(SOURCES): $(HEADERS)

$(SOURCES): %.c: %.c.in
	sed -e 's/pg_byte/${EXTENSION}/g' -e 's/PG_BYTE/${EXTENSION_UPPER}/g'  $< > $@

$(TESTS_built): %.sql: %.sql.in
	sed -e 's/pg_byte/${EXTENSION}/g' -e 's/PG_BYTE/${EXTENSION_UPPER}/g'  $< > $@

$(EXPECTED_built): %.out: %.out.in
	sed -e 's/pg_byte/${EXTENSION}/g' -e 's/PG_BYTE/${EXTENSION_UPPER}/g'  $< > $@


clean: clean-generated

clean-generated: clean-control clean-sources clean-sql clean-expected

clean-control:
	rm -f *.control

clean-sources:
	rm -f src/*.{c,h}

clean-sql:
	rm -f *.sql sql/*.sql

clean-expected:
	rm -f expected/*.out
