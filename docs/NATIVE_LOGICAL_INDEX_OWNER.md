# Native logical index stream destruction

The three complete functions below destroy an existing logical index stream and return its pooled slot. Construction, physical-buffer creation/retry, and renderer ownership are outside this packet. Names are descriptive hypotheses. These are new MSVC Win32 C++ interfaces, not drop-in replacements for the original ABI.

| Address | Native ABI | Reconstructed behavior |
| --- | --- | --- |
| `00B4B6F0` | ECX logical owner; RET; 205 bytes | Destruction, unregistering, atomic physical release, optional renderer guard and base cleanup |
| `00B4C1F0` | ECX owner, stack flags; EAX original address; RET4; 32 bytes | Destroy first; flag bit 0 then returns the raw slot to `0108FE50` |
| `00B495E0` | ECX actual pool, stack raw slot; RET4; 101 bytes | Return a 28h slot through the actual pool critical section and free-index array |

## Owner and physical release ordering

`00B4B6F0` first writes logical profile `00D61DE0`. Before optional entry it activates base cleanup. An enabled entry captures the current renderer from `00F8D394`, then calls `00B33AD0` and saves AL. The raw synchronization byte is not converted into a stored Boolean.

After entry it reads logical flags at `+10h` and the physical pointer at `+08h`. With `(flags & F000h) == 1000h`, `00B4B390` unregisters the logical address from that captured physical buffer. The other branch retains the otherwise-unused DWORD load from physical `+04h`. The owner then reloads the current global renderer for `00B26900`, reloads logical `+08h` after that call, and applies real `InterlockedDecrement` to the reloaded physical buffer's intrusive LONG at `+04h`.

Only a zero result invokes the physical profile's slot 0. Both observed index profiles contain `00BD30E0` there. That invoker reloads the physical profile and dispatches slot 1 with flag 1:

| Immutable profile | Slot 0 | Slot 1 | Actual terminal behavior |
| --- | --- | --- | --- |
| `00D61E10` | `00BD30E0` | `00B4BB20` | Existing physical index destructor, then scalar free |
| `00D61E58` | `00BD30E0` | `00B4C210` | Existing physical index destructor, then return to the actual `0108FDA8` pool |

The C++ context borrows the first two DWORDs of these actual immutable tables, preserving the original code-address tokens. It reads both the initial invoker table and the reloaded terminal table. There is no arbitrary virtual callback or default terminal. Other profiles or table contents are outside the recovered domain.

Normal guard cleanup uses the captured entry renderer, its current `+04h` lock, and the current synchronization mode. This differs from the current global renderer used for removal. It reads the complete saved DWORD through an isolated MOV, including padding; the leave helper ignores the argument. The entry-disabled/exit-enabled path would consume an uninitialized native guard; it remains outside the valid execution domain. The C++ interface does not repair it or initialize a replacement guard.

## Native exception states

The original handler `00CBF950` refers to FuncInfo `00DF8410` and two unwind entries at `00DF8400`. These bytes and both complete actions were compared with the installed executable.

| State/action | Established behavior |
| --- | --- |
| State 0, `00CBF940` | Reload logical owner from EBP-18h; tail through `00B49420` to `00BD30F0`, writing base profile `00CEB130` |
| State 1, `00CBF948` | Pass guard at EBP-14h to `00B21110`, then run state 0 |
| Normal leave | State 1 is disarmed before `00B33B00`; a leave exception therefore runs only base cleanup |
| Normal base call | State 0 is disarmed before `00BD30F0` |

An entry exception runs base cleanup without guard cleanup. A body exception runs guard cleanup and then base cleanup. A second exception from the guard unwind action terminates through the C++ runtime; a dedicated `noexcept` helper preserves this behavior in the reconstruction.

## Logical pool storage

The pool owns an initialized real critical section at `+0Ch`, a tracked DWORD depth at `+24h`, a slab-pointer array at `+28h`, and the lowest available slab index at `+34h`. A raw 28h slot stores its slab index at `+24h`. Each slab has 32 slots; WORD free indices begin at `+500h`, with the WORD free count at `+540h`.

`00B495E0` acquires the actual pool lock, increments tracked depth with DWORD wrapping, then reads slot metadata and the current slab pointer. It divides the signed 32-bit slot-address difference by 28h, stores the low WORD in the free list, increments its WORD count, and applies an unsigned minimum to the lowest slab. Finally it decrements tracked depth and leaves the same lock. No new unwind guard is added to this leaf.

## Verification and limits

The isolated complete Win32 build ran `scripts/build.ps1`; the existing `reconstructed_math` CTest passed. The build includes this source and frozen physical-owner/unregistration dependency files whose hashes match the integrator's current source. The fixture link map pins the three new entries and the production physical, unregistration and synchronization entries to `bsp_core.lib`.

One private composition fixture executed all 338 owned native bytes, complete native unregistration/synchronization helpers, the original `00BD30E0` invoker, and both original logical unwind actions. Seventeen original/source comparisons matched 10,744 trace DWORDs. These cover dynamic masking, current renderer and physical-pointer changes at real Win32 call boundaries, raw mode changes, both physical terminal profiles, both logical deleting-flag branches, both pools, atomic wraparound, and entry/body/normal-leave exceptions. Isolated original/source double-fault children both reached controlled terminate exit 91.

The fixture verified 23 Ghidra/installed-PE spans totaling 926 bytes and checked 31 relocation preimages. Native code and native table pointers are relocated together. Source table views retain verified immutable original tokens. Trace normalization maps only the two native profile pointers to those tokens. The two exact native terminal entry adapters verify their profile and flag 1, restore the corresponding original token, and enter the unchanged production physical deleting body. A host-image FH3 registration adapter points to relocated original metadata and actions. Win32 lock calls and atomics remain real; boundary observation forwards to those actual operations.

The new fixture's physical terminal stage is production-source composition. The separate physical-buffer proof establishes its original-byte behavior, including double faults. The new fixture does not execute the full native physical destructor again. Its COM object supplies only an observed IUnknown Release boundary; this is not D3D driver, game-runtime, or binary-ABI validation. The game installation and Ghidra program were read only for this worker packet.

Primary integration registered this source in CMake, preserved the native whole-DWORD guard load, and passed the combined Win32 build and both existing CTest checks. The same 17 comparisons and both double-fault children passed again against the corrected primary library; its link map identifies all three logical entries and the exact production dependencies. The three Ghidra names/comments and complete-function ledger entries are saved, with refreshed exports. Reproduction and SHA-256 pins are recorded in [the audit](../reports/native_logical_index_owner_audit.json).
