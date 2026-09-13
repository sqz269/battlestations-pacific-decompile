# Orchestrator 6 reconstruction batch V

Addresses: 0042B120, 006D1E80, 00951FB0, 00955448, 009273A0, 00CA6E60.

V connects the existing unit byte owners to lifecycle providers, reconstructs scene-handle operations, and supplies the complete normal pending-drain sequence. New C++ interfaces remain distinct from the original ABI.

| Change | Verified scope |
| --- | --- |
| [Actual unit byte view](GAME_UNIT_SCENE_LIFECYCLE_VIEW.md) | Same five owners, guarded borrowing and current activity snapshots. Two actual-host cases execute registered observer callbacks and affect the existing world/motion gates. |
| [Scene-handle operations](NATIVE_UNIT_SCENE_HANDLE.md) | Complete getter and clear/tail; producer store projection only. Three original-byte/source cases and captured table checks. |
| [Pending drain](NATIVE_PENDING_ENTITY_DRAIN.md) | Complete 562-byte normal sequence; empty and two-pass requeue fixtures match callback, flag, scratch and real-free traces. |
| [Shared RET4 target](OBSERVER_RET4_PROVIDER.md) | Exact three-byte 0042B120 body and 20 actual callback cells; literal selected provider used by the unit fixture. |

The unit fixture uses an explicit null getter input and throws at the unresolved wreck/killed tail boundaries. Its assertions cover the preceding real flag and observer effects. Source inspection establishes alias lookup withdrawal; the fixture disposes of borrowed views before destruction and checks survivor detachment. A retained C++ reference is not automatically revoked.

The drain copies both queues before clearing either, preserves native sentinel and callback ordering, and consumes callback requeues on a subsequent pass. Its required copy and virtual providers remain explicit. Original native exception dispatch was disabled in the byte clone and is unproved. Two free call sites remain outside Ghidra's stored function body; their full bytes and mechanical verifier failures are retained. The existing function was not recreated.

MSVC Win32 Release and both existing CTests passed at `ffc6ba2abaef53084954b9f920fe9a6b26a04821`. Executable SHA256: `0fcfac99990843e3d6dfbc1e6d074cc72e19aace15f0f890c0e9f04de2e01664`. The 120-frame USN01 run produced 18,557 finite trajectory rows and 241 unchanged Airfield2 samples, with 2,400 avoidance queries, 1,080 cruise reads, 10,080 generic ticks and 420 valid world nodes. All 77 unit observer prefixes were torn down while the owner was live, before manager drain.

Both pending queues remained empty through 240 passes. This run checks application compatibility; it does not prove that the mission exercises the new nonempty drain or lifecycle tails. Actual scene pointer ownership, copy/remove providers, renderer bindings, native exceptions/concurrency and full visual/gameplay parity remain incomplete.

The archive retains 557 verified worker artifacts and 60 root proof artifacts. The missing getter 006D1E80 and EH selector 00CA6E60 were formally defined from verified bytes and saved. The incorrect adjustor name at 00951FB0 was corrected with prior annotations preserved; its body leaves ECX unchanged and clears +4A4 before delegation. The formal definition tool decoded the verified ranges; the earlier commit description's wording about no disassembly was imprecise. No global analysis or function recreation was used.

## Follow-up packets

W recovers the killed-player-unit tail 00779AF0, the effect-handle fragment 00824F39..00824FE4 within the ship wreck handler, and the complete base-killed sequence 00928C80 using explicit required providers and borrowed actual owners.
