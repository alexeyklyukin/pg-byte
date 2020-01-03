#!env python3
import sys

template="""
#include "core.h"

char *pg_pbyte_to_string(pg_byte val) {{
{body_pg_pbyte_to_string}
   return "";
}}

pg_byte string_to_pg_byte(char *label) {{
{body_string_to_pg_byte}
    return 0;
}}
"""

def make_pg_byte_to_string(id_to_label):
    template = """    switch(pg_byte)
    {{
    {case_block}
       default: elog(ERROR, "invalid input value: %d", pg_byte);
    }}
    """
    case_block_template = """   case {id}: return "{label}";
    """
    case_block = []
    ids = sorted(id_to_label.keys())
    for id_int in ids:
        case_block.append(case_block_template.format(id=id_int, label=id_to_label[id_int]))
    return template.format(case_block=("".join(case_block).rstrip(' \n')))


def make_string_to_pg_byte(id_to_label):
    template ="""{strcmp_block}
    elog(ERROR, "invalid input label: %s", label);
    """

    strcmp_template = """    if (strcmp(label, "{label}") == 0) {{ return {id}; }}\n"""
    strcmp_block = []
    ids = sorted(id_to_label.keys())
    for id_int in ids:
        strcmp_block.append(strcmp_template.format(label=id_to_label[id_int], id=id_int))
    return template.format(strcmp_block = "".join(strcmp_block))

def main():
    label_to_id = {}
    id_to_label = {}
    try:
        fp=sys.stdin
        next_id = 0
        for line in fp:
            fields = [x.strip() for x in line.split('=', 2)]
            label, id_text = fields if len(fields) == 2 else [fields[0], next_id]
            id_int = int(id_text)
            if id_int > 255:
                print(f"ID for label {label} exceeds 255 ({id_text})")
                return 1;
            label_to_id[label] = id_int
            id_to_label[id_int] = label
            next_id = int(id_int) + 1
    except ValueError as e:
        print(f"Errornous ID: {e}")
        return 2

    if len(label_to_id) == 0:
        print(f"Input doesn't define any labels")

    body_pg_pbyte_to_string = make_pg_byte_to_string(id_to_label)
    body_string_to_pg_byte = make_string_to_pg_byte(id_to_label)
    print(template.format(body_pg_pbyte_to_string=body_pg_pbyte_to_string,
                          body_string_to_pg_byte=body_string_to_pg_byte))


if __name__ == '__main__':
    main()