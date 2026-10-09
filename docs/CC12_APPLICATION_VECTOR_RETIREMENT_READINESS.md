# Application interior vector resize and retirement

This read-only audit captures the complete `00735F30..00735F7F` ordinary body:
80 bytes, 31 instructions and one direct call to `00735EC0`. The current raw
Application vector layout and concrete reserve service already exist in Source.
No new Source implementation, owner, table or reconstruction credit is admitted
by this evidence packet.

The [report](../reports/cc12_application_vector_retirement_readiness.json)
retains the full live bytes, installed-PE comparison, complete independent
decode, Source excerpts and input pins. The only native function freshly
queried is `735F30`; its reserve child and exceptional paths remain unexpanded.

## Actual receiver and complete ordinary schedule

ECX addresses the actual 12-byte vector header: data DWORD at +0, signed count
at +4 and signed capacity at +8. The stack argument is the requested signed
count, captured in EDI; ESI captures the receiver. ESI and EDI are restored,
and the entry ends with `RET4`. Its incidental EAX value is not established
as a semantic return value.

1. Compare requested count with the current signed capacity. Only a greater
   request calls `735EC0` with the same actual header and request. The child
   returns before the next current count load.
2. Capture the post-reserve current count into EAX. While signed EAX is below
   requested count, freshly load the current data pointer, calculate the slot
   address using low32 wrapping `data + EAX*4`, and write a zero DWORD only if
   that calculated address is nonzero. Increment EAX with wrapping arithmetic.
   Do not publish a new count during this growth loop.
3. Compare requested count with the current header count again. If smaller,
   set EAX to all ones, then repeatedly add that DWORD to the actual current
   count field and compare its freshly updated signed value with the request.
   Continue while the request remains smaller. Shrink changes the live count
   one step at a time; it neither reads nor destroys any retired element.
4. Store the requested count into the actual header unconditionally, including
   after either loop or a path that enters neither. Restore EDI/ESI and return.

The six-byte `8D 9B 00 00 00 00` at `735F4A` and two-byte `8B FF` at `735F6E`
are retained no-op instructions in the physical body. There are five signed
comparisons and one calculated-address zero branch. No validation, virtual dispatch, element destructor,
data free, publication change, lock or local exception frame occurs here.

## Qualified zero-count Application call

The already captured `7379A0` outer normal destructor passes request zero to
this helper on the actual receiver+8 header. It then reloads that header's
current data pointer and calls `BF6989`, before restoring the outer receiver
and calling `BEA990`. The vector helper's return does not retire the outer
Application or any element reference.

With an established nonnegative capacity and count, request zero skips reserve
and growth, decrements the current count to zero, then stores zero again. It
leaves data, capacity and all element words unchanged. Replacing that sequence
with a vector clear, per-element release or a single early count assignment
would change the observed stores. The actual header invariants are a caller
qualification; this audit does not impose new guards. In particular a negative
capacity can reach reserve even for zero request, and a negative count can
enter growth. Such cases are not silently converted to an empty-vector path.

## Existing Source dependency and binding boundary

`NativeApplicationPointerVectorStorage` in `native_shader_preload.hpp` already
describes the actual 12-byte header with signed count/capacity. Its
`NativeApplicationPointerVectorAllocation` supplies the actual allocation/free
domain. The existing `reserve_native_application_pointer_vector_00735ec0`
uses that same storage and binding, clamps the requested capacity to one,
preserves current count/source reads, and frees current old data before
publishing new data and capacity. Its existing allocation, exception and
Native CRT qualifications remain unchanged. This packet pins and reviews its
Source; it does not renew the child's native or emitted-body proof.

A bounded future resize helper can borrow those existing storage/allocation
contracts and implement this complete ordinary schedule. It requires no new
Application overlay, synthetic owner, table, callback service or allocation
domain. That prospective helper would still be a new C++ interface with
explicit bindings, not an admitted binary replacement. Common Application
construction, callable profiles and full exceptional retirement remain open.

## Validation limits

All 80 fresh bytes match the current installed image and decode completely
to 31 instructions, including the single reserve call and both no-op encodings.
Current Ghidra lists all 31 starts. The target check reports saved project bsp,
program `/battlestationspacific.exe` and unchanged function count 64729.
Current Source/target/prior inputs are hashed with bounded excerpts. There
are no Source, CMake, ledger or Ghidra edits, builds, tests, probes, new Original
credit or runtime/gameplay assertions in this packet.
