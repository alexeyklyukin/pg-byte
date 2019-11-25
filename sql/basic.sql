CREATE SCHEMA IF NOT EXISTS test;
CREATE EXTENSION pg_byte SCHEMA test;
SELECT test.pg_byte_populate_values(ARRAY['one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine', 'ten']::name[]);
CREATE TABLE test(id smallint primary key, data test.pg_byte);
INSERT INTO test SELECT id::smallint, test.pg_byte_id_to_label(id::smallint)::test.pg_byte FROM generate_series(1, 10) id;
SELECT * FROM test;
