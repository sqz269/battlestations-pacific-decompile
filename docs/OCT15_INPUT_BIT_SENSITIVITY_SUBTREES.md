# Actual input bit and sensitivity subtree destruction

This packet reconstructs the whole ordinary entries `00558420` and `00558ED0`,
including their actual payload/key cleanup. Descriptive names remain hypotheses.
The old frontend and AV interfaces remain separate: their storage boundary does
not supply this propagating raw pool path.

Both native interfaces receive the opaque tree in ECX and one stacked node,
do not consume EDX, return with `RET4`, and have no semantic result. The tree
is forwarded through right recursion and never dereferenced. The Source APIs
retain those words and add `NativeStringRawPoolContext`. Complete typed bodies,
live bytes, installed PE bytes and all cached export instruction starts agree:
112 bytes/47 instructions for `00558420`, 109 bytes/46 for `00558ED0`.
Neither body installs a local exception frame or catch.

The nil byte is `+29`; any nonzero value stops without freeing that node.
Both bodies recurse into the current right link `+8` and iterate left.
For the bit payload, capture the left link first, clear bit count `+14`,
then read backing `+1C` and free that captured pointer if nonnull. For the
sensitivity payload, read/test backing `+1C` before capturing the left link;
there is no `+14` or `+18` write. Both clear `+1C/+20/+24` after the optional
backing free, then destroy the current pooled key at `+C`, free the captured
current node, and read the captured left node's nil byte before continuing.

The accepted full raw `0041DD20` key service reads current data `+10` once,
reads current length `+C` only when data is nonnull, computes unsigned
length+1 before resolving `00419CC0`, then returns the captured block through
`00BD1510` with flag1. It leaves key fields untouched. This matches both
inline native schedules. A getter failure escapes after prior effects;
the current node and later left chain are not subsequently freed. No
`NativeStringStorage::release noexcept` or terminating adapter is used.

Every direct call site is recorded in
`reports/oct15_input_bit_sensitivity_subtrees.json`:

```text
00558439 CALL 00558420
0055844B CALL 00BF65AC
0055846D CALL 00419CC0
00558474 CALL 00BD1510
0055847A CALL 00BF65AC
00558EE9 CALL 00558ED0
00558EF8 CALL 00BF65AC
00558F1A CALL 00419CC0
00558F21 CALL 00BD1510
00558F27 CALL 00BF65AC
```

The recursive targets bind the complete own Source entries, `00BF65AC`
binds current CRT free of captured backing then the current node, and the
getter/return pair binds the complete raw key composition described above.

Valid reached actual native node, mapped backing, key ownership and current
pool/CRT contracts apply. There is no null guard, traversal validator, callback,
rollback, snapshot or tree/head/count reset. The helper load/store order is
retained explicitly. These added C++ interfaces and current CRT services do not
establish original register/stack, SEH, instruction fault, exception, heap or
gameplay identity. The existing native math differential suite does not execute
these new entries. Whole parent record destruction remains a separate packet.

Build and independent review results are recorded in the packet report.
