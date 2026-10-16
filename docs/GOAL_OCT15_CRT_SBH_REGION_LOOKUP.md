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

Repair is authorized but has not run: the guarded read-only script transport
probe returned `Script execution disabled` from `/run_script_inline`. No
mutating fallback, generic re-disassembly or byte/listing change was attempted.
The packet's metadata/Source readiness gate remains pending safe atomic repair
and verification. A fixed transaction must preserve the interior label/comment,
unset only the false thunk/override, merge the whole body, retain every native
instruction, save and read back all invariants before Source admission.

The primary will review the complete Source and eventual repair proof before
registration and a later normal Win32 gate. No worker CMake change, build or
new test is authorized. General Original free/malloc, heap/bootstrap, SEH and
runtime/gameplay closure remain false; see the separate free-readiness report.
