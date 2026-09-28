# The automatic target's candidate list (packet `cc9_autotarget_recon_candidates`)

Worker cc9-gunnery4, rank 1 of GUNNERY_OPEN_ITEMS section 16. Ghidra was read-only.
Descriptive names are hypotheses.

## 1. The image

`009F5D30` `BSP_AutoTarget_ScanPartyList`, `__fastcall(searchState)`:

```
009f5d33: mov eax,[ebx+14h] ; mov ecx,[eax+54h]    ; the owner's party (unit+54h)
009f5d3a: call 0x8053c0                            ; BSP_Recon_EnsureSlot(party)
009f5d42: mov [ebx+18h],0 ; movss [ebx+1Ch],0      ; no best yet, score 0
009f5d4e: mov edi,[eax+0DE8h] ; test edi,edi ; je  ; the slot's enemy chain; empty -> no candidate
009f5d60: mov ecx,[edi+8] ; mov esi,[ecx+4]        ; the record's entity
009f5d68: call [edx+140h] ; ... cmp byte [eax+1D4h],0 ; jne skip
009f5d8e: call 0x9f5b70                            ; score it (BOT_FIRE_TARGET 2)
009f5d93: mov edi,[edi+4] ; jne 0x9f5d60           ; next
```

**What the chain holds.** `[slot+0DE8h]` is the head of the slot's **enemy** triple
(RECON_TEAM_LISTS 1, HUD_CAMERA_TEAM_LISTS 2). The recon rebuild `008073C0` fills it every 3.0 s
(`008079B0`), under these rules:
- identified contacts (level 2) only; blips (level 1) go to the unknown triple;
- **enemies only**, with neutrals in a separate triple;
- class buckets `00h..60h` in order, and scan order within a bucket.

## 2. The host

`TargetBinding::scan_party_list` (`src/game_hosts_ship_ai.cpp`) handed the scan every live unit of
another party, in unit order. It recorded `008053C0` as a stand-in. That list differs from the
image's in three ways:
- it includes undetected units;
- it includes blips;
- it includes neutral-party units.

The gunnery host already builds the image's triples (`kReconTeamListsBound`) and exposes them
through `GameGunneryHost::recon_triple_units(side, triple, out)`.

## 3. The binding (committed OFF)

`kAutoTargetReconCandidatesBound`:
- **The source:** the scan walks `recon_triple_units(owner party, 1)`, the enemy triple, in its own
  order. Each entry still takes the row's live test and the existing per-candidate gates.
- **Before the first rebuild,** when the triple does not exist yet, the list is empty: `009F5D54`'s
  `JE` leaves no candidate.
- **The census** is `summary mission ship ai autotarget recon candidates bound= scans=
  mean_candidates= unbuilt=`.

## 4. OFF, and the triples' sizes

OFF is this tree's build (`local\AR_OFF_<m>.log`). On USN04 it is gameplay-identical to reference
h's `rb8_usn04` (44 death rows).

| mission | OFF deaths / hit records / shots | recon triples (mean over builds): enemy / unknown / neutral |
| --- | --- | --- |
| USN02 9200/9000 | 10 / 4226 / 2621, failed 29.75 s | 19.4 / 1.2 / 0.0 |
| USN04 4700/4500 | 44 / 749 / 9310 | 38.7 / 3.3 / 0.0 |
| USN13 3200/3000 | 20 / 396 / 3903 | 135.2 / 0.0 / 336.7 |
| JM08 3200/3000 | 9 / 310 / 1689 | 179.6 / 0.0 / 530.2 |

## 5. Predictions, written before the ON runs

| row | prediction |
| --- | --- |
| census | `scans` above 0 on all four; `unbuilt` above 0 (the first steps before the first rebuild) |
| USN02 | pair_diff exit 3. Enemies are almost all identified, so the change is at the start: no AutoTarget pick before the first rebuild, and the one blip drops. The opening torpedo launches are **the same time or later** than 1.45 s. Houston's fate may move either way; the failure stays torpedo-driven |
| USN04 | exit 3 with small moves (3.3 blips leave the list); deaths 44 +- 4 |
| USN13 and JM08 | exit 3: the neutral-party units leave the ships' candidate lists; ship-gun shots at non-enemies fall, and total shots move down |

## 6. The pairs, and the flip

- **OFF** is `local\AR_OFF_<m>.log`, this tree at `de4b1ca09`.
- **ON** is `local\AR_ON_<m>.log`, from `pair_export --commit de4b1ca09 --flip
  kAutoTargetReconCandidatesBound=true` (`local\ar_on`).

Streams and the death table were on.

| mission | pair_diff | census (ON) |
| --- | --- | --- |
| USN02 | **exit 1, gameplay identical** | scans 10689, mean candidates 10.03, unbuilt 0 |
| USN04 | **exit 1** | 4746, 20.32, 0 |
| USN13 | **exit 1** | 43186, 28.24, 0 |
| JM08 | **exit 1** | 57531, 28.62, 0 |

| prediction | verdict |
| --- | --- |
| scans above 0 | held |
| unbuilt above 0 | **failed**: the gunnery host's first rebuild comes before the first AutoTarget scan |
| USN02, USN04, USN13, JM08 move (exit 3) | **failed on all four**: identical |

**What this means.**
- The recon list is shorter, a mean of 10 to 29 candidates per scan, but its winner is the same
  on every scan of these four missions.
- The units it drops (undetected, blips and neutral party) never win `009F5B70`'s score. **Not
  traced per scan.** The likely reasons are that a neutral matches no priority tier and that a
  far or undetected unit loses on distance.
- **So rank 1's reach was over-estimated.** It has 148923 calls, but it decides no pick on these
  missions.

**Decision: `kAutoTargetReconCandidatesBound` is ON.** The list is the image's (`009F5D4E`), the
missions are identical, and the switch removes a stand-in. GUNNERY_OPEN_ITEMS section 16's
rank 2, the commanded-target adoption, is next.
