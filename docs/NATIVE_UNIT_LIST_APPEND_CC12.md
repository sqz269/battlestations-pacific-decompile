# Raw unit-list append at 00484540

`append_native_unit_list_00484540` reconstructs the complete **87-byte / 36-instruction** entry at `[00484540,00484597)`. Its descriptive name is a hypothesis. The raw list operation uses the existing canonical allocation service through one private, real CDECL size adapter; the adapter receives no native reconstruction credit.

## Native evidence and exact scope

The installed original executable and live saved `bsp` analysis agree on all 87 bytes. The original executable SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`; the complete entry SHA-256 is `4b5cdb5a7a0792b0f0e94b3633502bc1a93a4092d79ee8d44adc733012fbc313`. The supported read-only Ghidra gateway verified `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, before both live-byte batches. The complete entry matched before and after the fixture.

All **83 bytes outside offsets 7..10** remain literal. The only COFF relocation is `IMAGE_REL_I386_REL32` at offset 7, the operand of the CALL at `00484546`. In the original that CALL targets `00BF681B`; Source binds it to `allocate_native_unit_list_bytes(size_t)`. Both native return paths, the null-allocation branch and its following native dereference, every DWORD store, and the fresh count/tail reads after allocation remain present. Source adds no guards, default result, alternate heap, callbacks, or `noexcept` wrapper.

Only this raw append entry is implemented here. The command-target parent initializer, registration/unregistration wrappers, owner class/world relations, and `004837D0` erase entry remain separate evidence boundaries. In particular, fixture teardown does not establish native erase behavior or its omitted saved-listing epilogues.

## Physical contract

The public Source signature uses MSVC Win32 fastcall: actual 12-byte list storage in ECX, unused EDX padding, and one actual payload DWORD on the stack. The list layout is `{count, head, tail}` at offsets `0/4/8`; allocated nodes are `{previous, next, borrowed_payload}` at `0/4/8`. Existing nodes and the caller-owned list root must be live, coherently linked, and in the canonical current-CRT allocation domain. Payload lifetime remains with the caller.

The native body allocates 12 bytes, retains the new pointer in ECX, stores the actual borrowed payload, links the old tail or initializes the empty head, then increments the actual count. It returns the payload in EAX and the newly allocated node in ECX, with EDX zero, ESI/EDI restored and `RET 4`. EBX and EBP are preserved through the real CDECL provider chain. Defined arithmetic flags come from the final count `ADD`, after the allocation call; no blanket floating-point or direction-flag preservation is asserted across CRT code. DF is clear on fixture entry, as required by the CRT calling convention.

The private adapter constructs the actual request `{SingletonAllocationKind::object, n, n}` and returns `singleton_lifetime_allocate` unchanged. Its normal return preserves the allocator's actual EAX result. The current canonical allocator reads `host_bytes` at request offset 8, calls real `malloc`, retries through real `_callnewh` if needed, and throws `std::bad_alloc` when that handler declines. These branches are retained statically. The fixture admits only normal non-null allocations; it does not force OOM, install a new handler, manufacture a null-return allocator, or establish exhaustion/reentry/private exception behavior. Historical original private-CRT and EH identity remains unbound.

## Fresh worker build and static inspection

The immutable worker family is:

`J:/PROG/battlestations-pacific-decompile-cc12_raw_unit_list_append/local/cc12_raw_unit_list_append_worker20261008a`

The family started from main `3f5d18acf`; later main changes do not rewrite this frozen Source association. It compiled exactly three fresh translation units: this Source with its private adapter, unchanged current `src/singleton_lifetime.cpp`, and a fresh ignored probe. There are zero old objects, BSP archives, or new tracked tests. Each TU compiled once and the executable linked once, without failed build stages.

MSVC 14.51.36231 targeted I386 with `/std:c++17 /MD /O2 /W4 /WX /fp:strict /permissive- /EHsc /Gy /GL-`. The link used `/OPT:REF /OPT:NOICF /INCREMENTAL:NO /BASE:0x22000000 /DYNAMICBASE:NO /MANIFEST:EMBED`. The embedded manifest was parsed and required `asInvoker`, `uiAccess=false`, before launch. The compiler/backend/link/resource tools, recipe, three TUs, actual Source headers, 184 consumed headers, seven resolved input libraries, original PE, and executable were pinned. Candidate header hashing includes extensionless C++ headers.

