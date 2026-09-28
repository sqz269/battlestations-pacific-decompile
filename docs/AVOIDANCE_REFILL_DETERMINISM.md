# Why `ship avoidance search: refills` differs between identical runs

Packet `cc9_avoidance_refill_determinism`, worker cc9-ships2, on main `a576d52f0`. Ghidra was
read-only.

## 1. The measurement

Two identical USN04 4700/4500 runs of this tree's `build\` (streams and the death table on,
lockstep 0.05, idle) are `local\ar_a_usn04.log` and `local\ar_b_usn04.log`. `pair_diff` exits 0,
identical apart from noise.

| run | queries | refills | clears |
| --- | --- | --- | --- |
| a | 80960 | 332 | 12 |
| b | 80960 | 326 | 12 |

Two more runs with the new diagnostic `BSP_AVOID_REFILL_TRACE=1` (`local\ar_ta_usn04.log` and
`local\ar_tb_usn04.log`) give refills of 339 and 350. Each prints every query with its inputs and
the cache bounds before and after, as hex floats. The 80960 query lines have **identical inputs**
(position, half extent, layer) in both runs. They differ from the **first query of each ship**,
in the cache's `before` bounds:

```
q=1 unit=1 ... layer=0 refilled=1 before=(0x1.ba44de0000000p-63 0x1.64407a0000000p-31 0x1.141a580000000p-101 0x1.1212140000000p+55 -1)
q=3 unit=3 ... layer=0 refilled=1 before=(0x1.d844b60000000p+67 0x1.ecc8dc0000000p+75 0x1.c6d2d00000000p+89 0x1.ba44ca0000000p-63 -1)
```

Those are heap leftovers. The first refill takes the cache's old width `max - min` when it exceeds
the query's expanded half extent (`009D70F9`..`009D7101`, the `JBE` in
`ship_ai_avoid_query_refresh_009d7050`). So a garbage width can make one ship's cache enormous
(`after` min_x = -0x1.c6d2c8p+88 on unit 3). Every later query of that ship then falls inside it
and never refills. How many ships get such a cache depends on what the heap held, and that decides
the total.

## 2. The image does the same

The searcher records sit in the ship AI block allocated through `00BF681B` (the CRT allocator,
not zeroed). The block constructor initialises only the enable byte, the layer key and the list:

```
009E4401  MOV [ESI+0A3Ch],EBX      ; list head = 0
009E4407  MOV [ESI+0A40h],EBX      ; list layer = 0
009E4411  MOV [ESI+0A38h],EAX      ; layer key = -1
009E4417  MOV byte [ESI+0A24h],1   ; enabled
          ; +0A28h..+0A34h, the four bounds, are never written
009E441E  MOV [ESI+0A5Ch],EBX      ; the second record, likewise
```

The first `009D7050` call then compares the layer key (-1 against the ship's) and falls into the
refill, which reads those four floats as the old extent. **The host's variation is the image's
own.**
- `src/ship_ai_search_storage.cpp` copies the native contract on purpose: raw allocation, no zero
  fill, the bounds snapshotted by byte copy.
- Two image runs differ the same way whenever the heap differs.

## 3. Consequences, and the verdict

- **Gameplay does not depend on it here.** A larger cache only makes the selected segment list a
  superset of the layer's segments. The searches (`004158E0` hit, `00415970` arc, `00415D70`
  clearance) test geometry, so they give the same answers.
- Both run pairs are gameplay-identical (exit 0 and exit 1 respectively), and 80960 queries match
  line for line.

**No binding.** Zeroing the bounds would make the host deterministic, but it would stop being the
image. The `pair_diff` mask row on this counter is correct and should stay. Nothing needs routing
to tooling.

The diagnostic stays. `BSP_AVOID_REFILL_TRACE` prints nothing when unset. It lives in
`AvoidSearchBinding::avoid_zone_query_refresh_009d7050` (`src/game_hosts_ship_ai.cpp`).
