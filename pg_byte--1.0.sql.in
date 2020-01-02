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

CREATE FUNCTION pg_byte_eq(pg_byte, pg_byte)
    RETURNS bool
    AS 'chareq'
    LANGUAGE internal immutable strict;

CREATE FUNCTION pg_byte_ne(pg_byte, pg_byte)
    RETURNS bool
    AS 'charne'
    LANGUAGE internal immutable strict;

CREATE FUNCTION pg_byte_lt(pg_byte, pg_byte)
    RETURNS bool
    AS 'charlt'
    LANGUAGE internal immutable strict;

CREATE FUNCTION pg_byte_le(pg_byte, pg_byte)
    RETURNS bool
    AS 'charle'
    LANGUAGE internal immutable strict;

CREATE FUNCTION pg_byte_gt(pg_byte, pg_byte)
    RETURNS bool
    AS 'chargt'
    LANGUAGE internal immutable strict;

CREATE FUNCTION pg_byte_ge(pg_byte, pg_byte)
    RETURNS bool
    AS 'charge'
    LANGUAGE internal immutable strict;

CREATE FUNCTION pg_byte_cmp(pg_byte, pg_byte)
    RETURNS integer
    AS 'btcharcmp'
    LANGUAGE internal immutable strict;

CREATE FUNCTION hash_pg_byte(pg_byte)
    RETURNS integer
    AS 'hashchar'
    LANGUAGE internal immutable strict;

CREATE OPERATOR = (
    LEFTARG = pg_byte,
    RIGHTARG = pg_byte,
    PROCEDURE = pg_byte_eq,
    COMMUTATOR = '=',
    NEGATOR = '<>',
    RESTRICT = eqsel,
    JOIN = eqjoinsel
);

CREATE OPERATOR <> (
    LEFTARG = pg_byte,
    RIGHTARG = pg_byte,
    PROCEDURE = pg_byte_ne,
    COMMUTATOR = '<>',
    NEGATOR = '=',
    RESTRICT = neqsel,
    JOIN = neqjoinsel
);

CREATE OPERATOR < (
    LEFTARG = pg_byte,
    RIGHTARG = pg_byte,
    PROCEDURE = pg_byte_lt,
    COMMUTATOR = '>',
    NEGATOR = '>=',
    RESTRICT = scalarltsel,
    JOIN = scalarltjoinsel
);

CREATE OPERATOR <= (
    LEFTARG = pg_byte,
    RIGHTARG = pg_byte,
    PROCEDURE = pg_byte_le,
    COMMUTATOR = '>=',
    NEGATOR = '>',
    RESTRICT = scalarltsel,
    JOIN = scalarltjoinsel
);

CREATE OPERATOR > (
    LEFTARG = pg_byte,
    RIGHTARG = pg_byte,
    PROCEDURE = pg_byte_gt,
    COMMUTATOR = '<',
    NEGATOR = '<=',
    RESTRICT = scalargtsel,
    JOIN = scalargtjoinsel
);

CREATE OPERATOR >= (
    LEFTARG = pg_byte,
    RIGHTARG = pg_byte,
    PROCEDURE = pg_byte_ge,
    COMMUTATOR = '<=',
    NEGATOR = '<',
    RESTRICT = scalargtsel,
    JOIN = scalargtjoinsel
);

DO $body$
DECLARE pg_version integer;
BEGIN
    SELECT current_setting('server_version_num') INTO STRICT pg_version;
    IF pg_version > 90600 THEN
        EXECUTE $$ ALTER FUNCTION pg_byte_in(cstring) PARALLEL SAFE $$;
        EXECUTE $$ ALTER FUNCTION pg_byte_out(pg_byte) PARALLEL SAFE $$;
        EXECUTE $$ ALTER FUNCTION pg_byte_send(pg_byte) PARALLEL SAFE $$;
        EXECUTE $$ ALTER FUNCTION pg_byte_recv(internal) PARALLEL SAFE $$;
        EXECUTE $$ ALTER FUNCTION pg_byte_eq(pg_byte, pg_byte) PARALLEL SAFE $$;
        EXECUTE $$ ALTER FUNCTION pg_byte_ne(pg_byte, pg_byte) PARALLEL SAFE $$;
        EXECUTE $$ ALTER FUNCTION pg_byte_lt(pg_byte, pg_byte) PARALLEL SAFE $$;
        EXECUTE $$ ALTER FUNCTION pg_byte_le(pg_byte, pg_byte) PARALLEL SAFE $$;
        EXECUTE $$ ALTER FUNCTION pg_byte_gt(pg_byte, pg_byte) PARALLEL SAFE $$;
        EXECUTE $$ ALTER FUNCTION pg_byte_ge(pg_byte, pg_byte) PARALLEL SAFE $$;
        EXECUTE $$ ALTER FUNCTION pg_byte_cmp(pg_byte, pg_byte) PARALLEL SAFE $$;
        EXECUTE $$ ALTER FUNCTION hash_pg_byte(pg_byte) PARALLEL SAFE $$;
    END IF;

    IF pg_version >= 110000 THEN
        EXECUTE $$ ALTER OPERATOR <= (pg_byte, pg_byte)
            SET (RESTRICT = scalarlesel, JOIN = scalarlejoinsel); $$;
        EXECUTE $$ ALTER OPERATOR >= (pg_byte, pg_byte)
            SET (RESTRICT = scalargesel, JOIN = scalargejoinsel); $$;
    END IF;
END;
$body$;


CREATE OPERATOR CLASS btree_pg_byte_ops
DEFAULT FOR TYPE pg_byte USING btree
AS
    OPERATOR    1   <,
    OPERATOR    2   <=,
    OPERATOR    3   =,
    OPERATOR    4   >=,
    OPERATOR    5   >,
    FUNCTION    1   pg_byte_cmp(pg_byte, pg_byte);

CREATE OPERATOR CLASS hash_pg_byte_ops
DEFAULT FOR TYPE pg_byte USING hash
AS
    OPERATOR    1   =,
    FUNCTION    1   hash_pg_byte(pg_byte);

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