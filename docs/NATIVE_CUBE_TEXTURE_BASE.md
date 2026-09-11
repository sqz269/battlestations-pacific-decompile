# Unnamed logical texture base used by cube textures

`B34020..B34067` is the complete 71-byte unnamed logical texture constructor.
It takes the actual owner in ECX, borrowed COM and flags on the stack, returns
the owner in EAX, and uses `RET 8`. It has no calls or exception frame. The
corresponding five-byte `B34090..B34095` cleanup entry jumps directly to the
already reconstructed `B33F50` named-base destructor.

The source is `src/native_cube_texture_base.cpp`, with its contract in
`include/bsp/native_cube_texture_base.hpp`. The constructor writes these fields
in native order: profiles `CEB130` then `D5F1F4`; reference count one; zero name
length and pointer; zero word `+14`; borrowed COM at `+10`; flags at `+1C`;
current shared serial at `+20`; increment the current shared serial with DWORD
wrap; final profile `D5F280`. Word `+18` and storage at and beyond `+24` remain
untouched. Existing name and COM values are overwritten without release or
retention. The raw access helpers preserve each reached load and store.

The serial parameter borrows the actual DWORD corresponding to `108D6E8`,
shared with the named and other unnamed texture constructors. It must not be a
new counter for each texture category. The increment rereads current storage
after the owner serial store, retaining the original behavior for backed
aliases. Native construction is not interlocked.

Cleanup delegates directly to
`destroy_native_logical_texture_named_base_00b33f50` with the actual owner's
current name and its genuine `NativeStringStorage`. It adds no preliminary
`D5F280` store and no second destructor implementation. The existing provider
retains the current-name release and base cleanup behavior documented in
`NATIVE_LOGICAL_TEXTURE_NAMED_BASE.md`.

Fresh guarded live/PE evidence and all direct dependency boundaries are in
`reports/native_cube_texture_owner_next.json`. Both complete entries and their
76 instruction bytes were independently checked by the primary. The existing
`B34020` numeric serial fragment remains a fragment; the full constructor is
now recorded separately after its independent fixture and annotation closure.

The strict integrated MSVC Win32 build and both existing CTests passed.
The current library is frozen at
`build/cube-base-primary-check/bsp_core.linked.lib`, SHA256
`e868288d29c1a2d75b09e520ea75648a792c9976d74b7b5307a381563e11ee6c`.
Independent verification passed with 1,152 matching DWORDs and 35 observed
owner stores, including equal-value writes, across three constructor serial
bindings and real pooled current-name destruction. The primary rechecked all
49 source/artifact pins, three fresh live/PE spans, 18 runtime postimages and
actual linker providers. See `NATIVE_CUBE_TEXTURE_BASE_FIXTURE.md`. Both complete
entries now have saved Ghidra evidence, refreshed exports and separate full
reconstruction records. The earlier numeric metadata fragment is retained. The original ABI,
malformed unbacked storage, full native string-pool ABI, GPU behavior, and
gameplay have not been validated by this packet.
