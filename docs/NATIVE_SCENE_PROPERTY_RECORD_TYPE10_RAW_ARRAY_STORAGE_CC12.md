# Type10 raw-array storage Source

`construct_native_scene_property_record_type10_raw_array_storage_008ef3e0` reconstructs the whole `[008EF3E0,008EF454)` Native body: 116 bytes / 36 instructions, including retention and copying. **Source admission credit remains 0 pending parent review.** Production registration/compilation and the fresh worker's complete-helper single-process raw-domain qualification have passed. The current qualification is recorded below; the original Source-only packet and Root production evidence remain distinct historical records.

The implementation is [the owned CPP](../src/native_scene_property_record_type10_raw_array_storage.cpp), with its contract in [the owned header](../include/bsp/native_scene_property_record_type10_raw_array_storage.hpp). The Native whole-body SHA-256 is `00c1508701d10a6508b556eff0feb4737f51a4ec289059dce77a796cc571919b`. Names are descriptive hypotheses, not recovered symbols or class identities.

## Bounded contract

Supply actual fresh, unowned, writable 56-byte root storage and an actual stable readable span of `4*count` bytes, with unsigned count in `1..3FFFFFFFh`. Root, input and active call frame must be disjoint and all address ranges nonwrapping. Require DF clear for current providers and successful normal allocation/copy. The maximum scaled size is `FFFFFFFCh`; this arithmetic limit does not guarantee available address space or allocation. Zero counts, overflow, aliasing, failures, faults and reentry are excluded preconditions, not new checks inserted into the Native sequence.

The naked MSVC Win32 fastcall interface reserves ECX for root and EDX for an unused second formal. At entry `T=ESP`, `[T+4]` is count, `[T+8]` is the actual input pointer, and `[T+C]` is a full DWORD of flag bits. It returns root in EAX and uses `RET 0Ch`. The Source prototype preserves all three stack DWORDs; it does not narrow the flag to a C++ bool.

Only the low flag byte selects behavior. Zero retains the exact input pointer, without allocation/copy/free or ownership transfer. A nonzero byte allocates and copies `4*count` bytes through genuine current providers. Input remains caller-managed. Observe a copied child while live, then free it exactly once through `singleton_lifetime_free` before root disposal; do not free retained input as a newly allocated child.

## Native ordering and partial storage

The Source follows this Type10 listing independently: save ESI/EDI, load count, double it, store phase `00CE89D4h` and tag 10, double it again, then compare the low flag byte. It preserves both unsigned DWORD additions and their placement around the stores. No signed multiplication, numeric element interpretation or overflow branch is substituted.

Both branches zero `+18`/`+1C` and store scaled byte count at `+24`. Retention stores the input pointer at `+20`. Copying calls the size adapter, then loads input from the original stack slot, stores the actual allocation at root `+20` **before** genuine memcpy, and cleans the four outstanding provider-argument DWORDs. Both tails restore EDI, zero ordinal `+34`, store byte one at `+2C`, return root and restore ESI.

Exactly 29 bytes are written: `[00,08)`, `[18,28)`, `[2C,2D)`, `[34,38)`. Exactly 27 bytes are preserved: `[08,18)`, `[28,2C)`, `[2D,34)`, including owner DWORD `+30`. The constructor publishes no owner. The phase remains opaque data, and byte `+2C` is one in both branches; neither grants a class, free-marker, vtable or destructor contract.

Retention leaves ECX=root and EDX=input. Its final CMP defines arithmetic flags mask `08D5h` as `0044h`, including AF=0. Copying leaves ECX/EDX as unconstrained provider residuals; its final `ADD ESP,10h` defines all six arithmetic flags from `(T-24)+16 = T-8`. ESI/EDI are saved/restored; EBX/EBP also depend on genuine provider ABI. There is no blanket DF/ES/FPU/MXCSR or exceptional-unwind promise. The interface has no `noexcept` promise, and provider failure may leave partial writes.

## Provider binding and evidence boundary

