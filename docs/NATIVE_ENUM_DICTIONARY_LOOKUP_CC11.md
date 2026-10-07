# Borrowed native enum dictionary readers

Packet `cc11_scene_enum_dictionary_lookup` reconstructs four complete normal
reader bodies. It supplies a useful prerequisite for the native enum lookup used
by traffic templates; it does not connect the currently unbound traffic loader
or construct a native enum library. Names are reconstruction hypotheses.

| Entry | Inclusive end | Exclusive end | Bytes | New Source function |
|---|---|---|---:|---|
| 004895B0 | 00489608 | 00489609 | 89 | `native_enum_symbol_bucket_004895b0` |
| 00489610 | 00489668 | 00489669 | 89 | `native_enum_table_bucket_00489610` |
| 0048D480 | 0048D4DC | 0048D4DD | 93 | `find_native_enum_symbol_node_0048d480` |
| 0048D4E0 | 0048D53C | 0048D53D | 93 | `find_native_enum_table_node_0048d4e0` |

The report records complete live/disk byte hashes and all eight direct call-site
rows. Read-only `bsp.py ghidra proto/bytes/disasm` verified the existing
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Neither listing
repair nor annotations were performed.

## Hash and lookup contracts

Both hash bodies load the key's data from +4 and length from +0. Initially
`hash = remaining = length`, with `step = (length | 20h) >> 5`. For each iteration
while `remaining >= step`, the native passes a sign-extended byte to `_toupper`,
retains the low result byte, and updates the DWORD hash as
`hash ^= byte + (hash << 5) + (hash >> 2)`. It subtracts `step` from the separate
remaining counter but advances the data pointer **one byte**. The final bucket
is `hash & 3Fh`. Thus a 64-byte key samples its first 21 bytes, not every third
byte. Empty keys skip the byte/provider read. Null data selects `00E186ED`.
The separate evolving ESI hash and EBX remaining counter are established by the
whole assembly; a decompiler variable conflation is not the contract.

The finders take the actual receiver in ECX and key-header/out-bucket pointers
on the stack. They call their corresponding hash, store the result through the
out-bucket pointer, then load the head at `receiver + 8 + bucket*4`. Only after a
nonnull first head do they capture the query length once. At each node they
compare length +0; matching lengths lead to data +4 (or `00E186ED`), query
DataOrEmpty `00419CA0` (or the distinct `00E17654`), and comparison `00438E10`.
A zero comparison returns that node. Otherwise traversal follows +0C until null.
The mapped DWORD +8 is neither interpreted nor read by these four bodies.

Observed terminal instructions are RET4 at `00489606`/`00489666`, and RET8 at
`0048D4D3`/`0048D4DA` and `0048D533`/`0048D53A`. These receipts establish the
observed boundary, not a formal original ABI for the new C++ interfaces.
In particular `0048D4E0` is another reader, **not an insertion/default helper**.

## Existing providers and ownership

`include/bsp/native_string.hpp` and `src/native_string.cpp` provide the actual
Win32 eight-byte `NativeString` header and the current raw-pool
`construct_native_string_header_0041e870`/explicit destruction bodies.
`NativeStringRawPoolContext` borrows publication/shutdown/manager cells.
`ActualNativeStringPoolStorage` lives in `native_string_pool_storage.hpp/.cpp`;
the genuine `NativeStringPoolStorage` owner construction/destruction is in
`native_string_pool_owner.hpp/.cpp`. `GameNativeStringProcess` in
`game_native_string_process.hpp/.cpp` is the current application's process owner.
The fixture uses the real owner and preconstructed publication directly, so its
admitted reads do not lazily create a manager or substitute allocator callbacks.

`native_string_data_or_00419ca0` in
`native_lua_script_overrides.hpp/.cpp` is the existing complete DataOrEmpty
provider. `compare_insensitive_00438e10` in `entity_identity.hpp/.cpp` binds the
complete `00438E10` rule: ECX left/EDX right, equal-pointer zero, null ordering,
then actual native `_stricmp` at `00BF7FBF`. Source uses current MSVC `_stricmp`;
the hashes use current CRT `toupper`. ASCII keys in the C locale are the admitted
Source provider domain. No non-ASCII/other-locale parity is claimed.

The four readers borrow every header, receiver and node and return a borrowed
node. They own no string buffer or map and only write out-bucket. Native
`008F2850`/`008F17E0` insertion proves the first node fields: owned copied key
+0/+4, mapped +8, next +0C, insertion at bucket head, count at receiver+4.
They call `004E7C00` with owners `00E17578`/`00E175E8`; that allocator computes
**14h-byte slots**. Only the first 10h fields are relevant here; neither Source
nor this report asserts sizeof native node = 10h. Existing query wrappers
`0048E840`/`0048E8D0`/`0048E960` construct and subsequently release temporary
eight-byte key headers. The dictionary's copied node keys have a separate owner.

## Fixture and qualification

The ignored fixture/recipe is under
`local/cc11_scene_enum_dictionary_lookup/`. Its source is
`dictionary_probe.cpp`, driven by `run_probe.py --out <fresh-directory>`.
The primary's current b02 libraries (same C++ through main `b982327be`) are frozen
in `support/`, with original-before/copy/original-after equality in
`support_freeze.json`; link uses only those immutable copies.
It freshly strict-compiles the changed reader TU, actual `scene_file.cpp`, and
fixture with MSVC Win32 `/W4 /WX /fp:strict`; no tracked tests or CMake edits.

