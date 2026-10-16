# Original CRT SBH region lookup

The whole physical `00C11D3D..00C11D67` routine is 43 bytes / 14 instructions.
Its four true incoming calls are `00BF9DF1`, `00BFDD47`, `00BFDD97`, and
`00C07BAA`. Native entry `00C11D3D` is cdecl with one live pointer DWORD at
`[ESP+4]`, an EAX descriptor/null result and plain RET. `00C11D61` is the
interior loop test with captured EAX/ECX register state; it has no independently
observed external caller or recovered standard-ABI contract.

The implementation borrows the existing `NativeCrtSbhState` canonical cells.
It reads count `0109ED64` before base `0109ED68`, captures the DWORD-wrapped end
`base+count*14h`, and uses the native unsigned descriptor/end comparison. Each
reached iteration reads the actual live argument cell anew, then the current
region base at descriptor `+0Ch`. A wrapped unsigned difference below `100000h`
returns that descriptor; otherwise the descriptor advances by exactly `14h`.
There is no initial positive-count, sentinel, null or alignment shortcut. An
empty captured interval does not read the argument or any descriptor word.
The Source interface takes a raw pointer to the actual four-byte argument cell,
so absent backing is admitted on that empty path and no reference-validity or
alignment precondition is introduced for an unreached read.

Only reached raw reads must be valid. The added state/argument bindings and
Source bookkeeping must remain stable and disjoint from reached raw storage.
The one native-width descriptor load uses the same MSVC Win32 raw-read form as
the accepted canonical SBH implementation; it adds no provider, SDK call,
callback, descriptor overlay, heap, lock, validation or startup owner. The new
C++ interface is not an Original stack/register/fault ABI adapter.

All 43 live bytes equal the installed PE bytes, SHA-256
`efe92ed21d91a645a4868b949361c61fea2c0a4c75201b12f07e6537cd6252fd`.
The complete fresh typed capture at modification 3629 verifies all 14 starts as
InstructionDB objects. It exposes an analysis defect: `00C11D3D` is a false
18-byte thunk to `00C11D61`, whose formal body owns the remaining 25 bytes.
The actual `00C11D4D` short JMP has default UNCONDITIONAL_JUMP but CALL_RETURN
override and effective CALL_TERMINATOR. It is the sole current reference to
the false interior function. The full prior name/body/comment preimages and
physical instruction schedule are retained for the authorized bounded repair.

The metadata repair is **NOT PERFORMED**. The guarded read-only transport probe
returned `Script execution disabled` from `/run_script_inline`; no permitted
atomic body-setting route is available. The fixed transaction and complete
preimages remain a repair proposal with no mutation credit. Broad script-policy
changes, restarting Ghidra, generic delete/recreate, and re-disassembly are not
authorized. Both existing names, the formal 18-byte/25-byte bodies, false thunk,
and CALL_RETURN override remain unchanged. No byte or instruction change is
claimed, and the physical JMP is never counted as an actual native call.

Whole ordinary Source readiness at the actual-storage contract is independent
of that analysis-boundary repair. Its earlier coupling to repair was a workflow
condition, not a missing native functional dependency: the full physical
43-byte algorithm and all 14 instruction starts have been verified. The primary
independently accepted the entire CPP/header against the complete assembly,
including canonical volatile count/base references and the fresh raw argument
read before the current descriptor word. The CPP/header hashes are unchanged.
The primary registered this Source once and ran `scripts/build.ps1` at
`c606835ba8653eda14e67c47fbd2cdbb375f8ab6`. MSVC Release Win32 succeeded and
all three existing checks passed in 13.98 seconds. The actual receipt is
`reports/goal_oct15_region_primary_review.json`; the owned report pins its
immutable receipt/log/archive/object hashes. The mapper's earlier gate supplies
no region validation, and no worker build or new entry execution is claimed.

A purpose-built, write-locked plate-only append at `00C11D3D` preserved the
prior plate and left the full `00C11D61` inventory comment and both names
unchanged. The project was saved and the affected export refreshed. Fresh
typed readback at modification 4765 confirms the formal bodies, false thunk
and CALL_RETURN override remain unchanged. The refreshed export still covers
the formal 18-byte entry; it is not proof of a repaired 43-byte formal body.
No rename, byte, instruction, prototype, body or flow write was performed.

Full worker and primary inspection of the actual emitted object passed. The
public function is 86 bytes / 36 instructions; an uncalled static raw-load
helper is 19 bytes / 10 instructions, giving 105 text bytes in the object.
That helper is a Source operation, not a second mapped Native provider. There
are zero emitted calls and code relocations, one positive external public
definition, and one identical archive body. Actual count read `0Ch` precedes
actual base read `14h`; the intervening pure count LEA changes no payload-read
schedule. Each reached loop reads the actual argument DWORD at `23h` before
the current descriptor word at `31h`, with the exact unsigned end, 1 MiB and
20-byte-stride behavior. The empty path skips both raw reads.

The compiler reuses its Source private argument home `[EBP+8]` at `2Bh/2Eh`
as an address temporary and uses private `-4/-8` cells for raw values. These
stores concern Source bookkeeping, not the actual native argument cell or
Original frame aliases. Supplied binding metadata must remain stable, and
binding/bookkeeping storage must be disjoint from reached raw storage. No
Original private-home, frame, fault or ABI identity is credited. The object
SHA-256 is `dfbaf3666b376e5ab2cd198f398c103a6849c9cb61a666785a1a3935b98b5ced`.

The retained repair transaction uses the historical pre-plate comment preimage
and must not run unchanged or automatically. A future permitted repair needs
fresh metadata/comment preimages and primary-directed review. No worker CMake
change, build or new test was performed. General Original free/malloc,
heap/bootstrap, SEH and runtime/gameplay closure remain false; see the separate
free-readiness report. The boundary repair remains **NOT PERFORMED**.
