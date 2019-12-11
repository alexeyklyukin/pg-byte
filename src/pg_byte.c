#include "postgres.h"

#include "catalog/pg_type_d.h"
#include "executor/spi.h"
#include "libpq/pqformat.h"
#include "utils/memutils.h"

#include "pg_byte.h"

PG_MODULE_MAGIC;

static HTAB         *labelsHash = NULL;
static char         **labels = NULL;
static MemoryContext pgByteCtx;

static void allocate_and_populate_caches();
static void destroy_caches();
static void _populate_labels_cache_and_labels();
static char * pg_byte_id_to_label_internal(int16 id);
void show_labels_cache(void);
void _PG_init(void);


typedef struct pgbyte_hashentry {
    char key[NAMEDATALEN];
    pg_byte value;
} pgbyte_hashentry;


void _PG_init(void)
{
    /* Figure out the proper values here */
    pgByteCtx = AllocSetContextCreateExtended(CacheMemoryContext,
                                              "PG Byte Context",
                                              16384,
                                              16384,
                                              32768);
    allocate_and_populate_caches();
}

/* pg_byte_in
 *
 *  input function for pg_byte type
 *
 *  Since labels are not hardcoded, but defined in the database, this function
 *  is a bit more involved than just parsing strings. Namely, it looks up the
 *  input value in the hash of the label to ids; the cache is stored in our own
 *  memory context, a child of CacheMemoryContext.
 */
PG_FUNCTION_INFO_V1(pg_byte_in);
Datum
pg_byte_in(PG_FUNCTION_ARGS)
{
    pgbyte_hashentry *entry;
    char    *name = PG_GETARG_CSTRING(0);

    Assert(labels != NULL && labelsHash != NULL);

    if ((entry = hash_search(labelsHash, name, HASH_FIND, NULL)) == NULL)
        ereport(ERROR, (errmsg("invalid input value for type pg_byte: %s", name)));

    PG_RETURN_PG_BYTE(entry->value)
}

PG_FUNCTION_INFO_V1(pg_byte_out);

Datum
pg_byte_out(PG_FUNCTION_ARGS)
{
    char        *label;
    pg_byte     id = PG_GETARG_PG_BYTE(0);

    Assert(labels != NULL && labelsHash != NULL);

    label = pg_byte_id_to_label_internal(id);
    if (label == NULL)
        ereport(ERROR, (errmsg("invalid output value for type pg_byte: %d", id)));
    PG_RETURN_CSTRING(label);
}

PG_FUNCTION_INFO_V1(pg_byte_send);

