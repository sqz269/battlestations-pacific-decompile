# Original sampler word: bounded origin audit

The frozen run03 capture proves one invocation's input, not its final producer.
At stops `00B3BFF9` and immediately before `00B3C018`, physical
`P=0162E86C` and `[P-30h]=[0162E83C]=0162E964`. This equals the builder
pointer in that invocation. The sampler call was not executed. No source
default, universal upper-byte value, or builder-derived rule follows.

This is a read-only audit of the already published capture in
[NATIVE_ORIGINAL_SAMPLER_CAPTURE_CC10.md](NATIVE_ORIGINAL_SAMPLER_CAPTURE_CC10.md).
It adds no source code, Ghidra mutation, process attachment, game/helper/probe
execution, profile restoration, or copied-game rebaseline. The two changed log
files and stale pre-run overlay remain as recorded in that capture.

## What the native instructions establish

The captured `B3BFF9..B3C018` straight-line span pushes at `P-4` and `P-8`
and writes builder fields `+8C/+90/+94`. Those fields are at
`P+184/P+188/P+18C`; none is the word `W=P-30h`.

If the later nonempty sampler path is taken, `B5F100` uses that same physical
word as an incompletely initialized local: `B5F10B` writes its low byte, and
`B5F13C` reads the full DWORD. Both operations are future, unexecuted work in
run03. The capture does not establish sampler contents, source selection,
or a second sampler invocation.

Earlier, `B3BFB3` pushes one constructor argument and `B3BFB6` calls
`B5F9B0`. Three EH pushes and `SUB ESP,18h` place `B5F9C8 PUSH EBX` at
`W`. This is a static writer on that path. Its historical EBX value was not
captured. Later COM release, owner replacement and decrement/virtual disposal
calls can reuse the released stack or write through pointers, so this is not
proof of the final writer.

## Conditional matching-module explanation

The captured stale word at `P-24h` is `6C05EBDF`, corresponding to the
instruction after a `CALL ESI` in the matching D3D9 module. If the earlier
`B3BFD7` COM call entered D3D9 RVA `7EB90`, its nested direct call at RVA
`7EBAE` reaches RVA `489E4`. The nested `PUSH EDI` at RVA `489F0` would
then write the incoming builder pointer at `W`.

That explanation remains conditional: the earlier COM receiver, profile and
target were not captured, and neither the nested branches nor subsequent
writes were traced. Later callbacks could overwrite the word. A separate
matching ntdll path can write zero to the same physical location on one
unobserved branch. Stale stack values do not establish an active frame or
executed call history.

The three current DLL files match their recorded loaded-module hashes.
Five extracted disk ranges match those files, including their recorded
relocations. Listings use the recorded module base for instruction positions;
absolute operands retain disk preferred-base values. There are no live-byte
captures of the relevant D3D9/ntdll bodies. The short kernel32 decrement body
was captured at the two later stops, which does not identify earlier COM calls.

## Primary verification and limits

The primary independently verified all 33 audit artifacts and 1,154 prior
indexed rows, the 765-event frozen journal, both stack observations, six native
bodies captured at both stops, three module file identities and five extracted
disk ranges. The complete audit and relevant instruction schedules were
reviewed. This verification adds no claim of whole-machine replay, native EH,
ABI compatibility, full renderer initialization or gameplay validation.

The remaining evidence gaps are the earlier current COM target, relevant live
module bodies, taken nested branches, historical writes through the first stop,
descriptor sampler contents, and actual sampler execution. The retained input
must remain explicit until its required behavior is established.

The primary receipt is
`reports/native_original_sampler_word_origin_cc10.json`. The ignored archive
`local/cc10-platform-evidence/original-sampler-word-origin.zip` contains the
frozen worker audit, exact byte evidence, primary verifier and review receipt;
its hash is recorded in the receipt and local archive index.
