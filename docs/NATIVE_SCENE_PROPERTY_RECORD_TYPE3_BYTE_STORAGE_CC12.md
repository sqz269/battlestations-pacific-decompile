# Native scene property record byte storage, CC12

`initialize_native_scene_property_record_type3_byte_storage_008ef1f0` implements the complete
`[008EF1F0, 008EF221)` body: **49 bytes, 14 instructions, zero CALLs**. The descriptive name
is a hypothesis. This is a raw partial-storage initializer, fixture-tested through its physical
Win32 ABI; native class ownership and game behavior remain unvalidated.

The Source declaration is `void* __fastcall initialize_native_scene_property_record_type3_byte_storage_008ef1f0(void*, std::uint32_t unused_edx, std::uint8_t value_bits) noexcept`.
ECX holds the actual root. One rounded DWORD stack slot supplies only its low byte at entry
ESP+4; the upper three bytes are ignored. RET4 consumes that slot. EAX returns the full root,
ECX returns zero, and EDX is neither read nor written. The explicit unused EDX formal preserves
this stack contract. All nonvolatile registers, DF and ES remain unchanged. XOR ECX,ECX defines
CF0/PF1/ZF1/SF0/OF0; AF is undefined and excluded. The body uses neither DF nor ES and needs
no direction precondition. It has no guard, allocation, normalization or Boolean conversion.

Supply at least 56 fresh/unowned writable bytes. The byte at +C is written **before** the
literal CE89D4 phase bits and DWORD tag 3. Byte +2C becomes 1, with no assumed semantic meaning.
Owner +30 and ordinal +34 become zero. Payload +C is not an owner and is never dereferenced.

| Written half-open byte range | Result |
|---|---|
| [00,08) | Native DATA bits CE89D4, tag 3 |
| [0C,0D) | Raw low8 value_bits |
| [18,28) | Four zero DWORDs |
| [2C,2D) | Byte 1 |
| [30,38) | Zero owner/ordinal DWORDs |

The body writes 34 bytes and preserves 22: `[08,0C)`, `[0D,18)`, `[28,2C)`, `[2D,30)`.
CE89D4's separately checked DATA DWORD equals 004E6730; it is retained as literal native
identity, with no Source vft, dispatch or owning-lifetime binding.

The whole native SHA256 is `77c57bbbe4e8f241db03c9753b6655903eb4e6028d32b1c6b256d083ce32e5ab`.
The exact 49-byte hex is:

```text
8bc18a4c240488480c33c9c700d489ce00c740040300000089481889481c894820894824894830894834c6402c01c20400
```

The genuine clone entry 008F4F60 has a bounded 45-byte/13-instruction dispatch witness. Its
type 3 table DWORD at 008F52C4 reaches 008F4F8D. The selected `[008F4F8D, 008F4FC2)` witness
is 53 bytes/17 instructions, SHA256 `4a5090a1cf28d2612fa8c382589c0aa6a88c365e6e418021a896074495c0a1f0`.
PUSH 38 is at 008F4F8D; CALL 00BF681B at 008F4F8F ends 008F4F94, followed by ADD ESP,4 and the
allocation-null branch. MOVZX ECX,byte[ESI+C] at 008F4F9F and PUSH ECX at 008F4FA3 supply
the actual native byte. MOV ECX,EAX at 008F4FA4 forwards that same allocation root.
CALL 008EF1F0 at 008F4FA6 ends 008F4FAB. The caller then reads the source ordinal at 008F4FAB
and writes [EAX+34] at 008F4FAF. Whole 854-byte clone/EH/other arms remain unexpanded; these
allocation and ordinal witnesses establish no complete 56-byte native class ownership.

The unique fresh worker family is `J:\PROG\battlestations-pacific-decompile-cc12_property_record_type3_byte_storage\local\cc12_property_record_type3_byte_storage\run04`. It compiles exactly the new Source CPP,
current `singleton_lifetime.cpp`, and a new ignored probe CPP with MSVC 18 Community 14.51.36231,
SDK 10.0.26100.0, x86 /MD/O2/W4/WX/fp:strict/permissive-, and embedded asInvoker manifest.
It links zero BSP archives and zero old objects. Actual four compiler backends, tools,
209 compiler headers, eight consumed project headers, six system libraries and all Source
inputs are pinned. Compilation/link/static gate/sole execution all passed.

