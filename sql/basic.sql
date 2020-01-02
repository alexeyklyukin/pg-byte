\pset format unaligned
SELECT pg_byte_populate_values(ARRAY['one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine', 'ten']::name[]);
CREATE TABLE test(id integer primary key, data pg_byte);
INSERT INTO test SELECT id, pg_byte_id_to_label(id::smallint)::pg_byte FROM generate_series(1, 10) id;
SELECT * FROM test;
