# Actual renderer state leaves

Four complete native bodies now have narrow MSVC Win32 hardware entries in
`src/native_render_state_leaves.cpp`. Their **45 compiled instruction bytes are
identical** to the existing Ghidra program and installed executable, with no
object relocations. These routines use borrowed actual storage; they need no
invented renderer class or unresolved callbacks.

| Original | Complete span | Behavior and native ABI |
|---|---|---|
| `B20240` | 27 bytes | ECX renderer. Read DWORD `+1D90`; nonzero immediately returns EAX=0. Otherwise read byte `+1D8A`; return EAX=1 only when zero. RET. |
| `B1FE20` | 11 bytes | ECX renderer. Compare DWORD `+1998` with zero and SETNZ AL; upper EAX and CMP flags remain as in the original. RET. |
| `B20210` | 3 bytes | Entire concrete renderer command-hook override is RET4. ECX and the stack argument are ignored. No fields or callbacks are touched. |
| `B51B20` | 4 bytes | ECX actual batch. Load its raw count DWORD `+10` into EAX and RET; signed interpretation belongs to callers. |

The first, second and fourth APIs use `__fastcall` for their sole ECX input.
The hook uses `__stdcall` for its sole ignored stack word; native ECX is unused.
The frame-active declaration exposes only the meaningful result byte. Inline
assembly preserves the original instruction order, flags and incidental register
bits without depending on C++ optimizer choices. The no-op is the complete
observed body, rather than an implementation substitute for a missing callee.

The renderer extents touched are `1D94h` and `199Ch`, respectively. The count
getter touches through batch offset `13h`; the already recovered actual batch
storage is `18h`. No renderer constructor, full class layout or vtable
installation is implied. Existing semantic readiness and frame fragments remain
in place. Full queue/command execution and the renderer lifecycle remain open.

[The audit](../reports/native_render_state_leaves_audit.json) records full byte
spans, live/installed/object comparison, source and build hashes, prior names and
comments, and the saved annotation journal. The project/program were verified
before analysis and annotation batches. Descriptive names remain hypotheses.
The strict Win32 build and both existing CTests passed. No new test case or
private runtime fixture was needed for these dependency-free leaves. This is
build and instruction verification, not a running renderer or gameplay result.
