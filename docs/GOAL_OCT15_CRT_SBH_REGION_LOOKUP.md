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
A later primary registered normal Win32 build remains pending; the mapper's
earlier gate supplies no validation for this Source.

A purpose-built, write-locked plate-only append at `00C11D3D` preserved the
prior plate and left the full `00C11D61` inventory comment and both names
unchanged. The project was saved and the affected export refreshed. Fresh
typed readback at modification 4765 confirms the formal bodies, false thunk
and CALL_RETURN override remain unchanged. The refreshed export still covers
the formal 18-byte entry; it is not proof of a repaired 43-byte formal body.
No rename, byte, instruction, prototype, body or flow write was performed.

No worker CMake change, build or new test is authorized. General Original
free/malloc, heap/bootstrap, SEH and runtime/gameplay closure remain false;
see the separate free-readiness report.
