# C07AC1 first direct-call entry: primary review

Worker commit e1b35c79faadcf6a35e831d2a084dfbe126052da was independently reviewed and merged. Its finite eight-input Git closure, 20 actual object payloads, two committed outputs and all 403 archive members agree with the actual worker tree. All 139 copied Root gate artifacts agree, including the 136 original metadata preimages.

The metadata gate admitted only the exact two-byte entry at 00C07AC1. Three complete typed raw aliases agree on InstructionDB length two, saved nonthunk/no-return-false owner, NONE override and fallthrough 00C07AC3. The saved owner contains 48 bytes in ranges 00C07AC1..00C07AE4 and 00C07AEC..00C07AF7. The seven-byte intervening gap and the remaining 46 owner bytes were not read or classified. Ten successful Native commands and 20 passive phase-two HTTP responses preserve project/program identity, epoch 98 and function count 64,730.

Root independently decoded 6A08 as PUSH of sign-extended immediate 8 and E867040000 at 00C07655 as CALL with displacement 1127, encoded return 00C0765A and numeric target 00C07AC1. Root's detailed Capstone 5.0.7 results agree with its own opcode, field, width and target checks.

At a reached, fault-free entry with actual 32-bit SS stack-address size, current ESP T becomes T-4 and SS[T-4] receives 8. A consecutive CALL at current ESP S followed by this PUSH stores the encoded return at SS[S-4] and 8 at SS[S-8]. Live SS attributes and memory stability were not authenticated. Any eventual callee return leaves all returned registers, EBP, ESP, flags and memory unconstrained; no caller preservation, argument meaning, library identity, EH behavior or Original ABI follows from this entry.

The physically reviewed pure helper replayed the copied ZIP under Root's stricter audit hook: exactly one ZIP read, one helper execution, one exclusive receipt write and no denied or other allowed audit events. Archive-contained code was not executed. Worker nonexecuted draft corrections were retained and were not execution failures. Root's metadata gate, actual review, strict replay and independent decode each passed without an execution failure; worker Native, decoder and replay executions also had no failures.

The accompanying report pins the physical helpers, copied proofs, Root producers and receipts. This is an exact entry and saved metadata review. Source, build, new tests/probes, Original/game/OS audio/runtime, GPR writes and whole-owner reconstruction were outside this packet.