The fixture's existing Source `SceneLexer` extracts two real installed
`global.enums` declarations: LandVehicleClasses (22 symbols) and SoldierTypes
(6). Literal installed values remain opaque mapped words. This extraction is
fixture input preparation, not a port of native enum declaration identity or
the library loader. Node strings and query strings come from the genuine current
raw-pool constructors. Nodes occupy externally backed producer-proved 14h slots;
heads/mapped/next/count fields are initialized only for the reader fixture.
This does not execute native node allocation, insertion, redeclaration or map
lifecycle. Case variants, natural collision chains, missing keys/heads, empty
null-data headers with distinct fallback cells, and long-prefix sampling exercise
the owning headers and borrowed reader path. Headers, key buffers, complete
fixture slots and opaque mapped words must remain unchanged.

`run05` passed all **68 matched original/Source reader cases**: 60 installed
authored/case-variant queries, three missing-key queries, and five empty/long-key
cases. All three fresh TU compiles, link and execution exited zero. There are
35 actual production header includes and two production CPP inputs (37 unique
production inputs); the fixture CPP, recipe and generated native-byte header
bring that explicitly counted Source/recipe/generated-file inventory to 40.
The embedded x86 manifest was extracted and verified `asInvoker`. Original and
Source outcome files have the same SHA256. The eight native call rows reverify
with zero failures. Full main registration/build remains the primary's task.

The original byte bodies use one approved common code relocation delta
`30000000h`, so `004895B0` executes at `304895B0`, for example. Every instruction
byte and relative CALL/JMP displacement is preserved. Ten reached direct
CALL/TAILJMP targets (the eight reader rows plus comparator-to-CRT and CRT-to-ASCII
tail) have checked arithmetic and resolution under that same delta. There are
no Source DataOrEmpty/comparison/uppercase hooks or missing-callee stubs. The
complete original code is copied only into checked previously free mappings,
then protected execute/read; exact bytes and protection are rechecked.
Absolute native cells `00E17654`, `00E186ED` and `0109DE1C` stay at their original
addresses, with exact PE image bytes and successful read-only protections.

Earlier native-code-address attempts `run02..run04` refused `00410000`, which
was already MEM_MAPPED/PAGE_READONLY (initial diagnostic region 9000h). No occupied
memory was changed. `run05` also checks that mapping's state/type/protection/base/
extent and a 9000h byte sample remain unchanged. This is relocated complete
original normal-body execution with invocation adapters, not execution at actual
native code addresses or a recovered whole ABI/class/game runtime.

Static
PE and saved Ghidra bytes at `0109DE1C` are zero: this is an **image-initial
zero-locale fixture domain**, not a captured live game global. The admitted
original providers are complete `00419CA0` (13 B), `00438E10` (34 B), `00BF924E`
(39 B), `00BF7FBF` (80 B), and no-call `00BF7EB7` (53 B). Nonzero-locale
`__toupper_l`/`__stricmp_l` and null CRT error paths remain unexecuted/unbound.
No Source comparator/uppercase hook may stand in for these providers.

Admission requires stable valid actual Win32 headers, closed NUL-free ASCII
keys whose lengths match their buffers, null data only for empty keys, finite
consistent chains, distinct valid empty-cell bindings and nonaliasing output.
Invalid pointers, cycles, alias-driven mutation, non-ASCII/locale changes,
allocation failure, concurrency, native EH/fault/reentry and whole ABI/gameplay
are outside the claim. Source compilation, Source fixture behavior, original
normal-body execution and whole original-game identity are separate evidence.

## Remaining traffic and enum dependencies

Original `0049CF80` traffic property application reaches the enum wrapper path
through `0048E960`, first trying LandVehicleClasses then SoldierTypes; existing
materialization `00964790`/`004B1400` is still required. The Source
`SceneTrafficHost` has no concrete production implementation and
`GameSceneContentsHost::load_traffic_block` remains unbound.

Native declaration/insert/reset owners (`008F2B90`, `008F17E0`, `008F2850`,
`004E7C00`), wrapper miss faults, enum declaration identity/context at the actual
typed parser, qualified entity roots/paths (`00925A90`), class materialization,
traffic record publication/runtime ownership (`00951220`, `004A5620`,
`004A50D0`) and the surrounding VFS/loader contracts remain explicit dependencies.
Current PropertyLibrary's narrow first-table/symbol retention and group capture
bindings do not manufacture this native dictionary storage or enum metadata.

Primary integration at `b7f1bb6c945b684dbe232f7b44f82b417af8bff5`: the complete Win32 build and all three existing CTests passed. An independent current-library probe freshly compiled 3 actual translation units and passed 68 matched reader cases. It pinned 38 Source/header/fixture inputs, three current support libraries and the original PE before and after; 36 actual compiler includes were verified. The report records native byte/literal checks, new Source COFF, manifest, logs and immutable receipts. Original ABI, whole ownership/world binding and gameplay remain unproved.
The 36 compiler includes comprise 35 production headers and one generated original-byte header; the Source/header/fixture inventory is 38, with recipe/generated inputs counted separately. PE virtual zero-filled empty cells are saved-analysis/image values, not live-game captures.
