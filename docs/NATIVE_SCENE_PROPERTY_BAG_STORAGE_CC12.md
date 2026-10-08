# CC12 raw scene property bag storage at 008F41A0

This packet implements one distinct whole **61-byte / 15-instruction** native
body, `[008F41A0,008F41DD)`, with SHA256
`4d8da1738a25d34afd2e5cba2773ad15e2642ae2ae55831e304751ac62b43420`.
`BSP_ScenePropertyBag_Construct` remains a descriptive hypothesis. Source is an
exact naked literal encoding; it introduces no calls, relocations, globals,
synthetic class, profile dispatch, or missing-callee expansion.

The public Source declaration has three `__fastcall` formals: actual root,
**unused EDX**, then owner bits. This is deliberate. The physical constructor
receives the root in ECX and one DWORD owner at entry ESP+4; PUSH EDI changes the
owner read to ESP+8. It returns the full root in EAX and uses RET4. An ordinary
two-formal fastcall would put the owner in EDX and would be incompatible.
Incoming EDX is overwritten with ECX. REP STOSD leaves ECX=0 and EDX=root;
EDI is restored, and EBX/ESI/EBP are untouched. DF=0 and valid flat ES are
preconditions. The body has no CLD or validation.

The routine initializes every byte of supplied fresh, unowned 0x114-byte storage:

| Root offset | Bytes | Native write |
| --- | ---: | --- |
| +0 | 4 | Literal DATA phase address 00D16504 |
| +4 | 4 | Literal DATA phase address 00D162C4 |
| +8 | 4 | Zero count |
| +0C..+10B | 256 | 64 zero DWORD heads through REP STOSD |
| +10C | 4 | Zero ordinal |
| +110 | 4 | Borrowed owner bits, never dereferenced |

Root+4 is a borrowed 0x108-byte map interior ending before +10C. It is never
separately allocated or freed. The individual original DATA DWORDs resolve
00D16504 slot0 to 008F59E0 and 00D162C4 slot0 to 008F4170. These are DATA-only
observations; the Source body never dispatches them, and no C++ vtable/lifetime
binding is admitted.

The bounded genuine allocation witness is `[00469CCE,00469CF4)`, 38 bytes /
10 instructions, SHA256
`c4ea45908d59f7174d9cd983062388bc1b24d2959ca661bd5899ea71ed62c90a`.
It pushes 114h, calls 00BF681B at 00469CD3, balances that cdecl argument, pushes
owner zero at 00469CEB, moves the same allocated EAX to ECX at 00469CED, and
calls this constructor at 00469CEF. 00469CDD is inside a MOV and is not an entry.
This witness does not admit the enclosing parser/factory/heap/EH behavior.

The sole executed fresh worker family is
`local/cc12_property_bag_storage/run04`. It compiles this new constructor,
the current `src/singleton_lifetime.cpp`, and one ignored probe: exactly three
translation units and zero BSP archives. Compiler/headers/backends/system
libraries are pinned before and after. It uses Visual Studio 18 Community,
MSVC 14.51.36231, SDK 10.0.26100.0, Win32 `/MD /O2 /W4 /WX /fp:strict
/permissive- /EHsc /GL- /GS-`, and an embedded `asInvoker` manifest.

Before any Source/Original construction, the static gate compares the whole
Source COFF and unique linked 61-byte body to the unchanged Original; verifies
no constructor CALL/relocation, whole linked current allocator/free bodies,
retained generated three-formal Source/Original caller argument placement,
and the actual physical capture caller. The runtime gate resolves the exact
malloc/free IAT slots through their heap API-set and the loaded x86 UCRT exports,
checks their loaded I386 module identity, and proves that the actual module path
and pinned physical UCRT path refer to the same volume/file index and byte size
through two real file handles. A read-only x86 preflight also proves their file
hashes equal: WOW64 reports logical System32 while the pin names SysWOW64. An
unmodified Original61 stub is changed from RW to RX without a callee patch.

