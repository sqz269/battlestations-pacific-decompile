# Native renderer clip planes

This packet implements the complete native storage paths B23E50 (166 bytes),
B25040 (62 bytes), and B25080 (46 bytes). Existing camera-frame functions
remain typed fragments. These new interfaces use actual renderer offsets,
the established optional guards, and the complete cached render-state path.
Names are descriptive hypotheses; native caller ABI and gameplay are separate.

B23E50 takes ECX renderer and stack index/float4, RET8. Optional guard entry
precedes all coefficient access. Four sequential x87 FLD/FSTP pairs copy to
renderer+190C+DWORD(index<<4). The native EH state arms after the second store,
before the third load. Masked signaling NaNs therefore quiet in the cache
and set the native x87 status, while the caller's original input pointer is
passed to current device+1A10/current table+DC SetClipPlane. Input/cache aliasing
is observed one coefficient at a time. There is no bounds check, mask setup,
counter increment or HRESULT handling. Current mode is read before normal
cleanup disarming. Native handler CBCF38 uses FuncInfo DF5560 and guard
funclet CBCF30. C++ cleanup exceptions preserve the existing termination policy.

B25040 takes ECX renderer and stack float4, RET4. It calls full B23E50 with
current active+19EC, increments the current field only after return, rereads
it and calls full B24460(state98, DWORD((1<<(active&31))-1)). The x86 shift
count wraps to five bits. Callbacks can change active before the increment.
The parent adds no direct pending+19F0 store. Unchecked child cache aliasing
and callback changes remain visible, including changes to neighboring fields.

B25080 takes ECX renderer, plain RET. It derives the same mask from current
pending+19F0, calls full B24460, then reloads pending and publishes it to
active+19EC. A throwing child prevents this final copy; a child mutation of
pending is reflected. Neither parent adds a guard outside its existing child
guards or invents rollback.

The strict MSVC Win32 main build and both existing CTests passed. Native
original-body fixture validation is pending.
No new permanent tests or game validation are claimed.
