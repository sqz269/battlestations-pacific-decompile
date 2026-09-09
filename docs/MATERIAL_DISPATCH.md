# Concrete material dispatch

The batch executor `00b55550` follows entry+4 -> section+20h -> material+7Ch
and invokes effect virtual+14h. The debug geometry factory `00b4c700` connects
this path: `00535320` obtains a material, `00533fa0` creates a draw section,
and `00b864c0` assigns its material, retaining the new reference before releasing
the old one.

`00535320` calls renderer virtual+48h and passes the resulting effect to material
constructor `00b18900`. That constructor installs vtable `00d5e520`, retains
the effect in material+7Ch and sets effect+B4h to one. Allocator, destructor and
full ownership behavior are not reconstructed.

Renderer virtual+48h is `00b318b0`. It takes the optional renderer guard,
normalizes a resource name and invokes generic loader `00b31090` on the embedded
registry at renderer+1A98h (LEA at `00b3194c`), passing flags 0, 1, 1.
Renderer constructor `00b32410` installs registry vtable `00d5f074` at
`00b325a8`. Its creation slot +8h resolves to `00b2ebb0`.

`00b2ebb0` constructs .mshd/.shfx name alternatives, checks availability through
`00bdf4c0`, allocates 178h bytes, constructs an effect with `00b407a0`, and loads
it via `00b46950`. Failure handling logs and requests `error.shfx` through the
renderer. Filename replacement direction and registry search-path rules need
further assembly tracing. Installed shader descriptors are discussed in
[ASSET_ENTRY.md](ASSET_ENTRY.md).

Effect constructor `00b407a0` installs vtable `00d61a00`, whose +14h slot is
`00b45360`. This is the concrete dispatcher reached by batch execution.
It receives the effect in ECX and one stack entry pointer, returning with RET4.
It selects behavior using entry+10h -> field+198h, entry+18h, effect pass storage
and an optional collection at entry+Ch -> field+170h. Mode 2 with effect+138h
present calls `00b44750` directly; other paths select pass storage and optionally
an override before calling the same helper. Pass names and complete override
semantics are unresolved. `00b44750` is the next execution dependency to trace.

Six functions received descriptive Ghidra names and evidence comments; prior
annotations were preserved locally and the project was saved. The report
`reports/material_dispatch_evidence.json` records assembly and hashes for six
original-file/Ghidra byte ranges. This establishes a concrete dispatch chain,
not a compiled material system or a rendered game frame. No native ABI or
gameplay equivalence is claimed.