The Native allocation CALL at `008EF412` reaches `00BF55BE`, a jump to `00BF681B`. Source instead calls its own private noinline cdecl size adapter, which directly calls `singleton_lifetime_allocate({object, bytes, bytes})`. That adapter is current-domain glue and earns **no Native credit**. The copy CALL at `008EF421` binds real `memcpy`; `<cstring>` and `#pragma function(memcpy)` follow the current verified Type9 transport pattern. No native numeric function pointers, provider stubs, callback services or fake globals are introduced.

Native CALL operand intervals `[51,55)` and `[66,70)` are the provider-binding differences. The original Source-only packet did not compile the Source or claim emitted-byte equality. Its Type9 CPP/HPP and canonical singleton-lifetime CPP/HPP were frozen as four strict then-current Source pins. Their transport/compiler guards informed the wrapper; each instruction and ordered arithmetic/store in this body was checked against **Type10's own** frozen Native listing. That static comparison passed 36/36 instructions and independently derived the 29/27 mask. Generated-code and helper evidence comes from the later records below.

The entire Root-verified readiness family was copied as data, including its all-file manifest SHA-256 `ea578a4a65ebc0a1ce46ef20e2affb917e5ceb79c94a5c62a723c7729e3fbd2d`. All 328 original/copied readiness files and the four current Source pins matched before/after. No old utility or fixture was executed. Historical paths inside readiness metadata remain data; they do not turn mutable Main metadata into continuing pins or authorize fixture reads.

The new evidence family is `local/t1011s/` in worktree `J:/PROG/battlestations-pacific-decompile-cc12_property_type10_type11_raw_storage_source`. Its exact all-file manifest includes new utilities, checks, all frozen input data and exact snapshots of all eight owned outputs. The manifest's independent hash is supplied in the handoff to avoid a hash cycle with these metadata snapshots.

Root owns shared registration and admission. The original Source-only packet changed no CMake, names, reconstruction ledger, parallel-work metadata or Ghidra state and added no tests. Whole clone, insertion/owner publication, native private CRT/EH, recursive release, class lifetime and game behavior remain outside this raw-storage Source implementation.

## Root production integration

Root independently reviewed the complete Source CPP/HPP and native listing.
The routine is registered in `bsp_core`; the fresh Win32 build and all three
existing checks passed with 4,051 Source/header/CMake inputs unchanged.
The actual I386 production COFF body is 116 bytes; 108 literal
bytes match Native, with only the expected symbolic CALL operands excluded.
Source admission remains **0**. Complete linked helpers, actual callers,
provider/frame captures and the fresh single-process fixture still require
primary qualification. Build/object evidence does not establish game ABI,
class/lifetime behavior, startup or gameplay.

The provisional raw-domain Ghidra name and evidence are saved in the configured
project, with previous names/comments recorded and affected exports refreshed.
This publication leaves Source admission at 0 and fixture/ABI qualification pending.

## Current worker single-process qualification, 2026-10-14

The fresh family is `J:/PROG/battlestations-pacific-decompile-cc12_type10_singleprocess/local/type10_current02/`, based on `9aa204875db05f1df770ea541c8fa1246a8f9de7`. Its `qualification.json` and the appended `worker_current_singleprocess_qualification` report section are the current worker evidence. Reconstructed CPP/HPP and CMake are unchanged. Parent acceptance remains pending; this does not replace the historical records or award Source credit.

All three complete TUs were compiled freshly: Type10, canonical singleton lifetime, and the fixture. MSVC x86 used `/std:c++17 /MD /O2 /W4 /WX /EHsc /Gy /GL-`; the linked PE32 has a verified embedded `asInvoker` manifest, `/OPT:REF /OPT:NOICF`, and fixed base `52000000`. The historical production object still has SHA-256 `b8cd0c8eb13681741e9db8778dda71bdbb89bb118054cbdc238b589956583b08`; its complete 116-byte body was rechecked. No prior object or process supplied the new runtime result. The full repository build/CTest was not rerun because no reconstructed source or build registration changed.

