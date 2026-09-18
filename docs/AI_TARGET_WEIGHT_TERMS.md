# The four terms `00A0F810` stood in for

Addresses: `00923BE0`, `00923BE4`, `00923BEA`, `00923BF1`, `00923C01`, `00923C09`, `00923C16`,
`00923C27`, `00923C34`, `00923C41`, `00923C4B`, `00876260`, `0087BCC0`, `00A0F810`, `00A0F84C`,
`00A0F859`, `00A0F864`, `00A0F8CE`, `00A04560`, `00A08460`.

Packet `cc8_ai_target_weight_terms`, read-only Ghidra analysis. Every descriptive name is a
hypothesis, not a recovered symbol. Reconstruction: `include/bsp/ai_close_attack_tick.hpp`,
`src/ai_close_attack_tick.cpp`. Host: `src/game_hosts_ai.cpp`. Report:
`reports/ai_target_weight_terms.json`. Predecessor: `docs/AI_TARGET_WEIGHT.md`, which bound
`00A0F810`'s four multipliers and labelled four terms inside them as substitutions.

This packet takes those four terms to their instructions, in the order the lead set: the health
term, the inner weight, the two record factors with the target scale, and the attacker's
command-building zeroing. **Runs are blocked** while the remote-desktop session the agents run in
is disconnected, so nothing here is measured; each term says what it would move.

## Term 1 — the health term, `00923BE0`

`BSP_UnitInstance_GetHealth`, `__thiscall(unit) -> float in ST0`, `RET 0`, body
`00923BE0`-`00923C4B`, **read in full**.

```
00923be4  CMP byte [unit+5Dh],0 / JZ 00923bef   ; the torn-down byte
00923bea  FLDZ / RET                            ; torn down -> 0.0f, the class getter is skipped
00923bef  EAX = [unit]
00923bf1  EDX = [EAX+110h] / CALL EDX           ; the class's health FRACTION
00923bf9  FSTP [ESP+4] / FLD [ESP+4]
00923c01  FLDZ / FCOMIP ST0,ST1 / FSTP ST0
00923c07  JBE 00923c21                          ; 0 <= value takes the high clamp
00923c09  XORPS XMM0,XMM0                       ; value < 0 -> 0.0f
00923c16  MOVSS [unit+164h],XMM0 / RET          ; cache and return
00923c21  MOVSS XMM0,[ESP+4]
00923c27  MOVSS XMM1,[00D7A24C]                 ; 1.0f
00923c2f  COMISS XMM0,XMM1 / JBE 00923c37
00923c34  MOVAPS XMM0,XMM1                      ; value > 1 -> 1.0f
00923c41  MOVSS [unit+164h],XMM0 / RET          ; cache and return
```

Note the comparison order at `00923C01`: `FLD` pushes the value, `FLDZ` pushes zero on top, so
`FCOMIP ST0,ST1` compares **zero against the value** and `JBE` takes the high-clamp arm when the
value is non-negative. Read the other way round the two clamps swap.

**It is a fraction, not an absolute.** The class getter is `vtable[+110h]`; for the destroyer family
that slot (`00CFC3D0 + 110h` = `00CFC4E0`) holds `00876260 BSP_UnitInstance_GetHealthFraction`,
body `00876260`-`00876274`, which is `unit+370h / unit+36Ch`, current over maximum. So the clamp
into `[0, 1]` is a safety clamp on a ratio.

Both arms cache the clamped result at `unit+164h`. A store census of that offset
(`tools/store_census.py 0x164`) finds three writers in this family: `0087BD09` in
`BSP_UnitInstance_InitHealthAndParts`, which seeds it, and `00923BE0`'s own two stores. Nothing
else writes it, so `+164h` is this routine's cache and not a field others maintain.

### Correction to the ledger's earlier reading of `00923BE0`

The existing ledger evidence (packet `cc2_unit_timed_subupdates`) records the routine as "the
virtual at vtable `+110h` **floored at zero**, with the zero also written back to `+164h`". The
floor is right and the ceiling is missing: `00923C27` compares the value against `1.0f` at
`00D7A24C` and `00923C34` clamps anything above it down, and **both** arms cache, not only the
zero one. A consumer that relied on the documented shape could pass a fraction above one through,
which the native never does. The ledger record is extended rather than replaced.

### What it does to the weight

`00A0F810` subtracts this from the double `2.0` at `00D7A308`, so slot D runs over **`[1.0, 2.0]`**:
a full-health target contributes `1.0`, a destroyed or torn-down one `2.0`. That is the model's
preference for a damaged target, worth at most a factor of two.

### Correction to `docs/AI_TARGET_WEIGHT.md`

That packet left `target_term` at `0.0f`, which made slot D the constant **`2.0`** — the value a
**destroyed** target yields. It should be the full-health `1.0`. `ai_unit_health_00923be0` now
supplies it, and the host passes the torn-down byte, which it does reach through the scene node
flags (`SceneNodeFlags::torn_down`, `+5Dh`), and `fraction_available = false`, which returns the
full-health `1.0f`.

The ordering of candidates does not change, because the factor was uniform either way and
`ai_close_attack_candidate_admitted` only tests for a positive weight. What changes is the
magnitude: every candidate weight halves. Any future rule that compares a weight against a
threshold rather than against another weight would have read the old value as twice its true size.

**The torn-down arm cannot be reached from the candidate loop.** `00A13B60` drops a candidate that
fails `close_candidate_alive` before scoring it, so every candidate that reaches the weight is
live. The arm is modelled because the native has it, and the census counts any hit so that a
non-zero value would flag the assumption breaking.

### Still stood in for

The fraction itself. `unit+370h` and `unit+36Ch` are the current and maximum health, and the values
this process holds for them live on the gunnery host's per-unit row (`health`, `max_health`), which
the AI coordinator does not hold and which this packet may only read. Term 2 carries the same
problem for the inner weight and settles the route for both.

TERM2_PLACEHOLDER

## Validation

**Blocked, not measured.** The remote-desktop session the agents run in is disconnected, so the
machine has no audio endpoint for session 1 and FMOD cannot initialise; every run dies before the
window. `docs/AI_TARGET_WEIGHT.md` section 3 carries the two failure texts and the evidence that it
is the environment. The build is clean at `/W4 /WX` and both existing ctest cases pass.

When runs work again, IJN01 goes first, and this packet's own census lines are
`summary mission ai target weight health` for the torn-down count. Term 1 alone should move no
ordering and no `served`, `attackmove` or `settarget` count; it halves every weight uniformly, and
a non-zero torn-down count would mean the liveness filter above it is not doing what this section
claims.
