# Native logical buffer device save

This packet reconstructs two complete operations on actual logical buffer
storage: vertex `00B49D00` through `00B49DBE` exclusive, and index `00B49F80`
through `00B4A03E` exclusive. Each native body is 190 bytes. The original ABI
takes the logical owner in ECX, has no stack arguments and uses plain RET.
There is no stable return-value contract or local FH3 cleanup state. The new
independently named `__fastcall` entries additionally borrow
`NativeLogicalBufferDeviceSaveContext` in EDX. Original callers require this
explicit service binding; the new interfaces are not binary replacements.

The public header is `include/bsp/native_logical_buffer_device_save.hpp` and
the implementation is `src/native_logical_buffer_device_save.cpp`. Names remain
reconstruction hypotheses. The context borrows the actual Lock service context
and four immutable native profile views. It does not replace the logical owner,
physical owner, COM object, shadow allocation or resource lifetime.

| Logical field | Vertex offset | Index offset |
| --- | --- | --- |
| Current physical owner | `58h` | `08h` |
| Flags | `60h` | `10h` |
| Current raw CPU shadow | `6Ch` | `1Ch` |

Only observed physical profiles `D61E10` private index, `D61E34` private vertex,
`D61E58` pooled index and `D61E7C` pooled vertex are admitted. Each borrowed
view contains all nine original DWORDs. Actual physical owner DWORD zero is a
native profile token in this host interface. Selection maps that token to its
borrowed view and reads the selected slot at the original point. Recovered
code-address tokens dispatch directly to these complete compiled providers:

| Slot | Index provider | Vertex provider |
| --- | --- | --- |
| `08h` Lock | `B4B850` | `B4BA00` |
| `0Ch` Unlock | `B4B820` | `B4B9D0` |
| `18h` capacity | `B4B800` | `B4B9B0` |
| `1Ch` COM getter | `B4B840` | `B4B9F0` |

Private and pooled profiles share these targets. No numeric native token is
called as a host pointer. Other profiles or slot contents are outside this
binding's valid domain; the source supplies no runtime fallback or validation
policy for them. Actual owner loads and stores use single x86 DWORD accesses,
including unaligned storage. Address arithmetic retains the Win32 DWORD width.

Save returns when `(flags & F000h) == 1000h`, when physical is null, or when the
first fresh physical COM getter returns null. Otherwise it reloads owner/table
and invokes the getter a second time. A nonnull second result is captured for
AddRef through its current COM table `+04`, followed by Release through the
captured object's freshly loaded table `+08`. A null second result still
continues. Owner and table are reloaded again for the first capacity call.

Array-new `BF55BE` is exactly a five-byte jump to `BF681B`: the captured DWORD
length is neither adjusted nor clamped, including zero. The established actual
`singleton_lifetime_allocate({object, n, n})` provider performs the corresponding
CRT malloc/new-handler retry path. A failed allocation throws before shadow
publication. On return, Save reloads the physical owner BEFORE publishing the
new shadow pointer. It never frees the previous shadow. It then initializes the
base-offset local to zero and captures that owner's current table.

The second capacity call uses this captured owner/table. After it returns, Save
reloads the physical owner but reads Lock from the PREVIOUSLY captured table.
Lock receives `(capacity, 0, 1, &base_offset, 1)` plus the actual host Lock
context. The third stack argument is unused; the final byte requests read-only.
After Lock, another fresh owner/table capacity call supplies the copy length.
Only then does Save read the current logical shadow destination and call actual
CRT memcpy from the captured Lock result. Allocation, Lock and copy therefore
use three distinct capacity observations.

Finally, Save reloads owner/table for actual Unlock, then reloads the physical
owner once more for the entry-specific direct release helper: `B23270` vertex
or `B23180` index. Exceptions propagate with already completed mutations intact.
There is no automatic Unlock, free, rollback, HRESULT branch, overflow clamp,
shadow-state guard, or owner lifetime operation beyond the observed calls.

## Verification

The worker started from main `4cd11c4` and privately cherry-picked frozen Lock2
`018478d1ccff1dccc2e8fe47f1bd7302a8bc9257` as `27d2916`. Access6, getters,
diagnostic3 and the CRT allocation service were already present in that base.
Guarded Ghidra queries verified project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base `00400000` before each
batch. Nineteen complete live spans totaling 2,015 bytes match the installed
PE: both owned bodies, four complete 36-byte profiles, all selected physical
provider bodies, the array-new thunk, its 105-byte allocator and the 869-byte
memcpy reference. Assembly resolves register arguments and all reload ordering.
The worker made no Ghidra or shared-ledger changes.