Before execution, complete Original 49/Source COFF 49/unique linked 49 equality, zero Source
CALLs/relocations, actual ordinary compiler caller frames, and the raw one-stack-DWORD/RET4
contract passed. The Source symbol is
`?initialize_native_scene_property_record_type3_byte_storage_008ef1f0@bsp@@YIPAXPAXIE@Z`.
The two actual compiler callers directly PUSH their original DWORD slot containing the uint8
formal. Symbolic frame/stack tracking proves its low8 identity, ECX receiver, unused EDX and
balanced caller return/EH frame; upper24 padding is unspecified and no zero extension is
claimed. The genuine native clone above independently contains actual MOVZX.

The sole native fixture allocates two genuine 56-byte current canonical malloc roots only
after validating allocator/free COFF-to-IAT targets, loaded I386/PE32 UCRT exports/module
ownership and physical-file identity. The logical loaded System32 name resolves through
real file handles to the pinned SysWOW64 image, ID `e8dcd6e0:00050000002c2ff6`, SHA256
`60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`.

Source receives raw DWORD 89ABCD80/DF0 and Original receives 123456FF/DF1; low 80/FF are preserved
without Boolean conversion despite nonzero upper 24 bits. Source/Original use different 81/39
poisons and an unmodified whole 49 RX Original stub. All 56 bytes, every 34 written/22 untouched
byte, full EAX/ECX/EDX and nonvolatile GPRs, ESP/RET4, defined XOR flags excluding AF, DF/ES,
every byte of the other live root and external capture/stack canaries passed. Only the still
raw actual allocation roots are explicitly currentfree released after checks. No destructor,
phase dispatch, interior free, freed-memory inspection, heap-metadata access or adjacent heap
red-zone proof is claimed. Static whole stores establish the intended write extent.

The immutable seal inventories 297 artifacts plus two metadata seals (299 total). Receipt
SHA256 is `5e1400c6278dfb3cc3ae648eb2a6c2a6f9dd1a4d24fbc8c14a41ccd489cd6512`, artifact manifest
`19e1311d969b87dfc44cf1da1f0b8ca20f83bc8d848dc9802644679e9d29263e`, and final input manifest
`4c5e3a6570a4346e293bbf083f477e18d0ec1048ca63560ec454999e4e87e8b6`. Schemas are
`bsp.cc12.type3_byte_storage.receipt.v1`, `bsp.cc12.type3_byte_storage.artifacts.v1` and
`bsp.cc12.type3_byte_storage.family.v1`. The report records whole-body, callers, toolchain,
observed results and exact seal paths. The accepted prepare/build/launch/seal recipes and
all earlier successful families are immutable and must never be replayed.

Preserved stopped stages contain zero constructor executions: run01 exceeded legacy
compiler MAX_PATH; run02 compiled/linked but its first uint8 gate incorrectly required
MOVZX instead of tracking the actual forwarded DWORD slot; run03 stopped before compilation
when unrelated main enum-dictionary Source/header content advanced. Main commit
998782c250618268e162b5dfb5be139788cc2f99 is explicitly unconsumed current context. Each of
those two prior 341 historical original paths and old hashes remains associated with its
existing matching immutable run02 frozen file. No accepted receipt was repinned or changed.
All prior 6209/history 341, constructor 282+282, type 0 worker/primary 300+300 and current readiness
40 artifacts plus two seals are checked; consumed new Source/header/toolchain hashes match
before and after. Current Core archives are external build context only and are not linked.

Source CPP SHA256 is `ffd50ed25d5caf4b120ae4f02f57c33509f72a98b0d0e522b37822ea4afebd94`; header is `2e6f87ee0429e245ae150f1b677416256e4e4c2a1f2b21db07b722e074a46288`.
There are zero new tracked tests, Ghidra writes, shared CMake/config/ledger edits or original
game writes in this worker patch. Root owns independent complementary validation, full
main Win32/all3 checks, Source metadata, Ghidra annotation/save/export/snapshot and publication.

Remaining prerequisites include original BF681B allocator/CRT/newhandler/EH and clone handler
CA4B6D; whole clone/factory/parser and record class ownership; actual scalar 4E6730,
thunk 8F0DE0, record cleanup 8F0640, nested 0x114 lifetime and ordinary clear 8F3F30; initialized
E175B0 pool/list/CS/pages/14-byte slots and external insertion 8F28F0/33F0 with actual key/payload
owners. Source class/dispatch/destructor, original heap/static runtime, world and game
admission remain separate. DATA identity, allocation size and this raw fixture do not satisfy
those dependencies.
