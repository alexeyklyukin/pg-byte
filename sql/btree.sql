\pset format unaligned
TRUNCATE TABLE test;
CREATE INDEX ON test(data);
SET enable_seqscan to 'off';
INSERT INTO test SELECT id, 'one' FROM generate_series(1, 5000) id;
INSERT INTO test SELECT id, 'two' FROM generate_series(5001, 5002) id;
INSERT INTO test SELECT id, 'three' FROM generate_series(5003, 5004) id;
INSERT INTO test SELECT id, 'four' FROM generate_series(5005, 9999) id;
ANALYZE test;
EXPLAIN (ANALYZE, COSTS FALSE, TIMING FALSE, SUMMARY FALSE) SELECT id FROM test WHERE data = 'two';
EXPLAIN (ANALYZE, COSTS FALSE, TIMING FALSE, SUMMARY FALSE) SELECT id FROM test WHERE data between 'three' and 'four';
EXPLAIN (ANALYZE, COSTS FALSE, TIMING FALSE, SUMMARY FALSE) SELECT id FROM test WHERE data < 'two';
EXPLAIN (ANALYZE, COSTS FALSE, TIMING FALSE, SUMMARY FALSE) SELECT id FROM test WHERE data > 'three' and data < 'four';
EXPLAIN (ANALYZE, COSTS FALSE, TIMING FALSE, SUMMARY FALSE) SELECT id FROM test WHERE data > 'four';
DROP TABLE test;
RESET enable_seqscan;