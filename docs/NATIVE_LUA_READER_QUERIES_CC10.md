# Raw Lua reader presence and key enumeration

Addresses: `00BD5EB0`, `00BD5F50`.

This packet reconstructs the successful normal bodies over the existing actual
20-byte `NativeLuaReaderStorage` and `NativeLuaObjectStorage`. Names are descriptive
hypotheses. It does not route through the logical `GuiLuaReader` or registry refs.

| Entry | Inclusive end / bytes | Original ABI | Coverage |
| --- | --- | --- | --- |
| `00BD5EB0` | `00BD5F4F` / 160 | ECX reader; tag/bits on stack; bool AL; `RET8` at `00BD5F4D` (3 bytes) | Complete successful normal path; invalid vector/FH3 excluded |
| `00BD5F50` | `00BD60FD` / 430 | ECX reader; output pointer on stack; `RET4` at `00BD60FB` (3 bytes) | Complete successful normal path with at most 250 supported keys; invalid vector/FH3 excluded |

The live Ghidra program was `/battlestationspacific.exe` in
`C:/Users/sqz269/bsp.gpr`. Both inclusive bodies were checked against the installed
PE bytes. Workers made no Ghidra changes. The JSON report records all 26 direct
call sites with native addresses and containing functions.

## Presence

`BD5EB0` borrows the current raw element, invokes `BD5790` into fresh scratch,
computes `!B65FB0`, then destroys the scratch before returning. It tests bound nil,
not truthiness: a false value is present. Unsupported key tags yield an unbound
object, whose nil predicate is false, so this path returns true. The existing
lookup retains float-index CVTTSS2SI truncation and its invalid sentinel.

The scratch is caller-owned fresh storage. Lookup writes its defined fields and
retains opaque/padding preimages. Actual Lua `__index` behavior remains in the raw
lookup; no fake callback or success fallback is supplied.

## Enumeration producer and ordering

The producer itself proves the layout: `BD602E/31`, `BD6051/54`, and
`BD6083/8A` write two DWORDs at `out + count*8`. The count is at `out+7D0`, so
`NativeLuaReaderKeys` contains 250 8-byte records followed by one DWORD, 2,004 bytes
in total. Tag 0 carries a borrowed C-string address; tag 1 signed integer bits;
tag 2 float32 bits. There is no native bounds check. The source has no count cap
and explicitly limits its valid domain to at most 250 supported keys.

Count zero is written before constructing table, key, then value. The current raw
table is assigned into the table scratch, registering that exact scratch address
when tracked. First/next iteration use the actual Lua stack and publish the exact
key/value scratch addresses. The key-unbound predicate takes its key on the stack;
its ECX is ignored. At completion, destruction runs value, key, table.

Each key is tested as exact STRING, then integer-number, then exact NUMBER.
Numeric strings therefore remain strings. The integer predicate uses the existing
float32/CVTTSS2SI/ordered x87 comparison, and the integer getter reads the caller's
live CRT-mode alias after narrowing. Consequently `16777217` becomes integer
`16777216`, and `-2147483649` becomes integer `-2147483648`. An out-of-range positive
integer can instead take the float output branch. The float accessor and explicit
x87 reload/spill preserve the two caller transfers at `BD606C/6076/607F`.

For supported keys, the getter runs before reading count; then tag, payload, and
incremented count are stored in that order. Volatile DWORD fields retain those
stores. Unsupported key types still advance iteration without output. Untouched
records and scratch opaque DWORD/padding preserve caller preimages. The string
pointer remains borrowed and requires the underlying Lua value to stay alive.

## Validation and limits

One ignored focused probe (`local/output/cc10_query_probe.cpp`) executes the
relocated original bodies with ABI bridges to the already reconstructed raw
primitives and real Lua 5.1.1. The bridges seed fresh original stack scratch with
the same A7/C3 poison preimages supplied to source scratch; they do not zero it.
Four original/source enumeration comparisons cover
empty, mixed keys in both CRT modes, and exactly 250 supported keys. Every output
byte and all 60 scratch bytes match. Mixed keys include numeric strings, rounded
integers, fractions, signed-boundary values, overflow/infinity, booleans, tables,
and light userdata. Seven presence comparisons cover present false, missing,
integer, truncated float, invalid-conversion sentinel, unsupported tag, and a real
`__index` callback. Full scratch, Lua top, all 50 slot counts, high-water, and the
active reader's exact reference identity are checked. The callback observes the
lookup scratch still unbound and the reader reference still registered.

Build and call-row results are recorded in the JSON report. This is an explicit
C++ interface over valid storage, not a native ABI hook or application binding.
Scratch/output/reader/owner storage must be distinct and stay stable. The reader
must be nonempty, the table valid, owner tracking within 50 slots/five refs, and
raw Lua operations successful. No native CRT invalid-parameter path, FH3 handler,
exception cleanup, Lua error/nonlocal unwind, unmasked FP trap, or game parity is
claimed. No new EH helper or custom Lua/CRT/STL port was introduced.

The integrator may append this evidence to the two existing descriptive Ghidra
comments under the write lock, preserving prior text and names. The exported
body and report ranges use inclusive final bytes, not end-instruction addresses.
