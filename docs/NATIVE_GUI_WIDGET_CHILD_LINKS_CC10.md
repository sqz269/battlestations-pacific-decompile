# Raw widget classification and child links

This packet reconstructs five raw bodies. It does not change AAA710 or AA6560,
create a logical widget owner, substitute a container, or port a library.
Names are descriptive hypotheses rather than recovered symbols.

| Entry | Inclusive end / bytes | Native ABI | Source coverage |
| --- | --- | --- | --- |
| AA2490 | AA2659 / 458 | ECX actual8h string; EAX type; RET at AA2659 | Complete normal classifier |
| AA83A0 | AA83EE / 79 | ECX parent widget; stack child; RET4 at AA83EC, 3B | Complete normal raw detach |
| A9BD50 | A9BDEB / 156 | ECX actual12B list; stack payload-cell pointer; RET4 at A9BDE9, 3B | Complete valid-list remove-all |
| A9B790 | A9B7C2 / 51 | Unused ECX; stack next,previous,source cell; EAX node; RET0C at A9B7C0, 3B | Alias to existing8665F0 |
| A9D480 | A9D512 / 147 | ECX actual12B list; stack increment; RET4 at A9D510, 3B | Alias to existing8675E0, host EH boundary |

All five live Ghidra bodies in `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, match the installed PE. The report records hashes,
inclusive ends, final instruction lengths and all 32 direct native call sites.
No Ghidra mutation was performed.

## Classification

The input remains the actual native eight-byte string: counted length at +0,
buffer at +4. AA2490 captures the buffer first, using the borrowed F8BC60 empty
buffer when null. It then calls existing
`reverse_find_native_string_bytes_004bcb80` with the borrowed CE7890 underscore
set and INT_MAX. The captured buffer plus returned index+1 forms the suffix;
FFFFFFFF wraps to offset zero.

The seventeen native ASCII names are compared in order through the current
CRT `_stricmp`: Icon6, Text3, Group2, Progbar5, Line4, Scrollbar7, Grid8, Model9,
Movie10, Listbox11, AnimIcon12, Curve13, Sound14, Button15, ClipBox16, Section17,
FrameBox18. Every miss returns zero. Counted reverse-search and C-string
comparison remain distinct: an underscore after an embedded NUL can select a
later suffix, while a short counted length does not truncate the comparison.
There is no string_view conversion or input copy.

## Detach and current node identity

AA83A0 captures the child identity and returns for null before accessing the
parent. If the initial child+4C is nonnull, the source calls the existing actual
requested-parent-null B6E680 path. It rereads child+4C for B6D890(null root),
then reads current parent+4C. A nonnull parent node causes a fresh child+4C
read for B6D940. Each raw address resolves through the same live
`SceneAttachmentRuntime::resolve_key(...).transform`; no raw pointer is cast
to a companion or replaced with a logical widget transform.

The current parent+64 list is processed after those calls. Child+70 is cleared
only after removal completes. The successful domain requires any node pointer
used after a callback to remain valid and bound; the initial nonnull child gate
is not repeated before later required node calls.

## Actual circular list and aliases

The list header is allocator/preimage+0, sentinel+4, count+8. Its nodes contain
next+0, previous+4, payload+8. This is the existing verified
`NativeEffectDeletionListStorage`/`NativeEffectDeletionNode` layout, already
used by raw GUI lifetime code. A9BD50 captures the initial sentinel, then the
payload DWORD, then the first node. Loop termination uses that captured end;
iterator validation uses the current head.

All matches are removed, never their payload objects. The next node is captured
before unlinking. The source stores previous->next, reloads the current next
and previous for next->previous, frees the removed node through existing
`singleton_lifetime_free`, then decrements the current count. Capturing the
payload once permits its input cell to reside in a removed node. Three current
head checks retain the existing CRT invalid-parameter entry. Impossible
valid-reference list-base checks and native failure delivery are excluded.

A9B790 and existing8665F0 are instruction-equivalent after their sole REL32 call
relocation. Both allocate12B, publish next and previous, then read the source
payload cell. A9D480 and existing8675E0 match after normalizing three REL32 calls
and their different EH handler immediate. Both use unsigned
`3FFFFFFF - captured_count < increment` and store captured_count+increment.
The existing owning host length-error projection is reused. This comparison
does not establish native EH-handler or exception ABI equivalence.

## Validation and limits

The strict MSVC Win32 build and both enabled CTests pass. All 32 call rows
verify. The ignored focused probe is `local/output/cc10_child_links_probe.cpp`;
the report retains its exact `/MD /fp:strict /MANIFEST:EMBED` build command and
source/executable hashes. A compile-time NDEBUG rejection keeps assertions
active; compilation and execution finish with exit zero.

The probe executes the five original bodies and original4BCB80. Classifier CRT
calls bind to the same current CRT as source. Original detach's node calls bind
to the same existing actual-node providers, using real node storage and
canonical scene companions. Forty-three classifier pairs cover all seventeen
IDs, case/prefix handling, borrowed fallback, counted-length versus NUL cases,
and unknown keys. Four detach pairs compare 1,960 widget/node bytes plus ring
payloads and callback traces, including callback replacement of both current
nodes and the sentinel. Additional comparisons cover a target cell in a freed
matching node, node creation, and four count updates including unsigned wrap.

The fixture tests this packet's composition; it does not newly compare complete
original node-provider bodies. Native count-overflow EH is not executed. Free
ordering is established by assembly/source inspection, without a CRT port.
The successful contract requires live retained storage, finite valid rings,
matching allocation/free domains, genuine node callbacks and live borrowed
literals. Native CRT locale/high-byte implementation parity, invalid-parameter
delivery, allocator failures, FH3/SEH identity, outer ABI/full machine state,
installed application binding and game parity remain outside the claim.