Private deferred CMake registration adds the Save and frozen Lock sources.
`scripts/build.ps1` passed strict MSVC Win32 Release compilation with `/W4`,
`/WX` and `/fp:strict`, plus both existing CTests after all eight seed spans
were verified. No tracked CMake file or permanent test was changed. The focused
fixture compiles only its own ignored source and links a frozen byte-identical
copy of this build's `bsp_core.lib`. Both 19-byte public entries, the complete
655-byte shared Save implementation and 72-byte profile selector come from
`bsp_core.linked:native_logical_buffer_device_save.obj`.

The original owned bodies execute unchanged at uniform code delta `1F4C0000`.
Their six direct call operands remain unchanged. Four external five-byte jumps
bind the two direct release entries, array-new and memcpy to actual compiled
providers. Virtual calls use four explicit callable physical tables, each with
its four used slots bound to actual compiled getters/capacities/Unlocks and
context-binding adapters for the actual compiled Locks. All other table cells
retain their original token values and are not invoked here. Separate original
token views remain byte-identical to the installed PE for the reconstructed
profile selector. No owned byte or operand is patched. This explicit table
binding also avoids the CRT heap reservation covering canonical `B40000`.

A real D3D9 HAL device on NVIDIA GeForce RTX 5090 creates six 64-byte SYSTEMMEM
vertex/index buffers; GetDesc checks size, usage and pool. Actual COM tables are
cloned only to observe AddRef, Release, Lock and Unlock. Every observer forwards
the saved real `d3d9.dll` method with the same actual receiver and arguments.
Actual objects retain a fixture reference so the method's final Release does
not invalidate observations. Mappings, reference changes, copied bytes and
method results come from these actual resources. No frame is drawn or presented
and the fixture window is never shown.

Temporary fixture IAT observers likewise forward actual malloc, free and
`_callnewh` from `ucrtbase.dll`, and actual memcpy from `VCRUNTIME140.dll`.
They record actual sizes/results and make declared owner/shadow mutations after
real calls. They never substitute an allocation, mapping, copy result or COM
provider. Cleanup restores all IAT/COM table entries, releases fixture resources,
unlocks retained mappings and frees the actual raw shadow allocations after
the method's observations. The previous CRT new handler is restored.

Nine states run for both entries and both implementations:

| State | Observed behavior |
| --- | --- |
| 0 | Dynamic logical-flag early return |
| 1 | Null physical-owner early return |
| 2 | Null first actual COM getter early return |
| 3 | Real allocation, READONLY Lock, copy, Unlock and direct release |
| 4 | Actual provider observers produce allocation 32 / Lock 16 / copy 8, change the current shadow, select a prelocked B for Unlock and A for direct release, reload the captured COM table, and wrap Lock/Unlock depth |
| 5 | Observer throws after successful real Lock; published shadow remains and copy/Unlock/release do not occur |
| 6 | Real `malloc(FFFFFFFFh)` fails; real `_callnewh(FFFFFFFFh)` returns zero; actual allocator throws before publication |
| 7 | Real malloc, Lock and memcpy each receive zero bytes; normal release completes |
| 8 | Observer throws after successful real Unlock; completed copy remains, depth decrement and direct release do not occur |

All 18 paired cases match: 47,122 DWORD observations and 256 event frames per
implementation. Each event records all 160 logical-storage bytes and three
48-byte physical-storage spans, including unaligned padding. Only explicit
profile binding identity and the current shadow allocation identity normalize;
remaining stored values compare literally. Actual mapping identity/presence and
mapped/copied data are checked separately. State 4 copies the actual C mapping
to a replacement raw shadow and leaves the previous shadow intact and unfreed.
All real Lock and Unlock results in this run were S_OK. The two explicit
observer throws are not claims of driver exceptions.

Fifty-four entire linked functions, both original bodies, four immutable token
views, four callable tables and four external jumps are captured before calls
and after each of 36 individual executions: 37 stages, 68 spans per stage,
2,516 unchanged span snapshots. The audit checks every initial compiled span
against the fixed-base linked PE and every non-relocation COFF byte, resolves
table and bridge destinations through the link map, compares the binary traces,
and pins complete sources, library, objects, executable and ignored evidence.

This establishes the complete bounded Save behavior with actual recovered
providers and explicit profile/service bindings. It does not establish original
caller ABI, arbitrary physical profiles, concurrent changes between pure getter
or capacity leaves, second-getter-null mutation coverage, allocation retry
success, full restore/reset integration, gameplay compatibility or visual parity.
The original CRT/helper machine bodies are verified references; runtime calls
use their established compiled providers. The evidence report is
`reports/native_logical_buffer_device_save_audit.json`.
