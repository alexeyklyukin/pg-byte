#ifndef PG_BYTE_H
#define PG_BYTE_H

#include "postgres.h"

typedef uint8 pg_byte;

#define PG_BYTE_MAX_LABELS 255
#define PG_RETURN_PG_BYTE(x) return UInt8GetDatum(x);
#define PG_GETARG_PG_BYTE(n) DatumGetUInt8(PG_GETARG_DATUM(n));

Datum pg_byte_in(PG_FUNCTION_ARGS);
Datum pg_byte_out(PG_FUNCTION_ARGS);
Datum pg_byte_send(PG_FUNCTION_ARGS);
Datum pg_byte_(PG_FUNCTION_ARGS);

#endif /* PG_BYTE_H */