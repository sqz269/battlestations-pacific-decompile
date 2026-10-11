# Ordinary Application retirement: failure-handler metadata

The two explicitly admitted metadata sites are closed at fresh modification number **98** and function count **64,730**. Both queried addresses, `00CC7260` and `00CC7265`, have no function-at or containing function. Each reports a five-byte instruction unit. This establishes saved metadata at those sites; it does not establish handler code, operands, complete extent, physical opcodes, target behavior or exceptional cleanup. No Source implementation or successor query is admitted.

The dependency comes from the accepted ordinary-retirement audit at `00BEA8B0`. That inherited 153-byte/40-instruction normal audit identifies local handler pointer `CC7260`, initial state -1, state 0 after the first receiver-table change, and state 1 after the optional Enter/increment over captured section S. It explicitly withholds exceptional behavior. Its exact document/report and the Root WinMain caller primary document/report are physically retained as qualified inputs. No fresh `BEA8B0` query, old Native graph replay, compiler replay or inherited runtime credit is added. The old retirement audit's historical count 64,729 remains distinct from this packet's fresh 64,730 epoch.

| Metadata | `00CC7260` | `00CC7265` |
| --- | --- | --- |
| Actual observations | Five: initial before/gate/after, then the second gate's before/after | One: second gate only |
| Listing | All four instruction/code-unit-at/containing records agree: InstructionDB, instruction, length 5 | Same shape, at the second address |
| Ownership at queried address | Function-at null, containing function null, functions empty | Same |
| Saved flow override | `NONE` | `CALL_RETURN` |
| Default classification | `FALL_THROUGH`; no call/jump/conditional/computed/terminal flags | `UNCONDITIONAL_JUMP`; jump only |
| Effective classification | Same as default | `CALL_TERMINATOR`; call and terminal true, other flags false |
| Default/effective fallthrough | Both `00CC7265` | Both null |
| Separate fallthrough override | False | False |
| Default/effective target addresses | Empty | Both contain only `00BF6B43` |

Root first reviewed the actual `CC7260` gate, then separately admitted metadata at its positive fallthrough `CC7265`. The second gate's bookends are at **CC7260**, not repeated CC7265 observations. Every batch retained verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86:LE:32:default, image base `00400000`, count and modification bookends. Interior bytes, neighboring units and the target address were not queried. Two adjacent reported units do not establish a ten-byte handler body or all-byte function ownership.

The second endpoint automatically included a target-function object under `direct_call_targets[0].function_at_target`. Its complete raw response is preserved, so that whole capture is **not strictly site-only**. The object remains opaque and quarantined from findings; derived projections retain only the permitted address/default/effective target flags. No name, target flags, FH3 identity, ABI, no-return behavior, target body or target function ownership is credited from that object. No separate target query occurred.

The second site's default/effective flow difference is a fact about saved analysis metadata. Without separately admitted physical bytes or operands, it is not evidence of a physical CALL/RET/tail-jump sequence, an annotation error, or a repair requirement. Nothing here proves an exception unlock, publication rollback, final receiver-table write, outer free, guard destruction, replay policy or selected Source failure policy. The concrete exceptional-retirement dependency remains open beyond these two metadata observations.

Baseline `1332c453de9235fb263ff1a8e2eb1b4b5d4261db` freezes the same 4,217 Source/input Git blobs as the accepted caller packet, including all 4,210 src/include paths. Against the retained Root Source3356 manifest, 3,350 raw files agree and six differ only by CRLF versus LF; all 3,356 normalized files and actual predecessor Git blob identities agree. Both raw epochs, exact LF pins and actual memberships remain distinct. Four newly required inherited document/report blobs, earlier qualified contract/tool blobs, and Source inputs total 4,231 frozen Git members. No semantic Source drift is claimed.

The portable replay takes only the supplied ZIP bytes. Original and distinct physical-copy runs pin the helper and ZIP before execution, verify exact ZIP membership, complete CLI/raw-response captures, queried-site metadata, the quarantined projection boundary, Source/raw/LF pins and actual frozen Git membership. The guard reads its supplied ZIP once, then denies further filesystem access, imports, writes, process/network operations and dynamic-library loads. Separate actual frozen, staged and final Git receipts check retained bytes against real objects; the staged/final phases require exactly these two output paths. Exact pins are in the report and post-commit delivery receipt. This is metadata evidence only: no Ghidra repair, code change, build, test, probe, process binding, audio or game execution occurred.
