# Logical stream upload path

Recovered vertex vtable `00d61d6c` uses `00b49980` at `+10h` for lock and
`00b49a80` at `+14h` for unlock. Index vtable `00d61de0` uses `00b49b60` at `+ch`
and `00b49c70` at `+10h`. These methods are ported into the renderer state interface
to reuse its optional synchronization state and guard.

Dynamic vertex flags with nibble `1000h` update stream vertex count `+64h`, request
count*stride bytes, multiply extra vertex offset by stride, force read-only false,
and store the physical lock's returned base byte offset at `+5ch`. Non-dynamic
locks use the stored vertex count if requested byte count is zero, add stream
base vertex `+70h` to the requested offset, forward read-only, and leave the bound
byte offset unchanged. Both cache the returned mapping at stream `+8h`. Without
a physical wrapper, the native method returns the existing mapping. Unlock calls
the physical wrapper when present, then always clears the mapped pointer.

Index locks derive element size from INDEX16/INDEX32 (2/4 bytes; other formats
produce zero). Zero requested byte count falls back to stored index count `+14h`.
The byte offset includes base index `+20h`; the returned physical offset is stored
at `+ch`. The method does not update stored index count. Index unlock only delegates
when the physical wrapper exists; unlike vertex unlock it has no mapping to clear.

The C++ interface returns HRESULT and exposes the pointer separately, using S_FALSE
for the absent-physical-wrapper path. The previously documented physical-lock
limitations still apply: diagnostic/sentinel branches return INVALIDCALL. Full
constructors, allocator/registry lifecycle and ABI compatibility remain unported.

The pixel probe now uploads its vertices and indices through these logical methods,
then binds and draws with the reconstructed stream/declaration paths. It checks
that the dynamic vertex count becomes four, its mapping clears on unlock, the byte
cursor advances to 80, and lock depth balances. The dynamic read-only input is
deliberately true and the port follows native behavior by issuing a writable lock.
Index upload uses zero requested count to cover fallback to all three stored indices.
Both non-indexed/indexed readbacks pass. No additional CTest cases were added.

`00b4b1e0`, previously suspected to allocate stream ranges, actually registers a
unique raw logical-stream pointer in the physical wrapper's list, with capacity
doubling and no AddRef. Cursor allocation remains in physical Lock. Recovered
offset invalidators `00b48d40`/`00b48dd0` only write -1 to logical offsets; they do
not rewind the physical buffer. Vtable target `00b4aaa0` is a single RET no-op.

Next: recover physical cursor reset and registered-stream invalidation, then connect
the actual shared-buffer constructor and resource release/recreate orchestration.
This remains a diagnostic geometry path, not a game startup or gameplay result.
