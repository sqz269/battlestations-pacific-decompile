# Tagged B4 x87 leaf 01

Addresses: `00B40040` only. Name and operation interpretation are provisional.

The complete measured body is represented by literal MSVC Win32 assembly in
`src/native_tagged_x87_leaf_b4_01.cpp`, with one actual backing-storage input
and a floating result. This closes the deferred x87 body from the B4 leaf
sweeps. It does not recover an owning class or an original C++ prototype.

| Address | Coverage | Measured complete body | Source physical binding |
| --- | --- | --- | --- |
| `00B40040` | complete, `00B40040..00B40046`, 7 bytes | `FLD m32real [ECX+1F8h]`; `RET` | `float __fastcall(const void*)`: backing storage in ECX, result in x87 ST0, no consumed EDX or stacked argument |

Fresh live Ghidra prototype and listing agree on one block, two instructions,
no calls, and seven bytes `D9 81 F8 01 00 00 C3` through the final RET. The
same whole extent equals the installed PE. Each CLI live query verifies project
`bsp`, program `/battlestationspacific.exe`, x86 language and base `00400000`;
configuration points at `C:/Users/sqz269/bsp.gpr` and port 8089. No project
restart, server change, Ghidra write, or game deployment was performed.

Before this packet, lookup had the existing `trivial_body` tag and no Source
reconstruction record; an address search found no body in `src/` or `include/`.
`GOAL15M_TAGGED_LEAF_B4_01.md` explicitly deferred this load/return.
`CC13_SHADER_VTABLES.md` observed the same offset without settling its class.
The input therefore stays untyped backing storage, with only the actual four
bytes at ECX+1F8h required readable. There is no invented layout or provider.
Fresh xrefs report no references; that is not proof that the body is unused.

The original saved Ghidra name is `TRIV_body_00b40040`, the signature is
`undefined TRIV_body_00b40040(void)`, and the documentation response has no
comments. These were captured under `local/goal15q_tagged_x87_b4_01/`.
The existing tag is retained. Ledger naming is an address-based operation
hypothesis; Ghidra annotation remains the integrator's separate step.

The Source definition uses `__declspec(naked)` solely to emit the measured
FLD and RET without an added frame or floating temporary. The instruction
mnemonics express the actual operation; there is no embedded byte array.
Earlier ordinary float-return forms, both direct read and memcpy-to-local,
compiled to `51 8B 81 F8 01 00 00 89 04 24 D9 04 24 59 C3`. Their additional
stack-mediated stores/reloads failed the whole-body gate. Their complete
objects, compiler listings and logs were copied unchanged into this packet's
ignored `prior_failures/` directory and hashed in the report. The return type
remains float; it was not changed to disguise an instruction mismatch.

FLD normally decrements TOP and loads the converted m32real into new ST0;
the prior occupied stack values remain below it. Full-stack exceptional
behavior follows the same instruction. NaN conversion, signaling-NaN invalid
operation and denormal handling use the inherited x87 masks. Intel specifically
documents a denormal push even with the denormal exception unmasked. C1 reports
stack overflow; C0/C2/C3 are undefined. No control-word change or status clear
is added. The same load and RET retain their memory, pending-exception and
return-stack fault paths; no extra data access or wait is introduced.
[Intel SDM, FLD](https://cdrdv2-public.intel.com/774492/325383-sdm-vol-2abcd.pdf)
describes this architecture behavior, not a runtime result from this packet.

These are instruction-semantic conclusions under equal incoming machine
state and addressed storage. Absolute x87 saved instruction pointers and
reported fault instruction addresses depend on actual code placement. A
separately linked Source symbol has not been adopted at `00B40040`. Thus
identical bytes do not establish placement, real caller integration, a
drop-in ABI replacement, or game validation. No x87 runtime probe was run.

Strict qualification uses actual MSVC x86 19.51.36244.0 with `/MD /O2 /W4 /WX
/fp:strict /Gy`. The entire function COFF section must be seven bytes, the
public symbol must start at offset zero, every nonempty executable section
must be accounted for, and every executable relocation count must be zero.
The registered `bsp_core` object is independently subjected to the same gate,
including equality of its complete executable section with the strict object.
The report records exact hashes, section extents, native PE offset, build
command, build duration, and the registered compiler settings.

Both complete-object gates passed: one executable section of seven bytes,
symbol offset zero, and no executable relocations. Strict compilation took
3.296 seconds. The registered full build and existing checks completed in
1310.656 seconds; both `reconstructed_math` and `tool_tests` passed (2/2).
These timings do not substitute for the complete worker wall time supplied
in the final handoff, which also includes setup, review, index and commit.

One deferred `target_sources(bsp_core ...)` line registers the module in
`cmake/startup.cmake`. `scripts/build.ps1` performs the required Release/Win32
build and existing CTest checks. No new routine tests, probes, fixtures or
runtime hooks are needed for the two-instruction whole-body comparison.
`docs/WORKER_VERIFICATION_CHECKLIST.md` was applied: the body is complete,
there are no callees or host-call claims, and field semantics remain unknown.

See `reports/goal15q_tagged_x87_b4_01.json` for the authoritative qualification
and build outcome. Ignored compilation and live-query evidence is retained
in `local/goal15q_tagged_x87_b4_01/`; its local verifier is
`local/goal15q_x87_verify.py`. These ignored paths are relative to worker
worktree `J:/PROG/battlestations-pacific-decompile-goal15q_b4_x87`, retained
for integrator review rather than published as source artifacts.
