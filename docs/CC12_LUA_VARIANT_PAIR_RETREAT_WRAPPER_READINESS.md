# Lua pair retreat wrapper readiness

`006EDE90..006EDE9B` is a complete 12-byte, six-instruction wrapper around
`006EDD80`. It captures incoming ECX in ESI before the child call, then copies
the **post-child ESI** value to EAX before restoring the wrapper's saved ESI
word. It does not return the child's EAX or load a result from pair storage.

This packet supplies Native readiness evidence only. Root must independently
read the complete wrapper before accepting it. The actual Source137 retreat
child is still pending Root build, emitted review and admission. There is no
wrapper Source declaration or implementation in this packet.

## Complete body and evidence

```asm
006EDE90  56              PUSH ESI
006EDE91  8B F1           MOV ESI,ECX
006EDE93  E8 E8 FE FF FF  CALL 006EDD80
006EDE98  8B C6           MOV EAX,ESI
006EDE9A  5E              POP ESI
006EDE9B  C3              RET
```

Body SHA-256:
`3d20fe5e8ca30c1a6d4bc3cef024ad8fdc9c398648954cd9368ea516b4fd9a3c`.
The installed PE maps this body to file offset 3,071,632. All 12 PE bytes
equal the live Ghidra byte query; Capstone's six instruction starts and
operations agree with the saved Ghidra listing. The report embeds the whole
small body, its decode, the query results and binary identity.

Each live query went through `bsp.py ghidra`, whose client verifies project
`bsp`, `/battlestationspacific.exe`, language and image base before access.
No Ghidra mutation or export change was made. Saved metadata reports one
block, six instructions, one direct call and the unhelpful prototype
`undefined FUN_006ede90(void)`. Register use in the complete listing supplies
the actual ECX/ESI/EAX contract; that saved prototype is not an ABI proof.

The indexed caller count is zero, and the live xref query found no references
to this address. The live callers endpoint returned `No callers found for
function: null`; that endpoint result is recorded as ambiguous. None of these
queries proves that the function cannot be reached indirectly at runtime.

## Register and stack sequence

Let `S` be entry ESP, `P` be entry ECX and `I` be entry ESI.

| Point | Effect |
| --- | --- |
| Entry | `[S]` is the caller return word; the wrapper reads no stack argument. |
| `PUSH ESI` | ESP becomes `S-4`; write `I` into that actual save slot. |
| `MOV ESI,ECX` | Set ESI to captured `P`; ECX remains `P`. |
| `CALL` | Write continuation `006EDE98` at `S-8`; child enters with ECX=ESI=`P`. |
| Child returns normally | Continuation requires a usable restored wrapper stack at `S-4`; ESI is whatever the child left. |
| `MOV EAX,ESI` | Copy that current ESI to EAX, before the following save-slot read. |
| `POP ESI` | Read the then-current word at `S-4`, restore it to ESI, advance ESP to `S`. |
| `RET` | Read the then-current return word at `S`; ordinary continuation leaves ESP=`S+4`. |

The incoming pointer is captured before the call. Its later use is a register
read, not a fresh ECX read, pair-field load or read of the wrapper's saved ESI
word. EAX equals the original `P` only under the child's ESI-preservation and
valid-control-backing conditions. A changed child ESI instead becomes the
wrapper's physical EAX result. A changed wrapper save word affects restored
ESI after the result has already been copied.

The wrapper passes no additional stack word and does not prepare an EDX dummy
argument. Its own instructions do not alter EBX, EDI or EBP. EAX is overwritten
after the child; ECX and EDX can retain child effects. No wrapper instruction
after the child modifies arithmetic flags. Child effects, faults and unusual
control flow are not erased by those observations.

The only direct wrapper memory accesses are its save/call/return stack words.
Any pair mutation belongs to the child. There is no local validation, null
branch, allocator, cleanup, rollback or SEH setup. A throwing/nonreturning
child does not establish the wrapper's ordinary return sequence.

## Child contract remains pending

The existing primary gate in
`local/cc12_reparent_and_reverse_pair_primary/` independently read the child
`006EDD80..006EDE08`: 137 bytes, 48 operations, SHA-256
`82fea256088f18debecf69b5b0200be8f1a44d809fdd00f6f93483ed41f9c5d4`.
The capture and approval files are pinned as accepted Native child evidence.
Only their status, record metadata and Source policy were read here; this
packet does not duplicate that body audit or decode its bytes.

The child worker reports the intended provider as
`bsp::retreat_native_lua_variant_pair_006edd80(void* actual_pair)`, a naked
`void __fastcall` entry using ECX alone, with no stack argument, EDX dummy,
`noexcept` or semantic EAX promise. This is a pending candidate contract,
not an admitted callable provider in this branch.
The worker subsequently committed candidate
`5af22c236547a0250a7b613c7a709b84d113e4ef` and sent it to Root; build and
admission remain pending at this readiness handoff.

The worker also reports that each child exit restores ESI from its current
saved word, including exits before tail dispatch. Nominal preservation needs
uncorrupted save/control backing and the selected Source17/CRT nonvolatile
register contract. Pair+4 writes can alias save or return backing; immutable
ESI restoration cannot be assumed for arbitrary aliases. The wrapper's
post-call ESI return must retain this qualification.

Before a Source wrapper can be accepted, Root needs to admit the actual
Source137 child, verify the selected call and its ESI/stack contract, and
independently read this complete wrapper. A future ordinary pair-identity
return interpretation must not silently turn physical post-child ESI into an
unconditional original-pointer guarantee. Native flags/fault timing, raw
aliases, caller ABI, CRT binding, lifetime, startup and gameplay remain open.

## Scope and provenance

Root admitted the separate Source43 candidate
`68763c79d3e0c999664e453c8460e8ad63584835` while this audit was in progress.
The worker synced published main `30a137097`, including its current Source107
receipt, without making worker edits to the initializer's four files.

The current baseline receipt is
`reports/cc12_native_lua_variant_header_initializer_primary_review.json`.
Its normal build ran from `2026-10-09T18:56:24.196838+00:00` to
`2026-10-09T18:56:40.997368+00:00`: three checks, 29 captured/replayed whole
objects and 33 positive Core definitions. All 107 input hashes match after
canonical-LF normalization; 105 also match raw bytes, with the two recorded
historical report line-ending differences. All four current artifact hashes
match exactly. The actual Source43 candidate report and compiled receipt are
pinned. Source105 is historical; it is not the final artifact baseline.

That initializer is not a provider for this wrapper. The baseline replay
records current integration state and does not close Source137, implement
the wrapper, or supply new ABI/startup/gameplay evidence. Prior object graphs
and full input manifests are not duplicated.

Only this document and
`reports/cc12_lua_variant_pair_retreat_wrapper_readiness.json` are owned by
this packet. No Source, CMake, ledger, GPR, test, probe, consumer, build or
runtime claim is added. Native byte agreement establishes this bounded
wrapper evidence; it does not close the pending child or prove runtime use.