Two separate readers agree on all three fresh COFF objects, all 628 relocation records, 108 retained sections, seven weak entries (two retained, all observed mode 1), and 29 complete linked bodies totaling 5,844 bytes / 1,690 instructions. The size adapter, canonical allocation/free, ordinary typed caller, raw capture caller, fixture helpers, cold exception support, cookie helper, and import thunks are included. Three current archive COFF sections establish complete normal/cookie support extents and relocated bytes; six complete import thunks are tied to current short-import archive members. Runtime associative sections are checked. Exact discardable non-code `.debug$F` records are distinguished from retained runtime sections using absent mapping/retained references and absent PE FPO entries; the separate PE Type13 POGO record is not treated as FPO.

Both readers reject unknown control-flow edges and internal unreachable nonpadding. Register-indirect imports require agreement of all incoming IAT facts. The only bounded dynamic call is the reviewed raw caller's current Source/fresh rebound Native target. The sole local unexpanded frontier remains the cookie helper's `___report_gsfailure` jump. Cold paths are structurally covered, not failure-executed; external providers and CRT startup remain boundaries. All 29 checked bodies were also read from the actual target process and matched the packed gate before and after work.

The successful family launched one target process, PID 61000, exit 0. Its 12 calls use four cases in each of raw Source, freshly rebound Native, and ordinary typed Source lanes. Counts are `4,1,4,1`; full flags are `73A9D500,C41E6800,9B2F7180,E063AC01`. Rebound code starts from fresh live-Ghidra/original-PE bytes and changes only the two rel32 CALL operands to the same current size adapter and real memcpy thunk. Native bytes match before/after, and all 227 selected current input pins remain unchanged.

The independent recorded decoder checks all 56 root bytes, all 40 input-array bytes, opaque copied payloads including embedded NULs, actual target/pointer identities, and simultaneously live disjoint root/input-array/child extents outside the actual thread stack. Raw lanes physically capture all three stack DWORDs, EAX=root, `RET0C` cleanup to `T+16`, EBX/EBP/ESI/EDI, retained ECX/EDX, and all six arithmetic flags including AF. Copy ECX/EDX remain unasserted. The ordinary lane carries no raw-register claim. Twelve roots and six copied children complete canonical cleanup; free counts come from completed explicit loops, not an independent per-free event stream. No heap redzones or untested maximum-count executions are claimed.

Loaded `malloc`, `free`, `memcpy`, and `_callnewh` identities match before allocations and after cleanup: resolved export addresses, module bases, canonical handle paths, file identities/sizes, pinned disk SHA-256, and 32 relocated loaded entry bytes. `malloc/free/_callnewh` resolve to the current Win32 `ucrtbase.dll`; `memcpy` resolves through the actual local import thunk to Win32 `vcruntime140.dll`. This does not prove provider-wide code equivalence or a new-handler invocation.

The preserved `local/type10_current01/` attempt stopped with exit 71 during the initial provider snapshot, before constructor calls: the historical probe incorrectly assumed `&memcpy` was already external. The successful family fixes only snapshot identity resolution through the verified local `FF25` IAT thunk, then freshly compiles all three TUs and launches a new process. Historical gate text also needed an exact eight-byte COFF short-name fix and explicit handling of discarded FPO metadata. All failed utility attempts remain recorded. The old draft's broader pre-interpreter/package/host admission machinery was not executed or claimed by this new isolated workflow.

Current executable SHA-256: `d072e0a4117ffc6b804e0590ba65873d3964f6121ca8924be835d9bc4ddf2897`. Observations: `7d950c47f94770b2fc54f759cc300160e5bacb9eb2ef31814a76a14fb5043470`. Complete loaded-code gate: `f7c1c907f389e815a32e1e0038e3363a1dc636b9f5ef4ffc7dd1f2af622ce430`. The artifact manifest includes final tracked-output snapshots; its independent hash is supplied in the handoff to avoid circular hashes.

The next gate is parent review of the complete current helper/caller/provider/frame evidence and a decision on bounded Source admission. Native private CRT/EH/OOM, exceptional unwind, class/vtable/owner lifetime, complete clone/publication, actual startup, and gameplay remain unqualified.
