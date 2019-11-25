-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_byte" to load this file. \quit

CREATE TYPE pg_byte;
CREATE TABLE pg_byte_labels(id smallserial primary key CHECK (id >= 0 AND id < 255), label name UNIQUE NOT NULL);

SELECT pg_catalog.pg_extension_config_dump('pg_byte_labels', '');
SELECT pg_catalog.pg_extension_config_dump('pg_byte_labels_id_seq', '');

-- regular in/out functions required for every data type
CREATE FUNCTION pg_byte_in(cstring)
    RETURNS pg_byte
    AS 'pg_byte.so'
    LANGUAGE C IMMUTABLE STRICT SET search_path = @extschema@;

CREATE FUNCTION pg_byte_out(pg_byte)
    RETURNS cstring
    AS 'pg_byte.so'
    LANGUAGE C IMMUTABLE STRICT SET search_path = @extschema@;

CREATE FUNCTION pg_byte_recv(internal)
    RETURNS pg_byte
    AS 'pg_byte.so'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION pg_byte_send(pg_byte)
    RETURNS bytea
    AS 'pg_byte.so'
    LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE pg_byte (
    input=pg_byte_in,
    output=pg_byte_out,
    receive=pg_byte_recv,
    send=pg_byte_send,
    like="char"
);

-- populate initial values into the byte type
-- each element will be assigned an unsigned int value
-- from 0 to 255 depending on its position in the array
CREATE FUNCTION pg_byte_populate_values(p_values name[])
    RETURNS void
    AS 'pg_byte.so'
    LANGUAGE C STRICT SET search_path = @extschema@;

-- return the label associated with the given internal id
CREATE FUNCTION pg_byte_id_to_label(id smallint)
    RETURNS text
    AS 'pg_byte.so'
    LANGUAGE C STRICT SET search_path = @extschema@;