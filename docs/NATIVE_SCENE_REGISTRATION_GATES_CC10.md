# Raw scene registration gates

This packet reconstructs the complete three normal bodies below over actual
storage. It composes the genuine raw registry providers published in main
fb98fe17a. Existing logical `SceneAttachmentRuntime` helpers remain separate.

| Entry | Half-open body | Bytes | Final instruction |
|---|---|---:|---|
| B83D50 | [B83D50,B83D8E) | 62 | B83D8B RET4, 3 bytes |
| B83EC0 | [B83EC0,B83EF3) | 51 | B83EF0 RET4, 3 bytes |
| B7AA70 | [B7AA70,B7AA76) | 6 | B7AA75 RET, 1 byte |

Original gate ABI is resource in ECX, node argument at stack+4, RET4. The
resource is actual3Ch storage; its actual28h registry starts at +14h. Registry
keys are borrowed actual node pointers. These bodies perform no ownership
increment, decrement, canonical admission or rollback.

## Capture and callback order

Both gates capture the incoming node and that node's numeric profile before
calling B7AA70. The getter loads the current DWORD at 0109018C; it does not
initialize a descriptor or return cached runtime metadata. The gate then
reads the captured profile's current +0C target and invokes the predicate on
the captured node with the captured token. Only AL controls the branch.

On false, no registry operation or argument/output store occurs. A predicate's
own changes remain. On true, B83D50 prepares key/output pointers, addresses
the captured resource+14, overwrites the original incoming argument word with
the captured node, and calls B83700. B83EC0 similarly prepares the key pointer,
addresses the captured registry, overwrites its incoming word, and calls
B83E50. Neither rereads a replacement node argument after the predicate.

The source uses separate caller-owned insert and erase frames. Insert exposes
the native12B result preimages and the volatile incoming argument/key word;
erase exposes its volatile incoming argument/key word. Each embeds the genuine
registry provider's frame. Only reached native argument cells are initialized;
nested opaque preimages and result padding remain intact. The insert Acquired
sidecar is separate, fresh, persistent and untouched when the predicate rejects.
Registry allocation failure keeps the native reached mutations and acquired
allocation diagnostics. No new cleanup is added by the gates.

## Type binding

Known numeric node/camera/light/directional profiles are D62C88, D62CF0,
D62F58 and D62FB0. Their supplied views borrow actual current numeric tables;
the views are never invoked as host C++ vtables. Current targets B6F570,
B71CE0, B7C580 and B7C6D0 reuse the existing genuine node, camera, light and
directional bootstrap predicates. They read current descriptor tokens and do
not initialize types. The camera provider must share the same actual light,
node and root token domain. A descriptor copy or a cached token is insufficient.

Additional profiles require an explicit side-effect-free metadata resolver;
additional targets require an explicit genuine predicate binding. The binding
returns uint8 AL. Unknown profiles/targets have no base predicate fallback.
Missing required bindings produce a source error, not successful registration.
All reached callback-modified pointers and cells must remain valid.

## Verification and limits

All 119 live Ghidra bytes match the installed PE. Exact complete listings and
four direct call rows plus two indirect predicate sites are retained. Ghidra
was read-only. `scripts/build.ps1` passed strict MSVC Win32 and both configured
CTests (`reconstructed_math`, `tool_tests`). No tracked test was added.

One ignored original/source sequence used actual3Ch storage with its registry
constructed by genuine B83600, initialized actual descriptor cells, raw node
prefix storage, genuine current predicates, and the genuine B83700/B83E50
providers. Copied original gates/getter were relocated to those same providers.
The sequence checked:

- Camera rejection with nonzero upper return bits and zero AL.
- True directional predicate callbacks changing the incoming word, node profile
  and current token; the gate restored the captured actual node key.
- Insertion/removal of that borrowed key, output+9..11 padding preservation,
  and no changes to actual resource/node counts or the type counter.
- A source known-numeric camera predicate route, with no derived binding call.

The probe compiled with active assertions (`#ifdef NDEBUG` rejects compilation)
and no `/DNDEBUG`. Its exact command is retained in the report and
`local/output/cc10_registration_gates_probe.cmd`; it uses `/MD`, `/fp:strict`,
and `/link /MANIFEST:EMBED`. The shared source predicate/registry providers are
an explicit differential boundary. This does not execute the original nested
registry machine bodies, native CRT failure/FH3, or full resource construction
and terminal cleanup. Caller frames expose stated cells, not arbitrary native
private-stack/saved-register aliases or binary replacement ABI.

Full B6D890 root propagation, base/camera B6ED80/B6EE10, light B7C020/B7BD60,
B6DFA0/current18, and raw node/light/outer-scene terminal cleanup remain separate.
The existing node B6F440 and light B7C5B0 C++ bodies still directly use host
scene resources during cleanup. These gates alone therefore do not admit raw
scene attachment, AC59A0, application rendering or gameplay parity. The bounded
readiness handoff is preserved in ignored
`local/cc10_raw_scene_attachment_readiness.md`.
