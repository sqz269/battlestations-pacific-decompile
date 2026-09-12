# Native tracked critical-section release

`release_native_tracked_critical_section_0041cc80` reconstructs the complete
**0041CC80..0041CCBF, 64-byte function**. Its descriptive name is a source
hypothesis. The entry consumes the address of an actual four-byte owner slot in
ECX, accepts no stack arguments, and returns with plain `RET`. It declares no
semantic return value and requires no added EDX context.

The slot must contain either null or the actual malloc-backed 1Ch tracked
critical section. The existing `TrackedCriticalSection` is exactly a Win32
`CRITICAL_SECTION` at +0 and signed physical depth at +18h; compile-time checks
enforce that layout. This entry does not reinterpret a typed singleton manager,
section projection, or another allocation domain.

## Native operation schedule

The complete naked body retains each native instruction and branch:

1. Capture the owner-slot address in EBP and its current section in ESI once.
   A null section returns without writing the owner slot. A null slot address
   is not checked or converted to a successful return.
2. Test the section's current depth as signed-positive. If it is positive,
   preserve EDI and capture the actual `LeaveCriticalSection` import once.
   Decrement depth before each Leave call and reread the physical depth after
   each return. Continue only while it remains signed-positive, then restore EDI.
3. Call the actual `DeleteCriticalSection` import on the captured section, then
   the existing free service on that same section. A zero or negative initial
   depth skips Leave but still performs Delete and free.
4. Discard the cdecl free argument and only then clear the original owner slot.
   Never reread the slot to select the pointer to free. Restore ESI/EBP and return.

The caller must satisfy the Win32 section ownership/lifetime requirements;
ordinary teardown is on the owning/quiescent thread. This source adds no
ownership repair, validation callback, counter clamp, slot recapture or cleanup
after a failed service call.

## Actual providers and allocation pairing

The two native import slots are **00CE2210 LeaveCriticalSection** and
**00CE2214 DeleteCriticalSection**, both in KERNEL32.dll. Source relocation
symbols bind the corresponding actual Win32 imports; the Leave pointer remains
captured for the loop.

Native **00BF65AC** is a returning five-byte jump to the game's **00BF9DC8** free
implementation. The source binds the existing cdecl
`singleton_lifetime_free(void*) noexcept`, whose emitted body jumps to the real
host CRT `free` import. That service is paired with completed raw
`create_native_tracked_critical_section_00bd1860`: its 1Ch request reaches
`allocate_worker_lock`, then `singleton_lifetime_allocate` with both native and
host sizes equal to 1Ch. The allocator uses real malloc/new-handler/retry/throw
behavior. The creator performs actual InitializeCriticalSection and then clears
the physical depth, without compensating cleanup for OS initialization failure.

The earlier `critical_section_create_00bd1860` convenience interface uses a
different new/delete boundary. Existing typed manager/projection paths remain
separate. This packet provides the exact raw release for subsequent raw manager
integration; it does not migrate their callers.

## Evidence and validation

Fresh reads through `tools/bsp.py ghidra` verify the existing project `bsp`,
`C:/Users/sqz269/bsp.gpr`, and program `/battlestationspacific.exe`. Four finite
spans cover 120 bytes: release64, raw creator39, native free jump5 and three
import slots12. All match the configured installed PE. The complete release
decodes to 25 instructions. The bytes at **41CCB3 ADD ESP,4** and
**41CCB6 MOV [EBP],0** are included even though the saved Ghidra listing still
omitted them after its stale no-return edge. This worker makes no Ghidra edits.
All 329 artifacts in the primary raw-discovery seal were also rehashed.

The strict MSVC Win32 Release build uses unchanged `scripts/build.ps1`, an
ignored CMake source hook, `/W4 /WX /fp:strict`, two existing passing CTests and
eight verified native seed spans. No test cases were added. The actual emitted
release is one 64-byte function with no added helpers: all 25 instructions map,
52 bytes match literally, and the other 12 bytes are exactly three explicit
relocations. The actual allocation/free/creator providers are decoded and their
complete COFF objects match the corresponding members of the sealed archive.

The [audit report](../reports/native_tracked_critical_section_release_audit.json)
records bindings, full instruction mapping, build evidence, source hashes and
the immutable `local/tracked_release/sealed.json` evidence location. The evidence
preserves the full source/provider COFF inventory and whole archive.

This establishes complete source reconstruction, original register/stack shape,
finite instruction mapping, and build verification. It does **not** reconstruct
the game's static CRT allocator, prove original instruction/return-PC identity,
hardware-fault/SEH equivalence, or make a drop-in binary replacement. Neither the
release nor original release body was executed by a dedicated lifecycle fixture;
the two existing CTests concern math. There is no game/runtime validation claim.
