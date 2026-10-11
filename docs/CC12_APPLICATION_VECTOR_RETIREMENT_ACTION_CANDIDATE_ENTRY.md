# Application retirement action candidate entry

The admitted complete saved range `00C86200..00C86207` contains these eight bytes:
`8B 4D F0 E9 88 47 F6 FF`.
SHA-256: `2c08320829fdef733253cdfa9bbf17d2f4a0f6a6e69eab756c032554f9f5d910`.
Both exact reads agree. Independent manual decoding and actual detailed
Capstone 5.0.7 decoding agree:

```text
00C86200  8B 4D F0        MOV ECX, DWORD SS:[EBP-10h]
00C86203  E9 88 47 F6 FF  JMP 00BEA990
```

The load uses the **current contents** of the addressed memory cell. ModRM
`4D` has mod=1, reg=1 (ECX), r/m=5 (EBP); displacement `F0` is -16.
The signed E9 displacement is -637048, added to next EIP `00C86208`.
The physical jump leaves ESP unchanged and pushes no return address.
Neither the destination nor any adjacent body/data range was queried.

## Why this address was admitted

The accepted row at `00DB57B0` contains numeric DWORDs `FFFFFFFF,00C86200`.
Only under the conditional classic absolute EH-row layout could word1 be
an action address. Root separately admitted exact-entry metadata, then
paired metadata for all eight addresses in the actual saved body, and
finally the two exact byte reads, context-zero text for its two units,
four prototype metadata prefix lines and a closing metadata sweep.
This path does not authenticate an Original FH3 action role.

## Saved listing and physical behavior

The saved owner is `Unwind@00c86200`, entry `00C86200`, non-thunk,
no-return=false, with one complete eight-byte body range. All four
instruction/code-unit aliases and all interior addresses agree on units
of three and five bytes. C86200 has ordinary fall-through. C86203 has
default `UNCONDITIONAL_JUMP`, override `CALL_RETURN` and effective
`CALL_TERMINATOR`. The physical E9 opcode still performs a jump.
No saved-analysis annotation or flow override was changed.

The only prototype lines used are:

```text
Function: Unwind@00c86200 at 00c86200
Signature: undefined Unwind@00c86200(void)
Entry: 00c86200
Body: 00c86200 - 00c86207
```

Automatically returned text beyond that prefix and typed target objects
remain opaque. Actual function invocation and ABI do not follow from this
saved prototype.

## Conditional receiver and frame relationship

An explicitly added ninth immediate Git input is the positive parent
93-byte report at `reports/cc12_application_base_retirement_readiness.json`,
selector `native_evidence.reused_parent`. Its retained `007379A0..007379FC`
bytes have SHA-256 `ef5b16ae5487c295eddb16bee693072e2f57ca13d59f437ca2b544ea90e38297`.
All 28 instructions were actually redecoded offline and matched the retained
bytes, addresses, lengths and normalized text. No previous archive was opened.

Let A be parent entry ECX, Q be parent entry ESP, P be ESP at the actual
receiver spill, and B be EBP at candidate entry. Six pushes before that
spill give `P=Q-18h`. At `007379B7`, `8B F1` copies A to ESI. The spill
instruction **starts at `007379BA`**, encoding `89 74 24 08`, and stores
A to `SS:[P+8] = SS:[Q-10h]`. The candidate loads `SS:[B-10h]`.

Under the same SS segment, address equality therefore requires
`B=P+18h=Q` modulo 2^32. For the loaded ECX to equal A, the actual action
frame must supply that B and the memory cell must still contain A, whether
preserved or reestablished. This is a conditional address equation, not
proof of Original dispatcher invocation, frame binding or memory lifetime.
The retained ordinary parent E8 at `007379E7` computes the same numeric
target `00BEA990`; its equality with the candidate target adds no destination
behavior or unwind-safety proof.

## Evidence and limits

All Native captures identify `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, x86/32-bit, modification epoch98 and function
count64730. The archive retains 26 complete typed response bodies, six
count bookends, byte/text/prototype streams, commands, both Root gate
artifacts and producer bodies, collection preimages and actual offline
decoder output. One offline decoder prefix assertion used a literal two
bytes too long; the failed producer and disclosure are retained. Correcting
that literal yielded the successful independent decode. No Native query failed.

The pure ZIP proof retains nine finite positive Git input blobs and actual
frozen/staged/final Git commit/tree closure for exactly this document and
its report. The original eight-input freeze and executed sealer are retained
before the explicit parent-report addition. Original and distinct-copy
replays verify Git objects, manifest hashes, admitted metadata, physical
bytes and the frame equation using only the supplied archive after bootstrap.
Source subtree IDs are revision identity only; no Source contents were read.

This is Native evidence with a conditional receiver equation. It grants
no Original/FH3/ABI, destination behavior, state0 cleanup, ownership,
activation or runtime credit. No Source, build, test, probe, game, OS audio
or Ghidra mutation occurred. Root acceptance remains required before any
parent Source authorization; the unresolved invocation/frame/receiver and
target-behavior contracts remain explicit.
