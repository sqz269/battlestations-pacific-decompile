# Native property tree library storage

`008F5670..008F5697` is one distinct complete constructor: 39 bytes,
12 instructions, no calls or relocations. Whole SourceCOFF and linked bytes
exactly equal the live Ghidra and installed PE bytes. SHA-256:
`486d83626dc3930fe1a30a4613d80b8a3ff0ea9cb407046b2ea0b4d64beeb2b5`.
The Source name is a descriptive storage hypothesis; the worker preserves
the current Ghidra name and makes no analysis mutations.

The naked MSVC Win32 single-ECX initializer preserves EDI, requires caller
DF=0, returns the same base in full EAX, and uses plain RET. It stores D1650C
at outer+0, D162C8 at +4, zero count at +8, and uses REP STOSD for exactly
64 heads at +Ch..+108h. The root is 10Ch (268 decimal); its map is 108h
(264 decimal) borrowed at root+4. These are actual DATA phase words. No
class/vft dispatch, allocation/free, defaults, guards or lifetime providers are
inserted into the constructor. The map is never allocated or freed separately.

The actual original caller pushes10Ch at `004D308F`, calls `00BF681B` at
`004D309A`, moves that same EAX allocation into ECX at `004D30B1`, and calls
this constructor at `004D30B3`. It publishes E18678 at `004D30CB` and calls
loader8F7100 at `004D30D0`; those parent/global/loader contracts remain unclosed.
This is a separate allocation from the enum registry's first10Ch allocation.
Earlier 4D3069 fragment34 and map32/outer39/ordinary11 are not counted again.

One fresh ignored family `local/cc11_property_tree_library_storage/run01`
compiles exactly three TUs: the constructor, current `singleton_lifetime.cpp`,
and the probe. Two genuine separate roots are allocated through freshly
compiled `singleton_lifetime_allocate({object,0x10c,0x10c})`. On each same
actual address and poison input, Source39 and unchanged Original39 RX code
compare full EAX identity, EDI sentinel, ESP, DF0, every268 output byte and
the entire other live root as a guard. The Original bytes and RX protection
are checked after execution. Both empty roots remain live through all checks;
explicit currentfree releases only their base addresses afterward. No dead
root read, secondfree, owning-header overwrite, original destructor or loader
execution occurs. Only successful current allocation/release is runtime-tested.

The strict Win32 build uses `/O2 /GL- /EHsc /MD /GS- /W4 /WX /fp:strict`,
actual showIncludes and an embedded asInvoker manifest. WholeCOFF and linked
constructor equality gate execution. Fresh current allocation/free bodies are
90 bytes/34 instructions and 6 bytes/one instruction; their complete normalized
COFF/linked bodies and ordered relocations match, with actual malloc/free IAT
addresses verified against PE imports from `api-ms-win-crt-heap-l1-1-0.dll`.
These current CRT providers do not bridge the original historical heap or EH.

Observed inputs: 8 project headers,
166 host headers, 13
searched libraries, two production CPPs plus fixture and two recipes.
The three fresh objects contain 330 code sections,
21797 bytes and 814 relocations;
95 unique linked application spans
(10589 bytes) have whole-body evidence.
Source/header/tool/backend/library hashes are bookended. The existing native
call verifier confirms all three exact allocation/constructor/loader rows.

Current supports were frozen before compiling from Root's dd9061fc5 integrated
Win32 build/all3CTest (`cc11_base_map_group_integrated_build.log`): Core
`be0bb037e18debf8fbd8e56f2bcfb271480d1d6f663628c1cca69dbe99baddc4`,
Lua `e9786d9484ea0689bf98ce2ce83437e749d60a05859cfe028e356f12442c08de`,
zlib `c6b5a17d184c45a80e5a9fb6d269a35b85c9732e434396cb31dcc9754a37536f`.
Current c329ab385 registers metadata only. Frozen copies were consumed; any
mutable main support state is reported separately. All341 earlier consumed
pins and 5560 historical files are unchanged,
including old20a5 copies, finalmaprun01 receipt and both earlier read-only audit
failures. No old family was replayed. This packet passed its first compile/link/
execution attempt and introduces no tracked tests.

Actual D1650C slot0 targets8F6790; D162C8 slot0 targets8F4DA0. D16508's
adjacent DWORD is separately written D1650C profile storage, so adjacency does
not establish a second CEnum virtual method. The constructor does not call
either profile. Neighbor8F6790 scalar, ordinary8F56A0 and clear8F4C50 remain
unready pending genuine propertymap/node/payload/pool/string ownership and
release contracts; no CE7514/CE78BC or enum pool substitution occurs.
Original historical allocation/free/failure ABI, private EH, class/vft/global
startup, parentfactory, world and game remain outside this leaf's proof.
Root independent review/integration/CMake/fullbuild/CTest/annotations are pending.

Sealed receipt SHA-256: `800f6bfbb68d03bf2d438c0a811c4c6c4c6bc1086c55c4e47980277ace0833df`.
Complete COFF/link receipt SHA-256: `dcd1faa07800f8f375c4d8add60e569660e0d99f2c3784ef037d307a71ebf422`.
