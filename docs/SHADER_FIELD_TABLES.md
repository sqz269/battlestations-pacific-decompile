# Shader VertexInput and Interpolators conversion

Read-only assembly-backed investigation, 2026-09-09. Client.verify() confirmed
project bsp and program /battlestationspacific.exe before each batch. Raw
exports remain ignored under exports/bsp/shader_descriptor. No names, metadata,
C++, installation files or Ghidra state were changed. This is not runtime
validation. Descriptive function names are proposals.

## Outer table contract

00b419b0 takes three stack arguments: Shader Lua reference, output pointer-array
header, native field-name string; it ends RET Ch. It looks up the named field
through00b68100 and checks Lua table type through00b661b0. Missing/non-table
fields leave the output array unchanged; the helper does not clear existing
entries. Shader reader00b43b00 invokes it for VertexInput (descriptor+D0h) and
Interpolators (+DCh).

When the field is a table,00b67080 begins iteration and00b67190 advances it.
Both ultimately call00a683a0, the embedded Lua next operation. The outer loop
calls00b573f0 on **every value**, without filtering the key and without a
prior inner-table type check. It appends each resulting pointer to the output
array; it does not sort. Capacity grows to max(oldCapacity+5,10). Allocation
failure behavior is not a safe host-interface contract.

## Iteration is lua_next, not indexed lookup

00b67080 resets key/value wrappers, pushes nil, calls00a683a0, then holds the
returned key and value.00b67190 releases the previous value, retains/copies the
previous key as necessary, and calls00a683a0 again.00a683a0 calls table-next
00a6ef60. The latter scans nonnil array slots in ascending index order, then
hash nodes in storage order. It does not establish numeric sorting for hash
keys. Named/hash-key order can depend on table construction and implementation.

Consequently an adapter should use its real Lua5.1 lua_next API. Do not use
ipairs, lua_objlen plus rawgeti, sort keys, or assume iteration stops at a hole.
For the installed dense array constructors, ordinal order matches the written
array order. That observation does not generalize to sparse/hash tables.

## Inner field conversion

00b573f0 is called with ECX=global0108fe90 and one stack Lua value reference;
the body does not use incoming ECX before replacing it with local references.
It returns an allocated 1Ch-byte field pointer in EAX and ends RET4. The
provided value must be iterable as a table for defined success; this routine
has no preliminary table type guard.

The routine uses the same lua_next iteration. It filters each **key** through
00b66a60. Accepted keys are counted from ordinal0; the actual numeric key
value is never used as a destination index. Nonaccepted keys are skipped and
do not increment that ordinal. More than five accepted entries are iterated
but their values are ignored.

| Accepted ordinal | Native result | Value conversion |
|---:|---|---|
| 0 | Name string at +0h length/+4h pointer | 00b685c0(default "undef"): only Lua string type4 accepted; otherwise use "undef"; C-string copy truncates at embedded NUL |
| 1 | Type enum at +8h | 00b66290: lua_tonumber coercion, float32 spill, truncate to signed integer |
| 2 | Component count at +Ch | 00b66380(default0): only Lua number type3 accepted; float32 spill and integer truncation; other types0 |
| 3 | Semantic enum at +14h | Same coercing00b66290 as type |
| 4 | Semantic index at +18h | Same strict-number00b66380(default0) as count |

Offset+10h is the component-mask slot in the existing ShaderField projection
and is always assigned zero (00b5756d, after XOR EDI,EDI at00b5751d).
Override the host ShaderField default0Fh when extracting these records. No
enum range, component-count range, uniqueness, or name validation occurs here.

Initial name is an empty string and semantic index is zero. Type, component
count and semantic locals are **uninitialized** until their ordinals are
visited (assembly writes stack+14h/+18h/+1Ch only in corresponding branches).
Thus a table with fewer than four accepted entries has no safe deterministic
native result for the required fields. Do not invent native defaults for
missing fields. A host adapter should return an explicitly unsupported or
malformed result for that case and non-table elements. That is an adapter
boundary, not a recovered native error policy. Four accepted entries are
sufficient; the absent fifth retains index0 as used by debug Interpolators.

## Exact key eligibility and numeric boundaries

00b66a60 requires Lua typeNUMBER. It obtains a double through00a67770,
spills to float32 at00b66a94, then performs CVTTSS2SI signed32 truncation.
It compares that float32 value to the resulting signed integer using x87
FUCOMIP and an ordered-equality flag test. Acceptance is equivalent, for
ordinary representable numbers, to:

```text
key has exact Lua number type
k = float32(lua_tonumber(key))
i = x86 signed32 truncation of k
accept only ordered equality k == i
```

There is no positive-key requirement. Numeric strings are rejected as keys.
A fractional double that rounds to an integral float32 can pass. NaN fails the
ordered test. Infinity and out-of-range conversions do not satisfy equality;
-2147483648 is the representable signed32 lower endpoint and can pass.
Do not cast out-of-range floats in portable C++: either use checked conversion
or an explicitly documented malformed-input boundary.

00a67770 is the embedded lua_tonumber operation: numeric values load a double;
other values pass through conversion00a6d190 and failure returns zero.
00b66290 always calls this coercion, rounds to float32 with FSTP/FLD, and
tailcalls00bf7420.00b66380 performs a typeNUMBER guard before the same
float32 conversion, using supplied default0 when the guard fails. Thus a
numeric string can supply type/semantic but becomes zero for count/index.
Boolean values are not native Lua numbers and fail coercion to zero.

00bf7420 selects an SSE conversion path when0109eea4 is nonzero: spill as
double then CVTTSD2SI into EAX. The alternate CRT fallback at00bf7456 was
not reconstructed in this bounded pass. Ordinary finite signed32-range
truncation is established; exceptional integer-conversion parity should not
be claimed without checking that fallback and the live selector.

## Installed debug descriptor result

shaderfx/common/debugshader.shfx constructs dense arrays using constants from
shaderfx/dx9_lua.inc: FLOAT=0, POSITION=0, COLOR=1.

| Table / order | Name | Type | Count | +10h | Semantic | Index |
|---|---|---:|---:|---:|---:|---:|
| VertexInput 0 | Position | 0 | 4 | 0 | 0 | 0 |
| VertexInput 1 | Color | 0 | 4 | 0 | 1 | 0 |
| Interpolators 0 | Color | 0 | 4 | 0 | 1 | 0 |

These are expected extraction values grounded in installed text and native
conversion, not a newly executed native fixture. A minimal real-Lua adapter
should preserve outer lua_next order, inner key filtering and ordinal roles,
strict versus coercing conversions, float32 rounding, and the fifth-field
default. Safe rejection of malformed shapes must be identified separately
from ordinary native valid-input semantics.

Follow-up: the parent integrated stock Lua5.1.1 execution and the recovered field
conversion rules for the installed debug/dummy fixture. Real evaluated strings
and fields now feed source generation and pixel readback. See
`SHADER_LUA_ADAPTER.md` for implemented scope, adapter differences and remaining
full-loader work.

## Correction from docs/NATIVE_SHADER_FIELD_READER.md

Full B573F0/B419B0 readers now allocate actual1Ch records with native8h names, append actual pointers and clean them through descriptor D0/DC ownership. Missing type/count/semantic ordinals use explicit native stack-preimage inputs; no zero defaults are invented. All five packet routines pass original-instruction fixtures, including seven installed shader fields and paired allocation/release traces. Original exception ABI, full descriptor loading and gameplay remain unvalidated; see reports/native_shader_field_reader.json.
