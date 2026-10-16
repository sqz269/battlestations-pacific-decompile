# Controller-name pair destruction

This packet reconstructs whole `006A2BB0..006A2C3E` (143 bytes/46 starts)
and the logical pooled-key action `00C7F200..07` (8 bytes/two starts).
Handler `00C7F208..11` and the 44-byte EH block `00DAC39C..C7` are read-only
support. Names describe a provisional STL instantiation, not recovered symbols.

Root authorized Source after range publication at Main
`545e4cb781520ad5a8e4cbb782ce441222bb45dd`. The worker synchronized to
published descendant `02b89a4acfac4138c2e375a4d06668ce16e3cc50`.
The whole range retains accepted pin `755d02e0ecc50bb547f411ecb2bf7a2ff965500b`;
all prior eraser/leaf pins, 38 provider fingerprints and parent frozen artifact
sets remain unchanged. The worker readiness SHA256 is
`da176cea476d6e2de1c2de2b2d54bb12b0b34dab85d412aebb9891fd57a51042`;
Root's independent readiness SHA256 is
`b57e1cd5eaf986601fa1272e08620c4e57b4dc7517b4c1787a450ef5507d6d7d`.
Their earlier pending-publication wording is historical; Source authorization
and the range publication gate are now met.

## Interfaces and storage

Native ECX is the pair, EDX is unused, and there are no stacked arguments:
plain RET with incidental EAX. The new Source returns semantic void, retains
both physical register slots and adds the borrowed actual RawCtx reference,
so it uses `RET 4`. The pair owns a pooled key header at length0/data4 and
an embedded tree at +8, with head+C/count+10h. Source introduces no host pair,
node, container, callback, generic model/Kind/layout/context bridge or allocator.

The logical action explicitly receives the borrowed pair and RawCtx. Its new
fastcall interface uses ECX pair/EDX RawCtx and plain RET. Native instead
obtains the pair from runtime EBP-18h before its tail transfer. Source has no
EBP reader, fabricated frame, added header offset or SBO cleanup. Both new
interfaces are qualified Source APIs, not Original binary replacements.

## Capture and exception progress

| Phase | Established schedule |
| --- | --- |
| Before arming | Capture initial embedded head from pair+C, then head.left. Compute embedded tree=pair+8 and form real private 8-byte first/last iterators with embedded-tree owner identity and those captured endpoints, plus ignored private output. |
| Arm | Native state0 starts at `006A2BE3`, after captures and before the whole range. Source arms the borrowed pair/RawCtx key guard at that boundary. |
| Range | Invoke the published whole range, preserving its full/partial path and separate private iterator/child/recursive origins. There is no direct-subtree or full-prefix substitute. |
| Head release | After range success capture CURRENT embedded head, free it through the accepted current CRT, then store embedded head=0 before embedded count=0. The pair allocation is not freed. |
| Capture/disarm | Capture CURRENT key data4 while cleanup is still armed. Then disarm and end the guard's EH scope, including for null data, before normal key length/getter work. |
| Normal key release | Nonnull captured data reads CURRENT length0, wraps +1, resolves actual getter, then returns the CAPTURED block. Null data skips length/getter/return. No key header clear or reread is introduced. |

Normal release uses the existing actual RawCtx pool/getter/return providers
directly. It cannot delegate to the header destructor after disarming because
that would reread data4; using it before disarming would change failure
progress. The getter is still resolved for every nonnull captured return,
including eventual large frees or disabled small returns. Borrowed publication
cells remain actual 01090AA8 pool, 01090AA0 manager and 01090AA4 gate.

An exception while armed invokes the logical action, which uses accepted
`destroy_native_string_header_0041dd20` on the CURRENT pair header with the
same RawCtx. It does not retain an earlier data/length snapshot. The Source
guard destructor is explicitly `noexcept(false)`; neither it nor the logical
action inserts a noexcept adapter around the propagating RawCtx getter. A
second cleanup exception during C++ unwinding retains the current nested
failure/termination limit, separately from Original FH3/SEH behavior.

A normal getter exception occurs after disarming, with head/count already
zero and completed provider progress intact. It propagates without key-cleanup
retry, head reset rollback, extra node/pair free or header clear.

## Native transfers and EH

| Scope | Native transfer |
| --- | --- |
| Owned whole pair | `006A2BEB CALL 006A1B70` |
| Owned whole pair | `006A2BF4 CALL 00BF65AC` |
| Owned whole pair | `006A2C22 CALL 00419CC0` |
| Owned whole pair | `006A2C29 CALL 00BD1510` |
| Owned logical action | `00C7F203 JMP 0041DD20` |
| Read-only handler | `00C7F20D JMP 00BF6B43` |

Native DAC39C contains `(-1,C7F200)`; FuncInfo DAC3A4 words are
`19930522,1,DAC39C,0,0,0,0,0,1`: one unwind state, no try/catch map.
The Native action is MOV ECX,[runtime EBP-18h], then the listed tail transfer.
The compiler's Source guard frame and handler remain distinct from this
Original runtime frame and handler; no handler Source is promoted.

## Verification and limits

Fresh Source-phase queries verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. All 161 code bytes and 44 EH bytes match the
installed PE and both frozen readiness audits. All 50 decoded starts, six
rel32 transfers and six callee entries are checked (56 exact current starts).

Default `./scripts/build.ps1` passed MSVC Win32 Release, exit 0. All three
existing checks passed: reconstructed_math, native_math_differential and
tool_tests, 7.93 seconds total CTest time. No new tests or probes were added;
these existing checks do not execute this pair against Native game state.

All 299 executable bytes/115 physical instructions and 17 relocations of the
actual CMake object were reviewed. The public pair is 182 bytes/65 instructions,
with compiler-frame EBP and `RET 4`. State0 at +47 precedes range +53; current
head free +5C precedes head0 +61/count0 +6B. Data4 load +72 and retained spill
+75 precede compiler state=-1 at +78, then current length +83/getter +8A.
Unsigned +1 is scheduled at +92 after the getter using the captured pre-getter
length in preserved ESI. Source has one unwind state/no try map; the 42-byte/
16-instruction compiler EH section includes five INT3 and two NOP padding
instructions. The public logical action is 11 bytes/five instructions; no
manual EBP manipulation or normal-path header-destructor delegation exists.

Source/build/static ordering evidence does not prove Original incidental
registers/flags/instruction scheduling, private-frame aliases, heap/CRT/FH3/
SEH/RTTI/fault delivery, binary ABI, packet differential execution, application
adoption or game behavior. CurrentCpp/noexcept/provider ownership limits
remain. Existing populated AV/typed/RawCtx domains are preserved. No Ghidra
mutation or publication occurred in this worker. Controller subtree/range/
owner entries remain outside this Source packet.
