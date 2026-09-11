# Native renderer binding getters

Eight complete native functions are reconstructed in
`src/native_renderer_binding_getters.cpp`. All **44 compiled instruction bytes**
match both the saved Ghidra program and installed executable, with zero COFF
code relocations. These are narrow borrowed-storage entries; they provide no
resource construction, reference management, allocation or full renderer owner.

| Original | Bytes | Actual read / result | Original profile and slot |
| --- | ---: | --- | --- |
| `00B3CEA0` | 4 | texture `+10` -> `IDirect3DTexture9*` | `00D61948 +1C` |
| `00B48CE0` | 4 | logical vertex `+68` -> engine CPU declaration | `00D61D6C +24` |
| `00B48D10` | 4 | logical vertex `+5C` -> raw offset word | `00D61D6C +28` |
| `00B48CF0` | 10 | logical vertex `+58` -> physical owner -> current table `+1C` tailcall | `00D61D6C +2C` |
| `00B48DC0` | 10 | logical index `+08` -> physical owner -> current table `+1C` tailcall | `00D61DE0 +28` |
| `00B4B9F0` | 4 | physical vertex `+28` -> `IDirect3DVertexBuffer9*` | heap `00D61E34 +1C`; pooled `00D61E7C +1C` |
| `00B4B840` | 4 | physical index `+28` -> `IDirect3DIndexBuffer9*` | heap `00D61E10 +1C`; pooled `00D61E58 +1C` |
| `00B5FF00` | 4 | hardware layout `+40` -> `IDirect3DVertexDeclaration9*` | `00D62AF4 +08` |

The public declarations use one `const void*` argument with `__fastcall` to put
the original input in ECX. All results are borrowed `void*`, except `B48D10`,
which returns `uint32_t`. The six direct functions are naked `MOV EAX,[ECX+n];
RET` entries. They preserve flags and do not add null checks, normalization,
lazy creation or AddRef calls. A null stored result is returned unchanged;
readable actual input storage remains a precondition. The CPU declaration from
`B48CE0` is distinct from the COM declaration returned by `B5FF00`.

The two indirect entries contain exactly:

```asm
mov ecx, [ecx + 58h]  ; B48CF0; B48DC0 uses +08h
mov eax, [ecx]
mov edx, [eax + 1ch]
jmp edx
```

They load the physical owner once, load that owner's current table once, then
read the target and transfer control with the physical owner in ECX. There is
no stack argument, wrapper frame, extra return, local exception handler or
cached table. EAX and EDX contain the table and selected target at transfer;
the target provides the returned EAX value and RET. Both declarations omit
`noexcept`, preserving an indirect boundary without adding an exception policy.
The concrete recovered targets here are the direct physical getters above.

A rebuilt host must install **callable relocated getter addresses** at the
corresponding table offsets. For example, the host version of either physical
vertex profile must route `+1C` to
`bsp::native_physical_vertex_buffer_get_com_00b4b9f0`; either physical index
profile routes to `bsp::native_physical_index_buffer_get_com_00b4b840`. Logical
objects must reference live physical storage with those current tables. Merely
writing numeric original table addresses into host objects does not establish
callable bindings. This packet does not create or install complete tables.

Heap and pooled physical profiles share their respective getter but have
different release and storage paths. Selecting the getter therefore establishes
no allocator identity or lifetime closure. Direct getters touch through offsets
`13h`, `6Bh`, `5Fh`, `2Bh`, `2Bh` and `43h`, respectively. The tailcalls touch
logical storage through `5Bh` or `0Bh`, the physical table pointer and table
slot through `1Fh`; their concrete target then reads physical `28h..2Bh`.

## Evidence and validation

The [audit](../reports/native_renderer_binding_getters_audit.json) records each
complete function span, compiled symbol and object offset, all ten original
table cells, source hashes and prior annotations. Every one of the 18 fresh
live reads verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` first. The 44 code bytes and 40 table-cell bytes
equal the installed PE. The earlier actual-binding discovery supplied the
profile interpretation; this packet independently rechecked every listed
getter body and table cell.

One private route check compiled the actual production source separately with
MSVC Win32 `/std:c++17 /EHsc /fp:strict /O2 /W4 /WX`. It populated ten host table
cells with callable production entries, called all eight leaves through those
cells, and followed the logical vertex/index routes with both heap and pooled
physical tables. Borrowed results, the high-bit raw offset and unchanged owner
storage passed. The fixture tables contain only the reachable getter entries;
they are not complete profiles usable for other virtual methods. The check
does not execute the installed game or fabricate alternative target callbacks.
Instruction equality establishes the load order; this small check exercises
ordinary valid calls through the concrete host bindings.

The eight existing native seeds matched. `./scripts/build.ps1` and both existing
CTest checks passed. This worker's new source is compiled by the private check;
the primary integrator owns registration in the normal CMake target. No tracked
test was added. Reproduction in the worker checkout:

```powershell
./local/build_native_renderer_binding_getters_check.ps1
python local/verify_native_renderer_binding_getters.py
python tools/ghidra_export.py verify-seeds
./scripts/build.ps1
```

The three four-byte entries `B3CEA0`, `B4B9F0` and `B4B840` have no function
definition in the captured Ghidra state. The primary must create those exact
definitions before annotation, preserve the five existing names/comments, and
refresh exports. This worker changed no Ghidra state, shared metadata or CMake.
Descriptive names are reconstruction hypotheses. Exact instruction and narrow
entry-ABI agreement does not install a binary replacement; full object/table
lifecycle, concurrent mutation, arbitrary target exceptions and gameplay remain
outside this proof.
