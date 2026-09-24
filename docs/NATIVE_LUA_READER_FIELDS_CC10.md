# Raw Lua reader field dispatch

`src/native_lua_reader_fields.cpp` reconstructs the normal paths of
`00BD6830..00BD68CE` (159 bytes) and `00BD68D0..00BD698B` (188 bytes).
These descriptive names are hypotheses. The original ABI takes the actual
reader in ECX and eight-byte key/field pairs by value on the stack, plus a
fallback pair for BD68D0, and returns with RET10h/RET18h. The new C++ interface
uses explicit caller-owned pair pointers, scratch storage and raw bindings.
It is not binary compatible with those methods.

Both entries borrow the actual final 20-byte vector element, then call
BD5790 using caller-owned temporary storage. BD6830 always passes the result
to BD63B0, including nil. BD68D0 calls B65FB0 and selects BD61C0 only for
actual Lua nil; an unsupported key tag produces an unbound object, for which
IsNil is false, and therefore still selects BD63B0. A successful store is
followed by B67700 release of that same temporary address. StoreDefault
dispatches only on the field tag and ignores the fallback tag.

The implementation uses the actual reader, Lua owner tracking and string
pool providers. It does not create a registry projection or copy the field
or fallback pairs. Scratch must be stable and initialized; lookup begins
unbound, and unwritten opaque/padding bytes retain their preimage. Existing
Lua slot/reference limits and every callee's raw pointer contract apply.

The six BF6713 validation calls in the native wrappers apply to invalid
vector ranges. This source admits valid nonempty readers only, so their CRT
failure behavior is excluded. The FH3 thunks CC5B18 and CC5B38 each load their
original metadata address and tail-jump to BF6B43. Their exact ten-byte
bodies were defined under the Ghidra write lock and saved; their presence
does not establish source cleanup equivalence. Exceptions, nonlocal Lua
transfers, native frame aliasing, binary ABI and gameplay remain unverified.

Evidence and validation are recorded in
`reports/native_lua_reader_fields_cc10.json`. No new tracked tests were
added for this composition of existing raw providers.

Primary integration `4539e852b` built these wrappers together with the raw
BD63B0 dependency. Strict MSVC Win32 Release and all three existing CTests
passed. The wrapper composition has static/build evidence only; the value
and query differential probes do not constitute wrapper runtime coverage.
