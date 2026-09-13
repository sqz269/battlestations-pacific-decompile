# Native render queue constructor

Addresses: `00B1F280`; consumed `00B28A90`, `00B1F150`, `00B1D590`, `00B1C3C0`; producer `004C11F0`; EH dependencies `00CBCBF0`, `00CBCBF8`, `00CBCC03`, `00CBCC0E`, `00DF50CC`, `00DF50E4`, `00BF6B43`.

| Native body | Coverage | Source | Original ABI |
| --- | --- | --- | --- |
| `B1F280..B1F320`, 161 bytes | complete normal body and represented C++ cleanup states | `construct_native_render_queue_00b1f280` | ECX actual queue, EAX same queue, no stack arguments, plain RET |

`BSP_RenderCommandQueue_Construct` is a descriptive hypothesis. The new source
interface borrows the actual renderer and queue publication cells, the existing
`NativeRendererStopPipelineContext`, and the queue's existing string storage
domain. It uses `NativeRenderCommandQueueStorage`, whose trivial 34h declaration
performs no initialization or automatic destruction. There is no second queue
owner or substituted stop callback.

The constructor installs `D5E5F4`, separately clears configuration bytes+04/+0C
and DWORDs+08/+10, then initializes the command header+14/+18/+1C and row
header+24/+28/+2C. At **B1F2C8**, it compares the **preexisting** control+20 with2.
At B1F2CC it arms state2, then B1F2D1 clears context+30. The comparison therefore
cannot be replaced with a read after prezeroing+20 or+30. Only the captured
equality enters the stop path.

That path reads current `F8D394`, the current renderer table, then table+11C.
The admitted actual renderer profile `D5F0A8` supplies `B28A90` at `D5F1C4`.
The existing complete provider requests/awaits a current worker if present,
disables synchronization through B33AA0, clears the complete B26920 pipeline,
and executes real `Sleep(100)`. The constructor adds no worker wait timeout,
join, success fallback or exception recovery.

After stop returns, B1F2E6 clears+20. The two-entry loop clears each byte/value
configuration again; B1F306..B1F30F writes value0 then enabled1 for each entry.
Padding+05..07/+0D..0F is never overwritten. Callback changes to command and row
headers, context+30, padding or the table word survive normal completion; the
constructor does not reread the publication to choose a different owner.

The sole native caller is `4C11F0`. Allocation ownership remains with that
caller: `4C124A` pushes34h, `4C124C` calls
BF681B, `4C1261` transfers the allocation to ECX, and `4C1263` calls B1F280.
Only after return does `4C1271` publish EAX to F8D440, followed by registration
at `4C1284 -> BD0C30`. That getter's locking, allocation, registration and
failure paths remain a separate reconstruction.

## Cleanup evidence

The original normal body registers raw handler `CBCC0E..CBCC17` (10 bytes):
`MOV EAX,DF50E4; JMP BF6B43`. Ghidra currently has no function at CBCC0E; it is
preserved as a consumed raw EH dependency and is not counted as reconstructed
or annotated by this packet. `BF6B43` retains its existing library identity.
`DF50E4..DF5107` is the 36-byte FuncInfo, with three states and unwind map
`DF50CC..DF50E3` (24 bytes):

| State | Predecessor | Complete raw action | Existing concrete terminal |
| --- | --- | --- | --- |
| 2 | 1 | `CBCC03..CBCC0D`: `[EBP-10]+24`, JMP at CBCC09 | B1F150, current row destruction through the same `NativeStringStorage` |
| 1 | 0 | `CBCBF8..CBCC02`: `[EBP-10]+14`, JMP at CBCBFE | B1D590, current command-pointer storage destruction |
| 0 | -1 | `CBCBF0..CBCBF7`: `[EBP-10]`, JMP at CBCBF3 | B1C3C0, base cleanup |

State0 becomes active at B1F2B2; state2 replaces it after the old-control
comparison at B1F2CC. There is no throwing native call between those stores.
On a stop exception, the three current member/base providers execute in that
order. Rows and command storage preserve their dangling data/capacity fields.
Base cleanup **unconditionally** clears actual F8D440, even when it points to a
different queue, then writes CE3818 to the original receiver. It does not free
the queue's raw allocation. A second C++ exception during cleanup terminates.
The source does not claim original stack-frame ABI or general hardware SEH.

## Verification

Nine freshly captured spans (833 bytes) match both the verified live
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` and the installed PE.
The owned source passes MSVC Win32 `/W4 /WX /O2 /MD /fp:strict` compilation.
Standard build and existing test results are recorded in the companion report;
the worker does not edit the shared CMake registration.

One external fixture at `C:/Users/sqz269/bsp-ax-render-queue` compares the full
original constructor with source: **five pairs, 36,650 postimage bytes and 186
event words**. It covers hostile old control!=2 with a null renderer publication,
normal null-worker stop, a real signaled worker acknowledgement, an early
texture exception and a nested depth exception. Stop calls forward to two real
D3D9 HAL devices and their actual backbuffer; successful stop paths execute
real Sleep100 (109-110ms observed). COM observers populate both queue headers
through existing providers and change table/configuration/context/padding;
normal survival and exception postimages agree. The observing string adapter
forwards to the same existing pooled string domain and confirms rows release
before command/base cleanup.

Original B1F280 differs only in two DWORD operands: its EH registration points
to a host-image jump-only trampoline, and its renderer-global operand points
to the exact cell borrowed by source. Original unwind actions, handler,
FuncInfo/map and B1C3C0 remain byte-identical. The original stop bodies and
complete cleanup/provider bridges are inherited from copied, hashed AW fixture
assets; their explicit relocations are verified after the run. Actual FH3
unwinds both selected exception cases. Unknown provider methods fail if reached.
Fixed-address reservation collisions retry only exit77 before execution;
behavioral failures are never retried as mapping failures.

Default `run.ps1 -Root <integrated checkout>` compiles only the probe and links
that checkout's current three libraries. Worker validation explicitly uses
`-WorkerSource -LibraryRoot <AW integrator checkout>`, adding only this owned
source. This is bounded composition and exception evidence, not independent
original execution of every provider, an original binary replacement, general
concurrency/SEH proof, full queue lifecycle or gameplay validation. No permanent
test suite or game installation files were added or changed.

## AX integration analysis refresh

The integrator saved all four AX original signatures and reviewed names,
verified their complete stored bodies and refreshed exports. Three ten-byte
EH handlers CBCC0E, CBD348 and CBD408 were defined under owned leases and
the Ghidra write lock. Missing-function observations above describe the
earlier worker capture. EH definitions are analysis metadata, not additional
reconstructed normal-body claims. Combined final-commit validation remains
separate from the worker fixture evidence.

## AX exact merged validation

The exact combined source commit `16dcc9e774a8d33eab4cb9565155d4c151bbd242` passed the Win32 build,
both existing tests and three current-library-only original-byte fixtures.
Full counts, original-byte relocation, exception branches and Reset coverage
are recorded in `reports/native_renderer_reset_ax_validation.json`.
Earlier pending statements describe initial capture stages. These results
do not establish whole-game rendering, native ABI identity or general concurrency.
