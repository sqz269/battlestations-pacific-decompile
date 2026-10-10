# Application singleton unwind map

**NO_CPP_GO; NO_FRAME_ACTIVATION_GO.** Sixteen exact data bytes establish a
conditional declared chain `1 -> 0 -> -1`, with encoded actions `CC7238` and
`CC7230`. Only metadata was inspected at those action addresses. Their bodies,
frame requirements, actual invocation and cleanup effects remain unproved.

Source is frozen at `c0805db020542abdd71e17623dbe92708d246f94`: all 4,210 tracked
src/include files, their Git blobs and working bytes, plus 26 supporting files.
Source is unchanged from the preceding descriptor packet. All 355 preceding
pins are copied unchanged, including nested 287/230/184/124 proofs and the
physically retained compiler-definition artifacts. Earlier Source/build results
and seven selected historical epochs retain their original qualifications.

## Native admission and raw values

The exact range `00E01CB4..00E01CC3` was leased before reading. Typed metadata
identifies one defined 16-byte DataDB, no instruction or containing function.
The explicit-program read-only segment query places the entire range in
`.rdata`, `00CE2000..00E07FFF`; the subsequent exact 16-byte read succeeded.
That proves saved-program readability, not live OS page protection.

Every batch was bracketed by verified project/program metadata: project `bsp`,
`C:\Users\sqz269\bsp.gpr`, `/battlestationspacific.exe`, x86/32-bit little-endian,
modification **90**, function count 64,730. Before/after typed records agree.
Only segment mapping used MCP `list_segments` because the CLI lacks that
subcommand; all other live queries used `bsp.py`. No mutation or repair ran.

| Address | Exact bytes | Raw DWORD |
| --- | --- | --- |
| `E01CB4` | `FF FF FF FF` | `FFFFFFFF` |
| `E01CB8` | `30 72 CC 00` | `00CC7230` |
| `E01CBC` | `00 00 00 00` | `00000000` |
| `E01CC0` | `38 72 CC 00` | `00CC7238` |

The report pins the exact bytes and their SHA-256. No descriptor-suffix bytes,
action instruction bytes/text, external target bytes or installed PE image
were read. This table is epoch 90; its retained descriptor prefix is epoch 88,
handler prefix epoch 85, and normal constructor body epoch 79. These are
separate observations, not a newly captured atomic descriptor/body snapshot.

## Conditional entries and action metadata

The unchanged frozen MSVC **14.51.36231** definitions provide the qualified
classic x86 absolute interpretation: an entry contains signed `toState` and
an action pointer, two DWORDs total. The preceding prefix supplies conditional
signed count two and pointer `E01CB4`. Definition semantics, not field names or
address adjacency, establish that candidate extent.

| Entry index | Conditional signed destination | Encoded action |
| --- | --- | --- |
| 0 | -1 | `00CC7230` |
| 1 | 0 | `00CC7238` |

The resulting declared graph is `1 -> 0 -> -1`. It is not an execution trace,
proof of action order during a particular exception, or proof that any action
returns. The compiler comparison does not identify the original compiler,
runtime, handler ABI or actual FH3 invocation.

Both nonnull action addresses were leased before metadata-only queries:

| Entry | Complete saved range | Stored prototype / flags |
| --- | --- | --- |
| `Unwind@00cc7230` | `CC7230..CC7237`, 8 bytes | `undefined(void)`, NoReturn false, not thunk |
| `Unwind@00cc7238` | `CC7238..CC723F`, 8 bytes | `undefined(void)`, NoReturn false, not thunk |

Each range is independently complete and contiguous. Entry instruction length
is three, with default/effective fallthrough and no override at that first
instruction. No terminal instruction was inspected. Prototype summaries report
two instructions and one call each, naming `BSP_SingletonBase_ResetProfile` and
`BSP_SystemSingletonGuard_Destroy`. These are saved metadata hints, not verified
opcodes, register contracts or cleanup effects. In particular, a summary call
count cannot distinguish a machine CALL from the kind of overridden JMP already
observed in the separate handler prefix. Existing names/prototypes are unchanged.

The only match to the constructor is the already pinned state stores: DWORD 0
at `BEA830` and byte 1 at `BEA861`. Their equality to declared indices does not
bind an action's incoming frame/registers to the receiver or guard spill. No
post-unwind publication, registry, lock or object state follows from this map.

## Custody, proof and next proposal

Nine whole-Source queries and 14 excerpts retain their lexical limits. Existing
manager, guard and registration providers do not compose an Application owner
or callable deletion profiles. Original receiver, current publication/selected
registration argument and captured section remain separate identities. Stable
raw storage, its actual +14 reference, authority cells and borrowed services must
outlive escaped references and retained startup/shutdown/failure records. No
reset, unregister, free, retry or destructor policy is admitted. E1AE78 remains
a separate ownership gate; normal callable retirement also remains unresolved.

Finite replay checks current Source/Git closure, prior proofs, compiler-artifact
identity, capture order, mapping, exact data, conditional entries and metadata.
Physical relocation copies only declared files and report; the verifier forbids
outside reads, writes, subprocesses and network. Actual-Git replay checks both
current/prior Source corpora and support blobs and both committed output files.
No C++/CMake/ledger/GPR/analysis edit, build, test, probe, Native/game/OS-provider
execution, binary ABI or actual unwind result is added.

Next single proposed packet: `cc12_application_singleton_unwind_actions_frame_readiness`.
Refresh typed metadata for both encoded actions. Only if the same complete
eight-byte hulls remain, claim their exact ranges and read those bodies:
**at most eight bytes each, sixteen total**. Missing, changed or incomplete
ownership is a handoff, not permission to infer boundaries. Compare recovered
operand requirements only against the already pinned constructor frame stores;
retain uncertainty about the incoming action frame/EBP binding. No callee body,
descriptor suffix, other handler, `BF6B43`, prototype/flow/no-return repair or
actual FH3 invocation is included in this proposal.

Evidence: `reports/cc12_application_singleton_unwind_map_readiness.json` and its
declared local finite bundle.
