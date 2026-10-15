# Whole ordinary property assignment storage (CC12)

The two helpers reconstruct complete normal bodies: `004357F0`, 26 bytes and
11 instructions, and `008F03B0`, 63 bytes and 27 instructions. Their new C++
interfaces use actual storage and the same current canonical heap as the
existing physical string/reference constructors. Names remain hypotheses.
Whole record assignment `008F0700` is a separate dependent packet.

Free-and-clear loads the real four-byte slot. A null pointer causes neither a
provider call nor a store; a nonnull pointer is freed first, then the slot is
physically cleared. The slot must remain live outside the freed allocation.

Array copy reads the real source pointer before its raw byte size and captures
both before reading the destination old pointer. A nonnull old allocation is
freed and its slot is physically cleared. The captured size is then published
before canonical allocation. The returned pointer is published before the real
`memcpy` call. Physical load/store helpers retain these writes and their order;
no C++ record type or callback model replaces the two-word block.

The admitted count is positive and its readable source span does not wrap.
The old destination pointer is null or a sole-owned canonical allocation.
Source bytes, headers and active frames are disjoint from old freed storage
and the new output. Owning self/shared-buffer assignment is excluded: capturing
the source pointer before freeing it would leave `memcpy` with freed storage.
The allocator may reuse the old address, so pointer inequality is not promised.

The array helper uses bytes without a stride or tag check. It does not inspect
or change ownership byte `record+2C`; a record destination must already have
owning flag one for later canonical retirement. No zero-count shortcut,
ownership repair or failure cleanup is added. Zero-size CRT behavior, failed
providers, invalid memory, overlapping storage, reentry and mutation remain
outside this bounded ordinary contract.

The report separates strict whole-object inspection, the required current
normal build and integrator-owned CMake/library registration. No new test or
Native target is required by this leaf-only packet. Original residual register,
stack, flag, private CRT and EH/OOM transport, class/vtable behavior and gameplay
are not claimed.

The first strict MSVC Win32 compile passed. Complete inspection accounts for all
five emitted functions, 312 bytes and 95 instructions, with no unknown normal
control-flow edge or FP/SSE instruction. Array copy is 220 bytes/63 instructions
and free-and-clear is 51 bytes/17 instructions; the other three are emitted
physical access/cast helpers. The required normal build passed its two existing
configured tests. At that build the new TU awaited shared CMake registration,
so its standalone object result is recorded separately from library membership.
