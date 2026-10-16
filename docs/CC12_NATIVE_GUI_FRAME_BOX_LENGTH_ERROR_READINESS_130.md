# FrameBox length-error transport readiness 130

A later whole AD1810 Source adapter can honestly call the existing owning
`native_singleton_length_error_00bd0590`, at exactly the qualified boundary
already accepted for 431F70. Complete normal bytes, resolved relocations and
physical EH/temporary actions match; this conclusion does not rely on names
or similar payload shape. No alias or Source implementation is added here.
AD2230 failure ownership remains open; pending 128/AD1910 is not touched.

This read-only packet retains the clean 125 baseline
`af63c366cdf5ed49fdb35356ae85a2ea6f478e15`, without merge/build. Captured 125/123
artifacts are unchanged. Packet 98's sealed byte/physical-listing evidence, its identical
126 retained copy, and accepted 431F70/BD0590 audits are hash-verified and checked
against the unchanged installed Original. Only the actual EH gaps are queried
live through project/program-verifying BSP wrappers; no export/listing repair,
Ghidra/ledger/Source/CMake/Main mutation, build, test, probe or execution occurs.

## Whole normal comparison

| Original | Exact extent / starts | SHA256 |
| --- | --- | --- |
| AD1810 | `[AD1810,AD1879)`, 105 B / 23 starts | `d6e4873cdfb4510a0e9cc2cba51f34fd7beddbf39e39ab6760c0b62d869f6408` |
| BD0590 | `[BD0590,BD05F9)`, 105 B / 23 starts | `867e54dad740442bfe261cae2e65739d00a71fa4051273401505d399b436cde9` |
| Existing 431F70 contract | `[431F70,431FD9)`, 105 B / 23 starts | retained full accepted audit; current disk rechecked |

Both compared functions consume no incoming register/stack arguments and have
no normal return. There is no RET or returning epilogue to invent after their
nonreturning throw call. All 89 nonrelocation bytes match. The only masked
operands are handler immediate offsets 03..06 and CALL displacements 39..3C,
4E..51, 65..68. Every displacement is independently resolved, not treated as a
wildcard service:

| Native call offset | AD1810 site | BD0590 site | SAME real target |
| --- | --- | --- | --- |
| 38 | AD1848 | BD05C8 | 408720 counted assignment |
| 4D | AD185D | BD05DD | 411700 legacy logic-error constructor |
| 64 | AD1874 | BD05F4 | BF6885 `__CxxThrowException@8` |

Both initialize only the observed fields of a no-init 1Ch legacy SBO temporary:
capacity 15, length 0, first BYTE NUL. The preserved leading DWORD and other buffer
bytes are not blank-zeroed. Both push count 12h and the SAME literal CE37E0:
`vector<T> too long` is 18 non-NUL bytes, with a 19th terminating NUL in storage.
408720 receives the explicit count 18, excluding the terminator from the 19-byte
literal size. Its real growth/allocation/copy/terminator side effects remain.

Only after counted assignment returns do both arm state 0, before calling 411700
on the completed temporary. Both then stamp the SAME actual 28h payload profile
D69260 and pass the SAME ThrowInfo D83F98 to BF6885. These are identical actual
constructor/metadata identities in the unmasked bytes, not parallel invented
types. Neither payload equality nor shared ThrowInfo makes modern Source
RTTI/catches compatible with the game's exception ABI.

The identical 44h normal stack reservation places the temporary at entry ESP-50h
and payload at entry ESP-34h; the 28h payload ends before the registration record.
Argument pushes/LEAs and state stores match physically, including the state arm
after 408720 and before 411700. Both cleanup actions use the same `[EBP-50h]`
temporary receiver convention. No runtime establisher-frame/private-stack-alias
or volatile-register compatibility is inferred from that static agreement.

## Complete EH comparison

