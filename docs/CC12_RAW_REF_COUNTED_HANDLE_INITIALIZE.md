# Raw one-word handle-slot initializer, 00415680

The complete native callback is nine bytes and three instructions: move ECX to
EAX, write DWORD zero through EAX, and return without popping arguments. It
reads no slot preimage, releases no old value, calls no provider and consumes no
global or constant. The new raw Win32 Source entry preserves that whole body.
"RefCountedHandle" is a descriptive hypothesis from the actual paired
`0041DE40` destructor; the one-word initializer behavior is established.

The caller supplies one writable four-byte slot and owns its lifetime. EAX
returns the same address. The body leaves ECX, EDX, flags, stack balance,
nonvolatile registers, x87 and SSE state unchanged. It neither constructs the
surrounding object nor performs ownership cleanup.

`00424BC9` supplies this callback to the genuine CRT array constructor in the
gameplay-settings constructor; `0064B7A8` is another callback data reference.
Neither caller is expanded here. Ghidra had no function at this actual callback
entry. The preceding routine ends before four alignment bytes; defining the
exact `00415680..00415688` range under the shared write lock creates a separate
function without truncating another body. Original/live bytes, prior absence,
definition events and preexisting caller identities are retained.

The Source entry is registered in the normal MSVC Win32 build. Validation and
complete production-object/archive evidence are recorded in
[the report](../reports/cc12_raw_ref_counted_handle_initialize.json). This is
static whole-body and build evidence. No new tests, probe, Source/native entry
execution, owning destructor ABI, settings constructor/Lua/EH closure, game
startup or gameplay qualification is claimed.
