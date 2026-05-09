#!/usr/bin/env python3
"""
Simple opcode codegen script.
Usage: python3 tools/generate_opcodes.py instructions.csv [out.cpp]

CSV format (header): id,mnemonic,value,argCount,derefMask
Example line:
ADD,ADD,0x0,3,0b000

This script outputs a C++ file defining opCodeTable, opCodeDereferenceMap,
mnemonicTable, opCodeMnemonicTable, opCodeValuesTable, registerMap and
registerNameTable based on the CSV.
If an output path is provided, the file is written there; otherwise written to stdout.
"""
import sys
import csv

if len(sys.argv) < 2:
    print("Usage: generate_opcodes.py <instructions.csv> [out.cpp]", file=sys.stderr)
    sys.exit(2)

infile = sys.argv[1]
outfile = sys.argv[2] if len(sys.argv) > 2 else None

ops = []
with open(infile, newline='') as f:
    # support comments starting with # by filtering
    lines = [l for l in f if not l.lstrip().startswith('#')]
    reader = csv.DictReader(lines)
    for row in reader:
        # normalize fields
        ops.append({
            'id': row['id'].strip(),
            'mnemonic': row.get('mnemonic', row['id']).strip(),
            'value': row['value'].strip(),
            'argCount': int(row['argCount'].strip()),
            'derefMask': row.get('derefMask', '0').strip()
        })

# First, try to read register definitions from a canonical CSV (tools/registers.csv)
reg_csv_path = 'tools/registers.csv'
reg_names = []
import os
if os.path.exists(reg_csv_path):
    with open(reg_csv_path, newline='') as rf:
        rreader = csv.DictReader(rf)
        for row in rreader:
            # expect a 'name' column
            name = (row.get('name') or row.get('Name') or '').strip()
            if name:
                reg_names.append(name)
# helper to format
def fmt_hex(v):
    if v.startswith('0x'):
        return v
    if v.startswith('0b'):
        # convert binary to hex
        return hex(int(v,2))
    # otherwise assume decimal
    try:
        return hex(int(v))
    except:
        return v

lines_out = []
append = lines_out.append
append('#include "assembler.h"')
append('#include "opcode.h"')
append('')
append('const std::unordered_map<OpCodeId, OpCode> opCodeTable = {')
for op in ops:
    append(f"    {{OpCodeId::{op['id']}, OpCode{{{fmt_hex(op['value'])}, {op['argCount']}}}}},")
append('};')
append('')
append('const std::unordered_map<OpCodeId, uint8_t> opCodeDereferenceMap = {')
for op in ops:
    append(f"    {{OpCodeId::{op['id']}, {op['derefMask']}}},")
append('};')
append('')
append('const std::unordered_map<std::string, OpCodeId> mnemonicTable = {')
for op in ops:
    append(f"    {{\"{op['mnemonic']}\", OpCodeId::{op['id']}}},")
append('};')
append('')
append('const std::unordered_map<OpCodeId, std::string> opCodeMnemonicTable = {')
for op in ops:
    append(f"    {{OpCodeId::{op['id']}, \"{op['mnemonic']}\"}},")
append('};')
append('')
append('const std::unordered_map<uint16_t, OpCodeId> opCodeValuesTable = {')
for op in ops:
    append(f"    {{{fmt_hex(op['value'])}, OpCodeId::{op['id']}}},")
append('};')
append('')
# Registers are provided by src/register_table.cpp

output = "\n".join(lines_out) + "\n"

if outfile:
    with open(outfile, 'w') as f:
        f.write(output)
else:
    sys.stdout.write(output)
