# Application singleton descriptor prefix

**NO_CPP_GO; NO_FRAME_ACTIVATION_GO.** Exactly 12 data bytes at `00E01CC4`
establish the raw DWORDs `19930522`, `00000002`, `00E01CB4`. A comparison with
previously frozen MSVC **14.51.36231** definitions supports a qualified classic
x86 absolute FuncInfo-prefix interpretation. No remaining descriptor bytes,
unwind-map entry, cleanup action or external handler target were read.

## Source, mapping and byte admission

The isolated worker was fast-forwarded to Main
`065f692e737236c5028fa40985c9d4806509bdc7` before claiming the exact data range
`00E01CC4..00E01CCF` and the two output files. The proof freezes all **4,210**
tracked src/include files as Git blobs and working bytes, plus 22 supporting
files. Source is unchanged from the preceding handler packet's
`281e91b6d5fa9dba441e5ec60b14874e4b9c90d6`. Later or separate Source/build
results receive no credit here.

Typed metadata at modification **88** identifies project `bsp`, marker
`C:\Users\sqz269\bsp.gpr`, program `/battlestationspacific.exe`, x86/32-bit
little-endian language, and 64,730 functions. Before/after typed captures agree.
At the queried address there is a defined `DataDB` unit of length **36**, with
no instruction, instruction flow, function at address or containing function.
This is existing listing metadata; the unit's other 24 bytes were not read.
No Ghidra data-type name or component-field listing was queried.

The CLI has no segment-map subcommand. Between the verified typed brackets,
the read-only MCP `list_segments` query explicitly targeted
`battlestationspacific.exe`, with offset zero and limit eight. Its complete six
segment names/ranges place all 12 approved bytes in `.rdata`,
`00CE2000..00E07FFF`. After mapping/listing admission, the CLI read exactly 12
bytes successfully. This establishes saved-program byte readability; segment
names and that read do not establish OS page protection or live-process state.
All other live questions used `bsp.py`; no inline Ghidra script ran.

| Address | Exact little-endian bytes | Raw DWORD |
| --- | --- | --- |
| `00E01CC4` | `22 05 93 19` | `19930522` |
| `00E01CC8` | `02 00 00 00` | `00000002` |
| `00E01CCC` | `B4 1C E0 00` | `00E01CB4` |

The 12 bytes have SHA-256
`e8281ec217a6733159b8180f15f97ff9245d2d30ddc3409167e2c68769bd2846`.
Only data was captured: no new instruction/body, image-file comparison, Native
execution or GPR mutation. The original handler's `MOV EAX,E01CC4` remains the
separate **epoch-85** ten-byte prefix proof. The normal singleton constructor
remains the separate **epoch-79** 145-byte proof.

## Qualified compiler-definition comparison

The supplied existing `cc12_locale_pool_profile_unwind_readiness/evidence.zip`
was physically checked against SHA-256
`ea19aa762cd4de2c8dff8d0a2d09b989d7c4b9d1eec9c08f1ebb8aea3e1c4570`
and size 22,034,395 bytes. Exactly four previously retained definition members
were copied: `ehdata.h`, `ehdata_values.h`, `ehdata_forceinclude.h`, and
`Microsoft.VCToolsVersion.default.txt`, totaling 35,172 bytes. Their individual
lengths/hashes match the supplied pins and the original capture metadata.
The retained version file and capture both identify **14.51.36231**.

No installed compiler header or online source was opened. The old capture JSON
is retained opaquely for its compiler-definition/version/provenance fields;
its other Native records were not interpreted or replayed for this comparison.
The original zip was hashed locally but is not copied into this packet. Portable
replay verifies the complete copied definition members and their capture fields;
it does not reconstruct or reopen that zip. Original installed-header paths are
historical provenance, not current filesystem observations.

The retained definitions provide the following evidence:

