# Native shader preload loop (R99)

Two full normal-path routines now have Win32 source: the shader preload loop at
`0073BF80` and its actual pointer-vector reserve helper at `00735EC0`.
The loop calls the existing concrete cache, Lua and material-effect providers.
It is **not yet connected to ordinary application startup or runtime-validated**;
the material/compiler graph and initial sampler stack inputs remain dependencies.

## Recovered behavior

| Routine | Original ABI | Source coverage |
| --- | --- | --- |
| `0073BF80` (670 bytes) | ECX application; RET; no useful result | Cache create, Lua mask1, preload script, FileNames iteration, current renderer+48 material load, application+08 append, cache release and reverse Lua cleanup. |
| `00735EC0` (95 bytes) | ECX 0Ch header; signed stack request; RET4 | Minimum1, signed capacity gate, wrapping byte request, live pointer/count copying, current old-data free, then new pointer/capacity publication. |

The preload routine constructs the exact 26-character
`shaderfx/shaderpreload.lua` name, runs the actual Lua file/override route, obtains
`FileNames`, and iterates every key in native Lua order. It converts each value
through `B662B0`, scans the returned string and creates the actual pooled header.
There is no added integral-key filter or filename sort.

Each entry reloads the current renderer, validates the supported original
`D5F0A8+48 -> B318B0` route, and invokes the real material cache/loader context.
The returned pointer is appended even when null. A nonnull result's acquired
reference transfers directly into the application's existing vector; the loop
adds no extra retain and does not replace the vector with host-owned storage.
When count equals capacity it doubles capacity with the original signed/wrapping
minimum-one rule. It reloads current vector data/count after reserve, publishes
the element/count, then releases the current name buffer.

After iteration, `B3B140` releases the shader cache **before** cleanup of Lua value,
key, table and owner. Both bracket trace targets at `4254B0` are a single RET;
the source adds no logging side effect. The exact original literals and renderer
slot were checked against the PE and live analysis.

## Repaired analysis

Ghidra's `CALL_RETURN` override at `00735F0C -> free` hid nine reachable bytes:
stack cleanup, the new data-pointer/capacity stores, and saved-EBX restoration.
The locked repair preserved prior evidence, cleared only that call-site override,
disassembled the bytes, saved the project and refreshed the export. No function
was created/deleted and the callee's global no-return flag was not changed.

## Lifetime and source interface

`NativeShaderPreloadOperation` persists before native effects and retains the
actual Lua state/objects, stable pooled-name header, cache operation and per-load
material frames. On success it owns metadata only; application vector entries
remain application-owned. On failure it retains partial state and any returned
but unpublished material reference, rejects replay and refuses silent destruction.
This is an explicit source boundary, not native FH3 rollback.

The context requires the same string, Lua DoFile, VFS and renderer publication
domains as its concrete child providers. The caller must activate the existing
Lua service binding and supply the actual application/vector lifetime. No private
material provider, preseeded success objects or no-op load callback was introduced.

## Validation and remaining work

- Strict MSVC Win32 compilation and all three existing CTests passed.
- The native bodies total 765 bytes. Including the RET-only trace, original
  literals and material slot, 869 live-analysis bytes match the original PE.
- One local original/source reserve-helper sequence compares minimum-one
  allocation, growth after allocator mutation of source/count, free-time count
  mutation, publication after free, and the no-growth path. The source and all
  95 original instruction bytes agree through two shared allocation/free adapters.
  Each leg frees its tracked fixture allocations.
- The unmodified application completed two ticks and one Present, joined its
  worker, released D3D device/API to 0/0 and exited 0. It did not run the new loop.
- No permanent tests or extra CMake test targets were added.

The complete preload loop still needs runtime validation after shared material
providers and compiler stack-input production are connected. The application's
preloaded-reference vector lifetime, original CRT/new-handler and FH3/SEH behavior,
nonlocal Lua errors, scene rendering and gameplay remain open.

[The report](../reports/native_shader_preload_r99.json) contains source/object
hashes, exact native call sites, the saved flow repair and annotations, the
reserve differential, application smoke result and immutable artifact receipts.