Exactly two genuine current allocator requests obtain actual 276-byte roots.
Source constructs the first once with a null owner. Original constructs the
second once with the first still-live root's address as borrowed owner. The
fixture checks all 276 bytes, full returned EAX/root identity, specified ECX/EDX
outputs, EDI/all nonvolatiles, balanced ESP/RET4, DF0, the flat ES descriptor,
other live root preservation, and capture/stack canaries. Arithmetic flags are
not compared. Static complete-body stores/REP bounds prove the intended write
extent. Runtime **adjacent allocation red zones are not proved**: no allocator
metadata or bytes beyond either requested block are read or altered. Explicit
current free releases only still-empty actual allocation roots, in reverse
order, after checks; root+4 is never freed, and freed memory is never read.

The 6,209 prior immutable artifacts, 341 prior consumed pins, sealed constructor
audit, array frozen Core071cf8b4 context, and earlier be0/20a5 context remain
preserved. Current main Core is external build context only, not a linked input;
no old consumed receipt is repinned. Failed pre-execution run01 retained three
successful compilations and a catalog error for extensionless `cstddef`;
run02 retained three successful compilations/linking and an invalid manifest
option rejected by mt. Their objects, frozen recipes and failure records remain
immutable. Run03 compiled/linked successfully; its COFF-addend and current
compiler EBP/FS caller-model inspection errors were recovered through separately
frozen read-only scripts without rebuilding. Its fixture stopped at literal
UCRT path equality before canonical allocation or either constructor. That
failed executable is preserved and never replayed. The actual file-identity
proof replaces that overstrict path predicate in fresh run04 without replacing
the provider. No old or accepted family is replayed. Successful prepare/build/
launch/seal recipes and complete before/after artifacts are frozen in run04.

Machine evidence and exact receipt hashes are recorded in
`reports/native_scene_property_bag_storage_cc12.json`; the worker receipt,
artifact inventory and complete COFF evidence remain in the ignored family.
Independent root review/fresh fixture, full CMake build/existing checks,
controlled Ghidra annotation/save/export/snapshot, and main publication are
integration responsibilities. This packet does not admit the native whole
class, ordinary/scalar record paths, whole type158, nested lifetime, historical
heap, EH, parser/factory, world, startup or game behavior.

## Primary integration

Distinct whole61B15 raw supplied-storage constructor; actual ECX root, unused EDX C++ padding formal, owner DWORD on stack, RET4, EAX root/ECX0/EDXroot and EDI preservation; DF0/flatES required. All276 fields and literal DATA phases verified. Main independent fresh3TU Source-live-owner/Original-null cases complement sealed worker Source-null/Original-live cases; each uses two genuine exact276-byte currentheap roots, raw caller/capture/wholeCOFF/unique-linked gates BEFORE sole execution. Loaded I386 UCRT logical System32 and pinned SysWOW64 files proved identical by actual volume/file identity and bytehash, with actual malloc/free IAT/export/module gates. All prior artifacts preserved; no old family replay or new tracked tests. Other live allocation/stack/capture guards checked, adjacent allocation red zones not proved. Explicit currentfree of empty actualroots only; borrowed root+4 never freed. No profile dispatch, native class/historicalCRT/ordinary/scalar/type158/nested/parser/factory/EH/world/startup/game admission.

Exact main Source build `0fa8ebb1b5e8e0d65b9b2f62df0e1242f197d140` (current main has only subsequent documentation/report changes) passed all three existing CTests. Independent three-TU family: `local/cc12_property_bag_storage_primary/run01/receipt.json`; Source nonzero live owner and unmodified Original null-owner both passed. Core `17c8897939e01c4adc9e270062db268b7f16098e7f164c2cb89fc93ec49aa862` is full-build context only; the component consumes no BSP archives. Saved analysis receipts follow in the report.
