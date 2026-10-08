# Type10 raw-array storage Source

`construct_native_scene_property_record_type10_raw_array_storage_008ef3e0` reconstructs the whole `[008EF3E0,008EF454)` Native body: 116 bytes / 36 instructions, including retention and copying. **Source admission credit remains 0.** Primary CMake registration, compilation, emitted-code review, complete-helper fixtures and ABI qualification are pending. No compiler, tests, provider process or Source/Native API was executed in this packet.

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

Native CALL operand intervals `[51,55)` and `[66,70)` are the planned provider-binding differences. This packet has not compiled the Source and makes no emitted-byte equality claim. The current Type9 CPP/HPP and canonical singleton-lifetime CPP/HPP were frozen as four strict current Source pins. Their transport/compiler guards informed the wrapper; each instruction and ordered arithmetic/store in this body was checked against **Type10's own** frozen Native listing. The static comparison passes 36/36 instructions and independently derives the 29/27 mask. It does not qualify generated machine code or helpers.

The entire Root-verified readiness family was copied as data, including its all-file manifest SHA-256 `ea578a4a65ebc0a1ce46ef20e2affb917e5ceb79c94a5c62a723c7729e3fbd2d`. All 328 original/copied readiness files and the four current Source pins matched before/after. No old utility or fixture was executed. Historical paths inside readiness metadata remain data; they do not turn mutable Main metadata into continuing pins or authorize fixture reads.

The new evidence family is `local/t1011s/` in worktree `J:/PROG/battlestations-pacific-decompile-cc12_property_type10_type11_raw_storage_source`. Its exact all-file manifest includes new utilities, checks, all frozen input data and exact snapshots of all eight owned outputs. The manifest's independent hash is supplied in the handoff to avoid a hash cycle with these metadata snapshots.

Root owns shared registration and later compilation/qualification. This packet changes no CMake, names, reconstruction ledger, parallel-work metadata or Ghidra state and adds no tests. Whole clone, insertion/owner publication, native private CRT/EH, recursive release, class lifetime and game behavior remain outside this raw-storage Source implementation.

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
