# AnimationChannels and Bone item parser outer bodies (R25)

This packet reconstructs the complete parser slot-8 bodies at `00B8A910` and
`00B8A990`. Both are ordinary MSVC `thiscall` entries: parser `ECX` is ignored,
one structured-node handle is stacked, the result item is returned in `EAX`, and
the callee executes `RET 4`. The source exposes a new explicit-service C++ ABI;
it is not a binary replacement for those entries.

## Exact construction and dispatch

| Parser | Body | Allocation | Result profile | Initialized payload | Current slot `+20` |
|---|---:|---:|---:|---|---:|
| AnimationChannels | `00B8A910..00B8A986` | `14h` | `00D6328C` | zero `+08`, `+0C`, `+10` | `00B8AD80` initially |
| Bone | `00B8A990..00B8AA0B` | `2Ch` | `00D632B8` | zero `+08`, `+0C` only | `00B8AF30` initially |

Each body allocates through `00BF681B`, calls the existing complete reference
base constructor `00B868B0`, overwrites its final profile with the result
profile above, captures the **current** table slot `+20`, and calls that target
with `ECX=item` and the original handle stacked. Bone bytes `+10..+2B` retain
allocator contents until its reader writes them; the source does not fabricate
defaults for that reader-owned region.

`NativeResourceExtraItemReaderCalls` is a required concrete dependency. It must
execute captured target `00B8AD80` or `00B8AF30` with the actual reader contract.
This packet does not claim either reader body. The finite root-dispatch adapter
intercepts only `00B8A910` and `00B8A990`; renderer hooks, appends, and unrelated
parser targets forward unchanged.

## Lifetime and exception boundary

The native functions arm EH state zero after allocation. Their state-zero
unwind funclets call `_free` on the raw allocation. Both set state `-1` before
the slot-20 reader call, so a reader exception retains the constructed,
reference-counted item and performs no parser-level rollback.

The source needs no separate C++ construction-failure guard. Its reused
`construct_native_resource_item_base_00b868b0` interface is `noexcept` and its
actual body consists only of the three profile/count stores. Allocation failure
throws before a returned allocation needs cleanup; a null-returning allocator
reaches the same null table access boundary as the original. Hardware faults,
the original FH3 machinery, and CRT exception identity remain outside this
source interface.

## Evidence and validation

Read-only Ghidra queries verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86-32 language, and image base `00400000`.
The two complete bodies, their raw-free unwind funclets and handler adapters,
and both unwind-map/`FuncInfo` ranges total 373 bytes. Every byte matched the
installed PE; the aggregate SHA-256 is
`ff044df08a71dd6368031f6f40164f35002bd2ffd419b7c4e316de1235e62f85`.

The committed source passed an independent MSVC Win32 `/W4 /WX /EHsc
/fp:strict /MD` compile. After `ghidra_export.py verify-seeds` matched all eight
seeds to disk, the full Release build compiled this source into `bsp_core` and
all three existing CTests passed. No permanent test or semantic reader stand-in
was added.

Detailed hashes, exact ranges, old metadata, reviewed annotation proposals, and
unresolved dependencies are recorded in
`reports/native_resource_extra_item_parsers_r25.json`. Raw ignored exports are
under the shared workspace at `exports/bsp/functions/00b8a910/` and
`exports/bsp/functions/00b8a990/`.

This establishes complete source-level behavior for the two outer parser
bodies. It does not establish either item reader, deletion paths, full resource
loading, original callable ABI/FH3/SEH parity, executable admission, or gameplay.
