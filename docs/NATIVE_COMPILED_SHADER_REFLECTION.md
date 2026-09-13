# Actual compiled shader reflection population

Addresses: `00B3AEA0`, `00B3A750`, `00B5BC60`, `00B5B960`, `00B5B830`.

This module populates the existing raw88h D61810 reflection owner directly
from the actual D3DX constant table. Unknown FLOAT constants become actual20h
owning records in its +78 array. The accepted native owner/array APIs perform
the copies and lifetime operations. No semantic reflection vector, replacement
owner, fake bytecode or successful compiler fallback is involved.

| Entry and inclusive body | Original ABI | Coverage |
| --- | --- | --- |
| B3AEA0..B3B13A | ECX unused; stack bytecode, metadata owner; RET8 | Complete normal populated-COM-output domain; explicit failed-output/invalid-ID limits below |
| B3A750..B3A7DD | ECX owner; stack register index/count/name/rows/columns/elements; RET18 | Complete normal body; failed native acquisitions retained in a host frame |
| B5BC60..B5BD01 | ECX fresh20h record; seven stack arguments; EAX same record; RET1C | Complete normal constructor body |
| B5B960..B5B9D3 | ECX actual registry; stack actual8h name; EAX first matching actual record or0; RET4 | Complete normal readable-record/string domain |
| B5B830..B5B833 | ECX actual20h record; EAX word1C; RET | Complete |

The C++ interfaces have new typed ABIs. These names are reconstruction
hypotheses. Existing semantic projections and their evidence comments remain
unchanged. The five saved bodies have no observed missing blocks; the report
records numeric direct calls and the separate runtime COM slots.

The actual owner producer and record/array layouts are established in
`NATIVE_COMPILED_SHADER_OWNER.md`. B5BC60 adds the parameterized record
producer: first clear string14/18, write register index00/count04, copy the
actual name through41DD40 and memcpy, then write semantic1C, elements10,
rows08 and columns0C. B3A750 passes semantic37h, calls B3A660 against owner+78,
and releases its temporary name only after append returns. It does not inspect
or copy D3DX default-value pointers, parameter class or struct-member fields.

Registry lookup consumes the actual producer's +04 data/+08 unsigned count.
B5BF70's read-only producer prefix publishes D62A3C and zeros its +04/+08/+0C
array; its record construction remains a separate integration packet. No
second registry type or producer is declared here. B5B960 re-reads count and
data as it scans, compares stored lengths before `_stricmp`, and reloads the
current data pointer when returning a match. It is also called at B5A07A
inside B59E70..B5A1E1 with the same live0108FE94 publication and a temporary
actual8h header. Its zero-length case never dereferences either string data
pointer. B5B830 reads current record+1C, and B3AEA0 invokes it twice for a new
system binding.

The saved B3AEA0 decompiler aliases its large stack frame incorrectly. The
complete assembly establishes its real two stack arguments: after the native
3038h stack probe, bytecode comes fromESP+3048 before saving EBX/EBP/EDI;
the owner enters EBP fromESP+3058 after those saves. ECX supplied by all four
B3B3C0 call sites is not a receiver. Sites B3B854/B3BB69 pass pass+70;
B3BDBF/B3BE7F pass pass+74. Each pushes metadata then actual bytecode so the
callee sees bytecode first and cleans8. No source-level shader object is
converted into either native argument.

The game import thunk C2DFF2 jumps through CE23F8, whose original PE import is
`d3dx9_40.dll!D3DXGetShaderConstantTable`. The concrete import binding resolves
that named export on the caller's actual loaded module. It neither chooses an
alternate DLL nor constructs an abstract successful provider. The caller keeps
the module loaded through all retained table lifetimes. Official SDK headers
confirm actual0Ch D3DXCONSTANTTABLE_DESC and actual30h D3DXCONSTANT_DESC;
GetDesc is COM+14, GetConstantDesc+18, GetConstant+20, and Release+08.

Native reflection acquires the table, calls GetDesc, then enumerates unsigned
ordinals. Every GetConstant receives null parent and the current ordinal.
Every GetConstantDesc receives capacity256 and a real3000h descriptor buffer;
only descriptor0 is consumed. Descriptor Type==FLOAT selects registry/name
processing, independently of its RegisterSet. A known semantic writes its
truncated count to owner+3E+id and THEN its truncated register to owner+08+id
only if the current register byte equalsFF. Duplicate system bindings skip
both writes and highest-start tracking. Unknown FLOAT names append actual
records. After those calls and name releases, the current register/count are
re-read and signed highest-start comparison is applied. Sampler RegisterSet
3 contributes `1 << (register_index & 31)` for any type, including duplicate
system entries. Final end byte74 and mask84 are written in order before the
actual table's Release. Previously populated slots and material records are
preserved; invoking reflection again appends unknown records again. Empty
tables write byte74=0 and mask84=0 without reading the registry.

Native code ignores the D3DX HRESULTs and returned descriptor count before
reading outputs. This interface requires successful, populated outputs and
rejects failed/missing results before reading uninitialized descriptors. It
also rejects semantic IDs outside0..53 before out-of-bounds owner writes.
Those are explicit host admission limits, not recovered native failure paths.
The descriptor buffer is not zero-filled to manufacture success. A failure
retains any already-produced table and prior owner mutations.

`NativeCompiledShaderReflectionOperation` owns stable actual descriptor
storage, two actual name headers and the current material-append frame. The
append frame owns its raw20h temporary and SAME accepted array operation, so
the array's retained `append_source` never points into a vanished stack frame.
Both operations reject destruction while running or failed. The caller must
also retain bytecode, owner, context, registry publication/owner and D3DX
module, and exclude owner terminal admission. There is no retry or automatic
rollback. Unlike native private SEH cleanup, host failures preserve acquired
state for the outer compiler continuation. Successful paths release temporary
names in native order and release the actual constant table exactly once;
they neither retain nor decrement the target owner's raw+04.

The ignored fixture uses the real installed D3DX9_40 compiler and constant
tables, two caller-owned canonical raw88h owners, and the actual string pool.
Its single raw10h registry prefix and20h record are explicitly fixture inputs,
not a recovered B5BF70 producer. Original/source execution covers nine byte
ranges (1607 bytes,538 instructions), including original stack-probe bytes
inside the fixture only; production does not port the CRT stack probe.
The fixture compares raw system bytes, unknown records, name-release order,
repeat/empty reflection and canonical teardown. A second invocation of the
same fixture injects a real string-allocation failure inside B3A660, verifies
the live table/temporary/name/array identities, then confirms failed-frame
disposal reaches its terminate handler (exit77). It does not claim cleanup or
native exception parity for that deliberately retained failure.

Validation details and exact call rows are in
`reports/native_compiled_shader_reflection.json`. The native B3B3C0 compiler
tail, actual registry production and any separate COM shader owners remain
external integration work. This is neither compiler-pass success nor game or
rendering validation. Workers performed no Ghidra mutation, re-import or game
installation write.

The resumed integration check compiled both the accepted owner and this
reflection source into the actual `bsp_core` target through an ignored local
additive CMake include. The existing fixture then linked that library only
and passed its normal and retained-failure modes; CTest passed 1/1. The real
startup registry was unchanged because another worker held its lease.
Default source registration remains a primary integration step; this local
build does not claim that the worker baseline registers either source.
