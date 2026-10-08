# Native type0 property record partial storage (CC12)

`initialize_native_scene_property_record_type0_storage_008ef140` reconstructs the whole native `[008EF140,008EF16D)` body: **45 bytes, 14 instructions, zero CALLs**. Its SHA-256 is `55f7af3bf7ae1662a71c0e72fa4ff5e411b43754989762c7b025a63a3e5f814e`. `BSP_ScenePropertyRecord_ConstructType0` is a descriptive hypothesis. This interface initializes supplied raw storage; it does not establish a complete native class or its ownership.

The physical ABI takes the root in ECX and one value DWORD at entry ESP+4. It returns the full root in EAX, zero ECX and the value bits in EDX, preserves all nonvolatile registers, and ends with RET4. The public three-formal `__fastcall` declaration deliberately includes an unused EDX formal so `value_bits` remains on the stack. The body overwrites incoming EDX. A two-formal declaration would place the value in the wrong location.

Supply fresh, unowned writable storage spanning at least 56 bytes. The stack value is stored at +0Ch without dereferencing. It is not an owner argument: owner +30h is zeroed. The leaf uses neither DF nor ES and contains no allocation, conditional branch, guard, unresolved call or dispatch. Exact byte encodings retain the native XOR form and leave no relocations.

| Root bytes | Effect |
|---|---|
| `[00,04)` | Literal native DATA phase bits `00CE89D4` |
| `[04,08)` | Type tag zero |
| `[0C,10)` | Stack `value_bits` |
| `[18,28)` | Four DWORD zeros |
| `[2C,2D)` | Byte 1; its semantic meaning remains unestablished |
| `[30,38)` | Zero owner and ordinal DWORDs |

These writes total **37 bytes**. The remaining **19 bytes** at `[08,0C)`, `[10,18)`, `[28,2C)` and `[2D,30)` retain the caller's contents. The profile's actual native DATA DWORD maps slot0 to `004E6730`; that is evidence about the original image, not a Source vft or Source dispatch binding.

The genuine direct caller starts at `008F4F60`. Its type0 jump-table DWORD at `008F52B8` selects `008F4FF9`. The bounded `[008F4FF9,008F502D)` witness is 52 bytes / 17 instructions: PUSH38h at `008F4FF9`, CALL operator_new `00BF681B` at `008F4FFB` ending `008F5000`, caller cleanup and null test, PUSH source value at `008F500E`, and the same allocated EAX moved to ECX at `008F500F`. CALL initializer at `008F5011` ends at `008F5016`; the caller then uses returned EAX to copy the source ordinal. Whole clone/EH/class ownership remains unexpanded. The separate whole 93-byte / 34-instruction setter `008F3710` inlines type0 field writes and forwards its actual 38h allocation to external `008F33F0`; it does not call this initializer.

The fresh worker family is `local/cc12_property_record_type0_storage/run03`, prepared from `33d1086d3`. It compiled exactly three new translation units: this Source CPP, current `singleton_lifetime.cpp`, and one ignored probe CPP. It linked zero BSP archives and zero old objects. Strict MSVC Win32 used Visual Studio18 Community 14.51.36231, SDK10.0.26100.0, `/MD /O2 /W4 /WX /fp:strict /permissive-`, and an embedded asInvoker manifest. Compiler/backends, 209 consumed compiler headers, eight project headers and six searched system libraries were pinned and checked after the run.

Before either constructor call, the static gate checked every Original45/COFF45/linked45 byte, zero CALLs/relocations, one linked occurrence, current canonical allocator/free code and IAT targets, and fresh ordinary plus raw caller assembly. The runtime gate checked the loaded I386/PE32 UCRT, malloc/free exports and actual IAT values, module ownership, and real handle-based physical file identity. Its logical `C:\WINDOWS\System32\ucrtbase.dll` path resolved to the pinned SysWOW64 file, identity `e8dcd6e0:00050000002c2ff6`; no guessed path normalization was used.

One Source call with value `12345678h` and DF0, plus one unmodified Original RX45 call with value `9ABCDEF0h` and DF1, used two genuine current canonical 56-byte malloc roots. Both full buffers passed, including every untouched byte and preservation of the other live root. The probe captured full EAX/ECX/EDX, nonvolatile GPRs, ESP/RET4, defined XOR flags (CF0/PF1/ZF1/SF0/OF0), DF/ES preservation and external capture/stack canaries. Undefined AF was excluded. Capture cleared DF before returning to C++. Only actual raw allocation roots were explicitly freed; no destructor, profile dispatch, interior free or freed-memory read occurred. The probe did not inspect adjacent allocation red zones or heap metadata. The literal whole body establishes its intended store bounds statically.

The sole successful prepare/build/launch/seal recipes and 298 artifacts are immutable. Earlier run01/run02 preparation failures are retained: the local read-only PowerShell preflight was first blocked by execution policy, then `Get-FileHash` was unavailable in the inherited x86 module environment. Both stopped before compilation or canonical allocations/constructor calls. The accepted run used a process-scoped policy override and .NET SHA-256; it was never replayed. Postprocessing performed read-only native and file bookends.

The 6,209 historical artifacts, 341 prior consumed pins, old frozen Core/support associations, sealed readiness audit and both accepted property-bag constructor families remain unchanged. Core is external build context and contributes no BSP linkage to this component family. See the machine report for exact manifest/receipt hashes and artifact paths.

This result establishes a whole raw partial-storage implementation and a current-heap component fixture with its physical ABI. Native record/class dispatch, scalar30/thunk11/type158 teardown, ordinary owning clear `008F3F30`, actual global pool `E175B0`, external insertion/key ownership, original CRT/EH/class/static lifetime, nested114/world/game runtime remain separate admission gaps. The 38h record allocation, 38h pool owner and 114h bag are distinct contracts. Root owns independent integration, full main build/tests and saved Ghidra annotation; this worker changed only its four owned files.
