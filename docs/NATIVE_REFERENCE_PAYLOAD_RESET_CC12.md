# Whole reference payload reset 008F0380

`reset_native_reference_payload_008f0380` reconstructs the entire
`[008F0380,008F03A2)` body: 34 bytes / 12 instructions. The descriptive name
is a hypothesis, not a recovered symbol. The original SHA-256 is
`cebff76019d054f62e5664408c7f5094a557b977e8db9b049c0347fe3e260437`.

ECX addresses a genuine live writable eight-byte reference payload, with
an opaque full DWORD scalar at +0 and an owned pointer at +4. Incoming EDX
is unused; there are no stack arguments. The entry saves/restores ESI and
uses plain RET. It captures +4 and tests the pointer. On the nonnull path it
calls canonical current free, restores the argument stack and only then
clears +4. Both paths clear the full scalar DWORD at +0. The null path does
not write +4. Thus the null path writes four distinct bytes, the nonnull
path eight; both finish with two zero DWORDs. No scalar value is read.

The sole call rebinds Original `00BF6989 -> 00BF65AC` to the accepted
`singleton_lifetime_free`, which calls current `std::free`. Only its four-byte
CALL operand `[12,16)` changes; all other 30 bytes remain literal. The leaf
frees no payload root, changes no surrounding phase/tag/kind/owner field,
and performs no virtual dispatch, lookup, allocation, insertion or startup.
It does not implement or call the separately blocked `008F0600` virtual reset.

Require actual accepted physical duplicate/0310/0340 payload provenance.
Pointer +4 is null or a sole-owned real matching current-heap duplicate.
The payload, owned allocation and active frame are disjoint, and payload
storage remains writable through the final stores. Retire the allocation
once, never inspect its bytes after free, and finish payload retirement
before reusing/discarding its owner. The full scalar accepts all bit patterns
and is reset to zero. This is the ordinary successful current-free domain:
no alias, reentry, concurrent mutation, invalid memory, fault or overflow.
The accepted `03F0` array header has pointer +0 and size +4, the reverse
layout, and is not interchangeable. No new surrounding class/caller domain
or existing 0310/0340/03F0/0640 credit is claimed.

The null path leaves EAX zero, ECX as the payload and incoming EDX unchanged;
TEST supplies the final arithmetic flags, with AF undefined. The nonnull
path inherits free's volatile residuals, then ADD ESP,4 supplies arithmetic
flags before the stores/POP/RET. The public API returns void and makes no
EAX/flags/FP result promise. Current ordinary provider behavior is qualified;
there is no full Original ABI, private CRT/EH, failure-unwind, rollback,
complete caller/class/vtable, runtime or game compatibility claim.

Before the sole strict MSVC Win32 compile, this packet copied/hashed 119
candidate source/include inputs, 34 current readiness source/tool/report
inputs, 64 tools including both MSVC chains and `clui.dll` files, the build
inputs and exact compile command. All 135 files of the sealed readiness
family were verified and copied without changing the original family.
The compiler's 81 actual source/include inputs were all pre-pinned. Options
included `/MD /W4 /WX /fp:strict /O2 /Ob2 /Oy- /EHsc /Gy /GL- /Bv` and
`/sourceDependencies`. There was no header gap, compile retry or replay.

The strict object contains 1,228 bytes, five sections, 15 symbol-table indices,
one 34-byte/12-instruction code section, one REL32 relocation and zero
noncode relocations. Every file byte, header, primary/auxiliary symbol,
string-table byte and noncode section is accounted for. Noncode content is
305 bytes of linker directives, 224 bytes of CodeView metadata, four bytes
of unreferenced AVX2 weak-value zero-fill and 40 bytes of compiler checksums.
There is no additional code or callable vtable in this object.

The new TU remains unregistered for the normal repository build. Its compile
proof is the strict object; the normal build validates existing registered
code and checks. One normal build passed both existing checks, 2/2 in
7.30 seconds. Exact build results, own tool receipts and full byte audits
are in the report and `local/WholeReferencePayloadResetSource01` family.
No new tests/probes, selected-entry or Native execution were performed.
The own UUID is `01a141c0-29a9-76c3-a341-c05f4800eeb8`; this Source phase starts
at brief chunk `ff543f`, distinct from earlier tasks in that same session.

The inherited readiness's global analysis/index qualification remains: live
leaf bytes/bodies were stable while other analysis changed; a real `008F5348`
call site had no live containing function, so no whole caller is inferred.
Working config is LF 455 bytes versus Main CRLF 467 bytes, with identical
normalized bytes and parsed JSON. This packet changes no CMake, registry,
ledger or Ghidra data. Primary review and integration control admission.