* `ehdata_values.h:22..31` defines `EH_MAGIC_NUMBER3` as `19930522`.
* `ehdata.h:240..274` places 29-bit `magicNumber` and three-bit `bbtFlags` in the
  first DWORD, followed by signed `maxState`. Its stated meaning is the highest
  state plus one, hence the number of unwind-map entries; this is not an inference
  from a field name alone. The non-relative branch next contains `pUnwindMap`.
* `ehdata_forceinclude.h:24..42` selects `_EH_RELATIVE_FUNCINFO=0` for the ordinary
  non-hybrid x86 branch and four-byte packing. `ehdata.h:87` defines the state
  index as `int`; `ehdata.h:171..190` describes an indexed unwind map with signed
  destination state and an action pointer in its absolute form.

Under that **conditional classic x86 absolute layout**, the observed prefix
matches magic `19930522`, BBT flags zero, signed `maxState=2`, and absolute
candidate map pointer `00E01CB4`. A two-entry map with two DWORDs per entry would
occupy 16 bytes. That extent comes from the retained definitions and actual
count value, not from the next symbol, descriptor adjacency or a guessed body.
The 36-byte saved data-unit extent is consistent with that modern structure's
size but neither proves its full contents nor authorizes reading them.

This comparison does **not** identify the original game compiler/version,
prove the full descriptor or handler ABI, establish runtime compatibility, or
show actual FH3 dispatch. The uninspected suffix includes the fields that would
describe try blocks, other maps and EH flags under that layout; their values
remain unknown. Two candidate entries do not imply two nonnull actions or tell
which cleanup operations run.

## Failure and custody remain unresolved

The preceding handler still encodes `JMP BF6B43` with its saved `CALL_RETURN`
override. Neither the target nor the override was changed. The descriptor
prefix supplies no destination-state value, cleanup target, guard-destruction
call, receiver destructor or publication policy. The earlier constructor's
observed state 0/1 stores are compatible with the conditional count; they are
not independently proved unwind transitions or post-unwind object states.

Nine complete Source searches and 14 bounded excerpts replay against the closed
Source corpus. The queried descriptor/FH3 definition terms and these handler
addresses have no Source hits; the four Application-authority-name hits remain
comments. Existing manager/guard/registration providers remain positive Source
boundaries, with no raw Application authority or callable `D68BC4`, `D68BC8`,
`CFEAB0` deletion profiles composed here.

Original receiver, current publication/registration argument and captured section
retain the earlier separate identities. Stable raw owner storage, its actual
+14 reference, authority cells and borrowed services must outlive registration,
startup/shutdown, escaped references and retained failure records. No reset,
unregister, free, retry or destructor policy follows from the prefix. Normal
callable retirement remains open. `E1AE78` process-duplicate ownership is separate.

## Finite replay and next proposal

All 287 preceding handler pins are copied unchanged and replayed, retaining
nested 230 base, 184 constructor and 124 authority pins and their original
Source/Native/build epochs. Current Source/Git closure, searches/excerpts,
typed metadata, mapping, capture order, exact bytes and compiler-member hashes
are recomputed. Physical relocation copies only declared files and report;
guarded replay permits no outside reads, writes, subprocesses or network.
Actual-Git verification checks both current/prior Source corpora and support
blobs, then the two exact committed output files. These are static artifact
checks, with no C++/CMake/ledger/GPR/analysis edit, build, test, probe, Native/game/
OS-provider execution, original ABI or actual unwind result.

Next proposed packet: `cc12_application_singleton_unwind_map_readiness`.
After its own metadata/mapping gate and exact lease, admit only the candidate
**`00E01CB4..00E01CC3`**, at most **16 bytes**, to inspect the two conditional
eight-byte map entries as raw values first. This proposal does not authorize a
read now. The remaining descriptor suffix, action bodies, destructors,
`BF6B43` target and other handlers require new explicit finite scopes. No body
creation, classification, prototype, no-return or flow repair is proposed.

Evidence: `reports/cc12_application_singleton_descriptor_prefix_readiness.json`
and its declared local finite bundle.