| Role | AD1810 | Accepted BD0590 |
| --- | --- | --- |
| Handler | CB9168 | CC5478 |
| FuncInfo | DF051C | DFF4BC |
| Unwind map | DF0514 | DFF4B4 |
| State 0 action | CB9160 | CC5470 |

Fresh full 36-byte FuncInfo records differ **only** at the actual unwind-map
pointer +08. Both have magic 19930522 / maxState 1, zero try-block count/map,
zero IP count/map, zero exception-specification type list, and flags 1.
The prior accepted 12-byte prefix alone did not establish these trailing fields.
Each map is `{toState=-1, respective action}`. Both actions are exactly
`LEA ECX,[EBP-50h]; JMP 4072D0`; each rel32 resolves independently to the actual
SBO destructor. Both handler thunks load their respective FuncInfo and
independently tail to BF6B43. No hidden local catch or different effective
temporary action remains in this comparison.

State -1 before assignment completion has no outer temporary action; the callee's
own failure contract applies. State 0 covers subsequent payload construction and
throw, consuming to -1 before completed-temporary cleanup. The constructor retains
its own partial-base cleanup. Native runtime delivery, payload copy/destruction
through ThrowInfo and arbitrary asynchronous faults remain separate from Source
C++ exception transport; this packet executes none of them.

## Actual Source transport and one next packet

Current BD0590 Source creates a no-init `NativeLegacySboStringStorage`, performs
the exact three initial writes and genuine 408720(count 18), then constructs its
`CompletedMessage` guard and throws `NativeSingletonVectorLengthError`.
That concrete class owns one actual 28h `NativeLegacyExceptionStorage`: real
411700, D69260 as an identity DWORD, copy 411940 and destructor 411780. It is
**not** `std::length_error` or a callable host Native vtable. Guard cleanup uses
real 4072D0 only after assignment returned. No extra SBO initialization, generic
replacement exception or dropped constructor effect is needed.

The ten inspected provider Source files match current Main byte-for-byte.
Raw string allocation uses the existing malloc/real `_callnewh` retry domain;
matching free is `singleton_lifetime_free` in the SAME Source CRT. Assignment/
constructor allocation failures retain their accepted Source behavior. The
fixed 18-byte nonaliased literal is not the raw string helper's separate generic
too-long/subrange error path. Source compiler throw/copy/unwind/catch type and
RTTI replace BF6885/FH3/D83F98/native `std::length_error` identity. Native heap,
new-handler/errno/callback count, hardware-fault timing and private spill aliases
are not established. No Source `std::length_error` catch compatibility is claimed.

The existing 431F70 adapter calls this genuine BD0590 provider directly after
the same complete Original comparison. Its actual current declaration, body,
owning exception class and qualification were read; it is an accepted service
domain precedent, not proof by a similarly named symbol or reused runtime test.

One later tiny Source packet, only after a fresh assignment/lease: append
`[[noreturn]] void __cdecl throw_native_gui_frame_box_vector_length_error_00ad1810()`
to the actual existing `native_gui_frame_box_vector_allocation.hpp/.cpp`, directly
delegating to the genuine BD0590 service without another temporary/catch/profile
or handler. Those two files currently contain ACEF10 only and are already
registered to bsp_core at Main `f1d6a6445506397b1bc508d71da40224f872ddfb`
(`cmake/startup.cmake:1073`), observed read-only and not merged here. Preserve
existing library-role `STL_xlen_throw_00ad1810`; no Native alias/RTTI/SEH or
drop-in ABI credit. No new lower service is missing for this qualified helper.
This does not admit AD2230/copy rollback, a populated FrameBox owner, production
activation or the Original private CRT domain.

The receipt contains six verified normal direct-call rows, exact relocation
masks/targets, reused complete bodies/literal, 98/126 provenance, five fresh EH
spans (98 B) plus three accepted BD EH spans (26 B), and twelve unchanged Source hashes.
Original SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every fresh batch verifies `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
Exact artifacts are sealed in the private 130 manifest before commit/reuse.
