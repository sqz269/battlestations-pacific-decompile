# Native vertex shader compilation and physical diagnostics

Addresses: `00b60f60`, `00be4430`, `00bf4f50`.

This packet reconstructs the actual vertex compiler's normal readable-input
path, its code-null return, and the physical diagnostic writers. It consumes
the existing native string pool, live renderer/VFS publications, actual VFS
manager and physical stream owners, and the installed `d3dx9_40.dll`. It does
not translate a semantic shader/stream record into native storage. The new
C++ interfaces are not drop-in binary ABI replacements or game validation.

| Routine | Original ABI | Coverage |
|---|---|---|
| B60F60 | ECX actual8h engine name, EDX profile; stack source/output; EAX HRESULT; RET8 | complete normal readable/output domain with BE4430/BF4F50 diagnostics; exact code-null return; native FH3 unwind excluded |
| BE4430 | ECX stream; stack actual8h name/optional count; RET8 | complete normal body with current28 target BF4F50; other write targets explicitly unsupported |
| BF4F50 | ECX BF50D0 actual20h stream; stack data/requested/optional count; RET0C | complete real synchronous Win32 write, including ignored BOOL and failed-handle behavior |

The saved Ghidra project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Workers made only read queries through BSP.
BE4430 and BF4F50 originally had no Ghidra function. The primary defined,
saved and re-exported `[00BE4430,00BE4457)` and `[00BF4F50,00BF4F87)` through
the official locked tool. Those are 39 and 55 bytes; the final instructions
are `RET8` at BE4454 and `RET0C` at BF4F84, both length3. Following INT3
padding is excluded. B60F60 is 604 bytes through B611BB; final RET8 starts
B611B9, length3. There are no skipped instructions or post-free gaps in
these three bodies. Byte hashes, instruction counts, calls and live/disk
comparisons are in the report and `local/vertex-byte-audit.json`.

The only direct caller is B3B3C0 at B3BAE9. B3BABE reads the generated source
from builder+50 (null becomes the actual empty literal), B3BAD1 reads the
descriptor profile pointer+38 with the same fallback, B3BADD/B3BAE1 supplies
the actual shader output cell, and B3BAE3 supplies builder+9C as the engine
name header. B60F60 saves ECX to EBP at B60F7B and EDX to ESI at B60F84;
the final RET8 establishes the two stack arguments. No profile selection,
source generation, cache orchestration or pixel compiler is added here.

The wrapper captures renderer publication F8D394 and device+1A10 before
scanning source. B1FEF0 is the existing seven-byte borrowed getter; it does
not AddRef. Compile uses first-NUL length, null macros/include/constants,
`main`, unchanged caller profile and flags1200. C2E00A/CE23EC and
C2E004/CE23F0 are the verified compile/disassemble imports. Ten compiler
and four disassembler arguments use the imported stdcall APIs. Presence of
code selects the diagnostic branch regardless of compile HRESULT.

After disassembly(FALSE, null comments), B60FF1 captures the current VFS
manager **before** allocating any name. Separate actual8h strings are
constructed in suffix, prefix, stem, complete-path order:
`shaderfx/debug/<engineName>.vsa`. Manager current virtual4 receives the
complete path and flags35. The path, stem, prefix and suffix are released
in that order, with original header words left untouched. A fifth string
copies the disassembly C string. Current stream virtual5C receives its
actual header and null count; its result is ignored. The source accepts
BE4430, which captures header length/data first, substitutes the native
0109DB64 empty literal when data is null, then dispatches current28.

BF50D0 establishes the existing physical20h layout: profile D691B0, refs4,
HANDLE8, untouched C, current position10/14 and cached size18/1C. D691B0+5C
is BE4430 and +28 is BF4F50. BF4F50 passes the requested stack cell itself
as WriteFile's count output, ignores BOOL, adds actual written bytes to
current10 and then current14 with carry, and finally publishes optional
count. It does not alter cached size. The actual WriteFile import is
CE2290; five arguments and a null OVERLAPPED are retained. Other profiles
sharing BE4430 remain explicit source boundaries when current28 differs.

After writing, text cleanup precedes the inline InterlockedDecrement on
stream+4. At zero, current virtual0 uses the same canonical VFS terminal
dispatcher; physical BF55A0 closes/recycles the stream through the existing
physical pool. Disassembly Release precedes shader creation. B61166
captures the device's table before code GetBufferPointer at B61173;
CreateVertexShader is then loaded from captured table+16C and receives the
original output pointer. Code and compiler-message releases follow the
device call, and the creation HRESULT becomes the result. No output cell
is cleared, moved or replaced by the wrapper.

| Failure/ownership boundary | Source behavior and evidence limit |
|---|---|
| Compiler returns no code | Returns compile HRESULT, preserves caller output and any returned messages. Operation destruction refuses retained COM ownership; explicit diagnostic cleanup is required. This preserves the native missing cleanup. |
| Disassembler returns null | Native would dereference null after opening the diagnostic stream; source reaches the same preceding acquisitions and then raises an explicit source error. No synthetic text or shader is supplied. |
| Reached VFS/open/write/terminal failure | Stable one-shot frame retains code, messages, disassembly, stream identity, actual name headers and call site. Failed operations cannot silently destruct or retry. The stream-reference-released flag prevents confusing a failed zero-reference terminal with an unreleased reference. |
| Failure inside existing 4261A0 helper | Existing helper's established child exception cleanup remains in effect. Entered/returned/released masks distinguish a failed child preimage from a returned owning name. No blanket claim that a freed child buffer is retained; external diagnostic resolution must establish its actual ownership. |
| Native outer FH3 | Not reconstructed. No automatic rollback or successful native unwind is invented. Borrowed source/name/output/context/module and the owning manager/device/lifetime domain must outlive the retained operation. |

`local/native_vertex_probe.cpp` is one focused fixture. It executes relocated
copies of the original B60F60, BE4430, BF4F50 and B1FEF0 instruction bodies.
The 18 direct CALL operands in the wrapper are redirected to the copied
getter, actual D3DX imports or existing source string helpers; internal
relative branches remain unchanged. Numeric data/profile addresses remain
native. For the original leg only, four invoked slots in the copied numeric
manager/physical tables are bound to callable copies or existing source
VFS-open/recycle bridges. Their original numeric entries are restored before
the source leg. Early process reservations prevented fixed-address code
mapping; no occupied mapping was removed or reused. This is an
original-wrapper/source comparison in the same real owner domain;
those bridged dependencies are shared baselines, not independently retested
original helper implementations. A real HAL device produces vertex shaders
whose function bytes match, while a real native physical mount rooted only
under `local/vertex-vfs/` produces matching `.vsa` bytes. The focused error
check verifies source code-null message retention; a throwing text allocation
retains the real code/disassembly/open physical stream and untouched output
until explicit cleanup. Original/source physical writes also agree on real
file bytes, low-to-high position carry, count publication, unchanged cached
size and failed-handle behavior. No installed shader source is modified.
The fixture uses synthetic minimal HLSL and proves this wrapper domain,
not installed material generation, native outer exception ABI or gameplay.

The source is appended to the default `cmake/startup.cmake` registry only
after an unleased check under the existing lease transaction. Build and
fixture results, hashes and the exact dependency boundary are recorded in
`reports/native_vertex_shader_compilation.json`.