| Fresh linked body | Bytes | Instructions |
| --- | ---: | ---: |
| Raw append Source | 87 | 36 |
| Private size adapter | 61 | 17 |
| Canonical allocator | 90 | 34 |
| Canonical free | 6 | 1 |
| Ordinary CDECL-to-fastcall caller | 18 | 5 |
| Physical capture caller | 140 | 52 |
| `std::bad_alloc` constructor | 24 | 6 |

These counts were independently derived from this family's fresh objects and linked bytes. They are not copied from the older full-build 90-byte/37-instruction provider or another standalone fixture. Full COFF relocation records and linked disassemblies are retained. The first six bodies were checked against complete COFF bytes and every resolved relocation; the seventh cold constructor was recovered and pinned as a complete additional live span. The raw Source has one unique normalized linked occurrence.

The entire adapter, allocator/free and both caller bodies were inspected before execution. The adapter's separately linked 14-byte/4-instruction security-cookie helper was also inspected: its successful comparison path leaves EAX intact. That helper is covered by whole executable file pins, not an additional runtime live span. The seven tabled spans are compared live before the first target call and after final teardown.

## Actual CRT provider gate

Before **zero target allocations/entries**, the process checked each actual `malloc`, `free`, and `_callnewh` IAT value against `GetProcAddress` and the physical export RVA of its mapped I386 module. It required `MEM_IMAGE`, matching allocation base, live/physical I386 PE metadata, matching mapped/opened NT paths, physical volume/file identity, full DLL SHA-256, and a 32-byte export prefix adjusted only for actual image relocations. Open read-only file handles excluded write/delete sharing through final teardown; all these provider facts were checked again afterward.

All three heap API-set imports resolved to the same physical `C:/Windows/SysWOW64/ucrtbase.dll`, SHA-256 `60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`. WOW64 reported a `C:/WINDOWS/System32/ucrtbase.dll` loader path, while both the mapped image and opened physical file resolved to `\Device\HarddiskVolume3\Windows\SysWOW64\ucrtbase.dll`. The physical volume was `3906787040`, file index high/low `327680/2895862`, size `1114240`. This resolves the actual loaded file rather than equating the loader path string with physical identity.

This provider gate precedes target allocation and append; process startup and probe APIs may themselves use CRT services. The gate proves the current providers' identity, not original private-CRT equivalence or dynamic execution of the new-handler path.

## Sole accepted execution

One process ran once and passed. The canonical-current-CRT-bound Original copy changes only its four CALL-operand bytes to the **same actual compiled Source adapter**. It is not an unmodified original private allocator execution.

Original and Source each performed an empty append followed by a nonempty append using the same actual declared aligned caller root and the same actual declared payload objects. The nonempty prefix was created by the preceding complete admitted append using genuine canonical allocation; no node, allocator address, vector projection, or return value was fabricated. The ordinary compiler-emitted fastcall caller then performed one additional empty Source append. Total: **5 target entries, 5 returned allocations, 5 matching frees**.

The four raw captures observed EAX equal to the actual payload, ECX equal to the actual new node, EDX zero, preserved EBX/EBP/ESI/EDI, and after-ESP equal to before-ESP plus four. All ended with EFLAGS `0x202`; defined arithmetic mask `0x8D5` was zero, matching actual postallocator count transitions `0 -> 1` and `1 -> 2`. This is evidence for those admitted transitions, not a claim about unexecuted overflow states.

Each observation checked actual count/head/tail, all new-node fields, the existing head's previous/next/payload fields, caller-owned declared guards, and unchanged borrowed payload bytes. Natural allocator reuse was accepted without forcing equality of new-node addresses across phases. Teardown first detached/reset the real root, then read each next pointer while live and freed each exact produced node once through canonical free. No freed memory was read.

All complete runtime live-code spans, bound Original bytes, current CRT gates, Source/tool/header/library/object/executable pins and original live/disk bytes passed before/after checks. All 271 earlier artifacts were unchanged: the prior 234 pins plus all 37 files of the readiness family, including first seal, clarification and final seal. Earlier recipe/executable families were never replayed.

The accepted core family seal is `e6476206fdc56e93d51742b7645dfb5ad282a191bc345d8a9f61ac0d2e7aa5a4`. The report records the actual Source/header hashes and per-artifact receipt links. This establishes fresh standalone strict compilation, whole-body evidence and the bounded normal current-CRT fixture. Root owns shared CMake/ledger/Ghidra integration and full Win32/CTest validation. Startup, gameplay, parent ownership, original private allocation/EH, and arbitrary binary drop-in compatibility are not established by this worker fixture.
