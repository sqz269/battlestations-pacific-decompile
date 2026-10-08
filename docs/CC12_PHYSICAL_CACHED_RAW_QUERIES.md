# CC12 physical cached raw queries

Two distinct raw entries are added to the existing compiled physical-stream module: `raw_position_native_physical_stream_00bf4f40` and `raw_size_native_physical_stream_00bf4f90`. Their complete Original, normal-production COFF and linked bodies agree exactly. Four fresh memory-only Source/Original pairs passed. Existing ordinary ports and previous raw entries are retained.

## Complete Original contract

Both entries receive actual stable backing in ECX, preserve ECX, load a 64-bit bit pattern into EDX:EAX, and execute plain `RET` with no stack arguments. Incoming EDX is unused. Each entire body is seven bytes and three instructions, with zero calls, globals, relocations or code normalizations.

| Entry and inclusive extent | Whole bytes | EAX low | EDX high |
|---|---|---|---|
| `00BF4F40..00BF4F46` position | `8B 41 10 8B 51 14 C3` | `[ECX+10h]` | `[ECX+14h]` |
| `00BF4F90..00BF4F96` cached size | `8B 41 18 8B 51 1C C3` | `[ECX+18h]` | `[ECX+1Ch]` |

These are cached-field loads only: no HANDLE use, backing write, OS query or size refresh occurs. The C++ raw declarations expose unsigned 64-bit return bits on MSVC Win32. The ordinary cached-size and position ports are unchanged.

## Exact identity and the repeated byte pattern

The fresh family is `local/cc12_physical_cached_raw_queries_20261008a`. Run01 stopped at its static gate before any fixture production entry executed: the new position body was exact, but the same seven-byte sequence also appeared at an unrelated `PhysicalFile::size_00bf4f90` member from `physical_file.obj`. The strict global-occurrence==1 assertion and its frozen recipe, manifest and failure logs are retained. No source implementation changed to pass this gate.

Run02 identifies each raw function by its unique decorated linker symbol, exact start address and actual normal-production COFF member, then compares the entire body. It also extracts the complete unrelated `physical_file.obj` from the pinned real archive, verifies that member's size-leaf COFF bytes, and identifies its distinct symbol/address. The raw position entry is `20001030`; the unrelated equal-byte model member is `20002120`. **The position byte pattern is not globally unique.** Raw size is at `20001040`, and the current Source constructor is at `20001000`. Exact whole-object membership of the production module in `bsp_core.lib` is verified.

All static gates finish before execution. The probe then checks every loaded Source byte and named entry address, and copies the unchanged complete Original seven-byte bodies to private RX memory. No call/import or code operand is rebound or normalized.

## Fresh memory-only comparison

One call to the current Source raw constructor `00BF50D0` establishes the actual retained 20h backing. Its complete 43-byte Original/current COFF/linked equality is proved beforehand. The constructor result and the full 64-byte guarded image are checked. It retains literal Original profile `00D691B0`; this is not a Source class/vtable. No Original constructor executes.

The fixture puts distinct high-bit values in both cached fields, compares each leaf once, mutates the same live fields without changing the receiver address, and compares each leaf once again:

| Leaf | Epoch | Expected and observed EDX:EAX |
|---|---|---|
| Position | Before mutation | `80000001:FEDCBA98` |
| Cached size | Before mutation | `F1234567:89ABCDEF` |
| Position | After mutation | `FFFFFFFF:10203040` |
| Cached size | After mutation | `80000000:FEDCBA98` |

All four pairs preserve the complete backing and both 16-byte guards, ECX, ESP/plain-RET cleanup, and EBX/ESI/EDI/EBP. Different unused incoming EDX seeds do not affect the returned high word. HANDLE remains `FFFFFFFF` and is never consumed. EFLAGS are not qualified. There are eight instrumented query calls and one current Source constructor call: 57 bytes across the executed Source production bodies, and 14 Original query bytes. No previous constructor, seek, open or GetFileSize fixture is replayed, and the unrelated equal-byte model method is not called by this fixture.

## Build and retained inputs

`./scripts/build.ps1` passed with all three existing CTests: `reconstructed_math`, `native_math_differential`, and `tool_tests`. There are no new permanent tests. The initial two-object/zero-BSP-archive link failed with 10 unresolved externals (exit 1120); its recipe/log is retained. The real current core, Lua511 and zlib121 archives resolve those ordinary dependencies without stubs or adapters. Those unexecuted providers are not admitted. The probe embeds an `asInvoker` manifest.

Successful run02 has matching before/after/frozen-copy hashes for the whole normal object/archive, both explicit objects, the complete unrelated archive member, all recorded actual inputs and recipes, 202 normal-module input files, 164 probe headers, 19 searched libraries, 16 recipe/tool records, 46 compiler/backend files, two harness OS DLLs and eight generated recipes. The normal module's actual compiler command and generated MSBuild project are also frozen and checked across the build. The manifest and report retain exact paths, hashes, entry identities and proof/execution timestamps; later builds cannot overwrite these frozen copies.

See [the report](../reports/cc12_physical_cached_raw_queries.json), `run02/whole_cached_proof.json`, `run02/inputs/manifest.json` and `run02/execution.log`. This closes these two raw cached-field leaves only. Source class/table/lifetime, raw substream dispatch, refresh/OS semantics, universal CPU/ABI equivalence, startup and gameplay remain unqualified. Ghidra and shared ledgers remain primary-owned.
