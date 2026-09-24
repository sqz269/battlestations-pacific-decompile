# Raw Lua field defaults

`store_native_lua_field_default_00bd61c0` reconstructs the complete 312-byte
`BD61C0..BD62F7` normal body over actual 8-byte field and fallback pairs.
The original takes the field in ECX and the fallback pointer on the stack,
with `RET4`. The new source adds the canonical raw string-pool context.
Names are descriptive hypotheses, not recovered symbols.

Only the field tag controls dispatch; the fallback's own tag is ignored.
Tags 1 and 4 copy one DWORD, tag 3 copies one byte, and tags 2 and A use an
x87 float load/store. Tags 5 and 6 use three or two sequential x87 pairs.
Tag 7 tail-calls the existing exact `4134F0` matrix entry. Tag 8 copies four
DWORDs sequentially, with its first source read preceding the destination
pointer read. Tag 9 and other unrecognized tags do nothing. Source accesses
preserve these distinctions and forward overlap; a vector snapshot or plain
float bit copy would change the original behavior for some overlaps/NaNs.

Tag 0 captures the actual destination header and fallback C-string pointer,
measures the nullable string, and calls `41DD40` with preserve false. It then
reads the current destination data pointer and, if nonnull, copies its current
length. It does not append another terminator. The existing resize writes its
own terminator. The copy uses the admitted overlap behavior of `BF7680`; its
zero-byte branch performs no memory access. The canonical raw pool and its
current publication/gate are reused without a second string owner.

The full 312 live Ghidra bytes matched the installed PE. Strict MSVC Win32
and all three existing CTests passed. One ignored `/MD /W4 /WX` probe with an
embedded manifest passed 18 original/source pairs: all field tags 0..A plus
unknown tags, deliberately mismatched fallback tags, forward vector/matrix
overlap, masked x87 exception flags including signaling-NaN/denormal payloads,
and real pooled strings. Scalar/vector instructions executed from the copied
original body; its two calls and matrix tail jump were rebound to the same
production resize, copy and matrix primitives. This does not independently
validate those callees, unmasked faults, native ABI/SEH, the full reader, or
gameplay. No tracked tests were added. Detailed evidence and hashes are in
`reports/native_lua_field_defaults_cc10.json`.
