# CC12 locale initializer return-site operand

`00CC8D44` is the one-byte instruction `59` / `POP ECX`. Fresh typed metadata retains the unowned one-byte `InstructionDB`, exact and containing unit agreement, `NONE` override, empty targets and default/effective `FALL_THROUGH` to `CC8D45`. `CC8D45` remains unread. “Return site” means the preceding accepted CALL's encoded next address only.

The exact lease covers `CC8D44` and these two outputs. Fresh identity was `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, `x86:LE:32:default`, image base `00400000`. The two typed raw responses agree at epoch 98; count pairs equal 64730. Code/count/text have three exact before/after pairs, text used context 0 with one returned start and empty neighbor arrays. One unique code byte and zero data bytes were admitted; no saved Ghidra project mutation, preceding instruction refresh, decompilation, target or later-site query occurred. No function at/containing this site exists, so no-return is unavailable, not false.

For a successful execution in the decoded 32-bit mode with a normal 32-bit stack, let entry ESP=U and Q be the DWORD read from SS:U. This POP sets ECX=Q, ESP=(U+4) modulo 2^32 and next EIP=CC8D45. EAX/EDX/EBX/EBP/ESI/EDI and EFLAGS are preserved relative to entry to this instruction. It reads four stack bytes and performs no architectural memory store; those stack bytes were not captured. Entry values, reachability, faults and abnormal transfers remain unproved.

The prior PUSH and CALL snapshots conditionally put CD9410 at BF6FF5 entry ESP+4 and push return address CC8D44. They supply no post-BF6FF5 state. **Post-callee ESP, GPRs, flags and stackword contents remain unknown.** Deriving ECX=CD9410 here requires both an explicit returned ESP=S-4 hypothesis and the separate hypothesis that the DWORD at SS:(S-4) still contains CD9410. Even a plain-return or zero-argument-cleanup hypothesis cannot establish the latter. Earlier EAX=E1768C credit ends at BF6FF5 entry; POP preserving unknown incoming EAX does not restore that claim. No actual stack-cleanup policy, parameter/signature, registration, initializer completion or callback/EH meaning is established.

Epoch 92 initializer prefix, epoch 95 PUSH, previous epoch 98 CALL, previous epoch 98 metadata and this fresh epoch 98 operand capture are distinct evidence sets. Their composition is conditional, not an execution trace. The exact capture order is retained in the report; equal finite samples do not claim an atomic database transaction or continuous stability.

Frozen Source HEAD is `987f5b7ca7ab5e045c823f84a6e6094c58181919`. All 4217 `src/`, `include/`, `cmake/` and `CMakeLists.txt` paths equal the prior metadata Source snapshot; actual Git closure includes 4274 selected blob occurrences, 57 named dependencies and 11 commit/tree objects. Complete enumerated seven-pattern searches, actual object bytes, Source subtree traversal and the transitive helper graph replay inside the package. Source changes subsequently accepted on Main require a separate comparison. Historical Source3356 renderer pin is retained only as inherited evidence, with no current build credit.

Core archive: `local/cc12_locale_initializer_return_site_operand/evidence.zip`, 187586630 bytes, SHA256 `6ccb66dad98a6e08dcfa1ea8ef10aa96523c0c46c7fc501741fce35885c2839e`. Both original and distinct physical-copy guarded replay passed. The previous metadata core (163967198 bytes, SHA256 `f881b73377f983ee217f7b2dcbdc26101b0d1195e47d47a47a3eff2cef6c205f`) and previous actual-output delivery (289171 bytes, SHA256 `a566490f3e7c232079f7f432a5d2e6748f971d51807a95707e769f52d7744c03`) passed physical copied replay and recur under this guard. The nested delivery is commit `bf819aae308ef8640f2c7c28707a7bc4eeb74b94`; its exact outputs equal the corresponding current dependency blobs.

Place the following 15 physical helpers beside `evidence.zip`, then run `python guarded_replay.py evidence.zip --output result.json`. The guard matches physical helper bytes to archived Tools before loading the replay graph. Replay allows only the supplied ZIP read and result write; external reads, process/network creation and native loading are denied, with three negative self-probes and zero unexpected denials.

| Required core helper | Bytes | SHA256 |
| --- | ---: | --- |
| `replay.py` | 13268 | `c47081e1a52c18656155650973251acfa2018b0817d5389135ae95a002da687a` |
| `guarded_replay.py` | 3723 | `48254fe337a5def3c5c7564f5bacec65838aca4564c2b6b1694bb62bda09bced` |
| `prior_replay.py` | 5817 | `5787d5059905efafa5cbf041ddd604aa7c455abe1799b9a530d081669bef3f25` |
| `prefix_replay.py` | 10761 | `a1b73d8c05fb3d6ddbe0be426db625ef7572da6bc27891df7cc34b55178e2d40` |
| `prefix_delivery_replay.py` | 2465 | `a126e45579611ca05c5e4b02c42567c01ad0e8f4eaf645b59b0744747eb847f6` |
| `metadata_replay.py` | 9853 | `b31b71745833305917e8367b8b18575b539b71dd18d6346de0b065c1e02fb16b` |
| `metadata_delivery_replay.py` | 2467 | `25147f2eff8a3ba2c819b76d950eca1ce6d078c2b44314124ac32524b7cbad77` |
| `operand_replay.py` | 10585 | `1c941b6bc0eae0c2ff25ab87af765e66fbbff0dc581b6f175b1bf038a61f7f63` |
| `operand_delivery_replay.py` | 2480 | `11e83e2c8ec79cfeadf4ee19ec68e8c022ef1a81681466364ed36fe1eff43503` |
| `post_metadata_replay.py` | 12435 | `20ecd3c6cf8aeb12d774ba59ca76cda39a0c07a4cdabdada589e9fedb40a4434` |
| `post_metadata_delivery_replay.py` | 2462 | `647cb76904c1a7d4cccd49a0de2ba2d9b0212ce9863d6fe205c98dec63feff04` |
| `call_replay.py` | 12906 | `018f614495c0a6f551cd8d481047a67d66ec4ec6cac22e93b70b43c5c98e53bf` |
| `call_delivery_replay.py` | 2466 | `2252ea9d80d691df785d1211472f4fa44fc8da105aca39802fca8fbe31b4ebe0` |
| `return_metadata_replay.py` | 10363 | `02e9da53e519c568fdd960418674950a81bd294e23477d85901eac004bbac35d` |
| `return_metadata_delivery_replay.py` | 2453 | `977f708c138accaf6692198296378067f1478642401b092f3f2b73bbe72cddbd` |

Actual-output Git delivery is captured after this immutable two-file commit and physically replayed with `guarded_delivery_replay.py`, `delivery_replay.py` and `prior_replay.py`. Its external pin avoids a circular hash of this report. The machine-readable report contains helper pins, exact Native records, Source dependency inventory, complete search counts, inherited proof pins and boundaries.

No C++, CMake, build, native test, probe, ABI execution, OS/audio/game behavior execution or Ghidra write is part of this packet. A possible next packet is **metadata only at CC8D45**, subject to separate Root admission and fresh identity/ownership/unit gates; no byte, context, body, target, callback or EH expansion is admitted here.