Datum
pg_byte_send(PG_FUNCTION_ARGS)
{
    StringInfoData  buf;
    pg_byte         id = PG_GETARG_PG_BYTE(0);

    pq_begintypsend(&buf);
    pq_sendint8(&buf, id);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(pg_byte_recv);

Datum
pg_byte_recv(PG_FUNCTION_ARGS)
{
    pg_byte         result;
    StringInfo      buf = (StringInfo) PG_GETARG_POINTER(0);

    result = pq_getmsgint(buf, 1);

    PG_RETURN_PG_BYTE(result);
}

/*
 * Replace the values in the labels table with
 * those supplied as arguments and re-populate the labels cache
 */
PG_FUNCTION_INFO_V1(pg_byte_populate_values);

Datum
pg_byte_populate_values(PG_FUNCTION_ARGS)
{
    int         ret;
    Datum       labels = PG_GETARG_DATUM(0);
    Oid         argtypes[] = {NAMEARRAYOID};
    Datum       values[] = {labels};
    char        *truncate_sql = "TRUNCATE pg_byte_labels RESTART IDENTITY";
    char        *insert_sql =   "INSERT INTO pg_byte_labels(label) SELECT unnest($1)";

    SPI_connect();

    ret = SPI_exec(truncate_sql, 0);
    if (ret != SPI_OK_UTILITY)
       ereport(ERROR, (errmsg("could not execute %s: %d", truncate_sql, ret)));

    ret = SPI_execute_with_args(insert_sql, 1, argtypes, values, NULL, false, 0);
    if (ret != SPI_OK_INSERT)
       ereport(ERROR, (errmsg("could not execute %s: %d", insert_sql, ret)));

    SPI_finish();

    /* now repopulate the cache with the new data */
    destroy_caches();
    allocate_and_populate_caches();

    PG_RETURN_VOID();
}

/*
 * return a label corresponding to the given smallint id.
 *
 */
PG_FUNCTION_INFO_V1(pg_byte_id_to_label);

Datum
pg_byte_id_to_label(PG_FUNCTION_ARGS)
{
    uint32       label_len;
    text        *label_text;
    char        *label;

    int16    id  = PG_GETARG_INT16(0);

    if (id < 0 || id > 255)
        ereport(ERROR, (errmsg("invalid id %d", id),
                        errhint("id must be in the range from 0 to 255")));

    Assert(labels != NULL && labelsHash != NULL);

    label = pg_byte_id_to_label_internal(id);
    if (label == NULL)
        ereport(ERROR, (errmsg("no matching label for an id: %d", id)));

    label_len = strlen(label);

    label_text = (text *)palloc0(label_len + VARHDRSZ);
    SET_VARSIZE(label_text, label_len + VARHDRSZ);
    memcpy((void *)VARDATA(label_text), label, label_len);

    PG_RETURN_TEXT_P(label_text);
}

static char * pg_byte_id_to_label_internal(int16 id)
{
    char    *label;

    label = labels[id];
    return label;
}

static void allocate_and_populate_caches()
{
    HASHCTL         ctl;

    Assert(labelsHash == NULL && labels == NULL);

    // Create or own memory context, a child of CacheMemoryContext;

    ctl.keysize = NAMEDATALEN;
    ctl.entrysize = sizeof(pgbyte_hashentry);
    ctl.hcxt = pgByteCtx;

    labelsHash = hash_create("pg_byte label ids hash", 256,
                                &ctl, HASH_ELEM | HASH_CONTEXT);

    labels = MemoryContextAllocZero(pgByteCtx, PG_BYTE_MAX_LABELS * sizeof(char *));
    _populate_labels_cache_and_labels();
}

static void destroy_caches()
{
    if (labelsHash == NULL || labels == NULL)
    {
        Assert(labelsHash == NULL && labels == NULL);
        return;
    }
    MemoryContextReset(pgByteCtx);

    labels = NULL;
    labelsHash = NULL;
}

/* populate labels to IDs hash by reading pg_byte_labels database table */
static void _populate_labels_cache_and_labels()
{
    int         ret,
                processed;

    const char *sql = "SELECT id, label FROM pg_byte_labels ORDER BY id";

    Assert(labelsHash != NULL && labels != NULL);

    SPI_connect();
    /*
     * read-only = false since we need to see the potential  of the previous
     * truncate/insert command.
     */
    ret = SPI_execute(sql, false, 0);
    if (ret != SPI_OK_SELECT)
        ereport(ERROR, (errmsg("could not execute %s: %d", sql, ret)));


    processed = SPI_processed;
    if (processed > 0 && SPI_tuptable != NULL)
    {
        TupleDesc tupdesc = SPI_tuptable->tupdesc;
        SPITupleTable *tuptable = SPI_tuptable;
        Assert(tupdesc->natts == 2);

        for (int i = 0; i < processed; i++)
        {
            char                    *label;
            uint8                   id;
            pgbyte_hashentry       *entry;
            HeapTuple               tuple;
            bool                    isnull,
                                    found;

            tuple = tuptable->vals[i];

            id = DatumGetUInt8(SPI_getbinval(tuple, tupdesc, 1, &isnull));
            Assert(!isnull);

            label = SPI_getvalue(tuple, tupdesc, 2);
            entry = (pgbyte_hashentry *)hash_search(labelsHash, label, HASH_ENTER, &found);
            /*
             * this should be a new entry, since there should be no
             * duplicate lables in the target table.
             */
            Assert(!found);
            entry->value = id;
            /* add it to the id -> label mapping as well */
            labels[id] = MemoryContextStrdup(pgByteCtx, label);
        }
    }
    SPI_finish();
}

void show_labels_cache(void)
{
    ereport(DEBUG1, (errmsg("labels cache contents")));
    Assert(labelsHash != NULL && labels != NULL);
    for (int i = 0; i < 255; i++)
        if (labels[i] != NULL)
            ereport(DEBUG1, (errmsg("labels cache %d -> %s", i, labels[i])));
}