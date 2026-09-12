# Explosion radial damage: 0084BAD0, the gather, the sphere query and the shapes

Addresses: 0084BAD0, 00904470, 0098C630, 0098C510, 0098AAE0, 0098B9C0, 0070F090, 0042E630,
00428800, 004E6540, 004E6560, 004E6470, 004F13A0, 00929FF0, 0087FEC0, 0087FF80, 00929B80,
0053F2D0, 006FCD20, 00819A20, 0084BC60, 0070C210, 00CE89DC, 00CE89E8, 00CEA050, 00CEA05C,
00CFD7B8, 00D194A8, 00D194B8

This packet takes the "analyzed, not reconstructed" half of `docs/HIT_NARROWPHASE.md`: the blast
path's argument contract, what the gather actually collects, the geometry the sphere side runs, and
which of the eight `0084BAD0` call sites is which explosion. The consumer side (the falloff divide
at `004705C0`, the hull formula at `00470510`, the drain at `009239A0`) stays in
`docs/UNIT_HIT_PATH.md` and `docs/PROJECTILE_IMPACT.md`. The reconstruction is
`include/bsp/blast_damage.hpp` and `src/blast_damage.cpp`; the machine-readable evidence is
`reports/blast_damage.json`.

## The argument contract of 0084BAD0

`__fastcall`, `RET 10h` at `0084BC5B`, so four stack arguments on top of the two register ones.

| slot | argument | evidence |
| --- | --- | --- |
| ECX | `centre`, a `float3` | `EDI = ECX` at `0084BAFB`; `FLD [EDI]`, `[EDI+4]`, `[EDI+8]` at `0084BBB6`..`0084BBC9` |
| EDX | `&radius`, a pointer to one float | `ESI = EDX` at `0084BAF7`, `FLD [ESI]` at `0084BAF9` and again at `0084BB25` |
| `+4h` | `&damage`, a pointer to one float | `MOV EAX,[ESP+10h]` at `0084BADE` is entry+4 after the three SEH pushes; `FLD [EAX]` |
| `+8h` | `ignoreFalloff`, **the low byte only** | `MOV AL,byte ptr [ESP+40h]` at `0084BBC2`, stored to `record+2Ch` |
| `+Ch` | `sourceEntity` | `MOV ECX,[ESP+44h]` at `0084BBB8`, compared with `[ESI]` at `0084BBCF` |
| `+10h` | `shot` | `MOV EDX,[ESP+48h]` at `0084BBA7`, pushed into `00470350` |

Neither pointer is written through: the radius and the damage are read once each.

Order of effects:

| order | site | step |
| --- | --- | --- |
| 1 | `0084BB00` | `00428800(radius, damage)`, both as floats by value. **Before** the world-mode test, so a suppressed blast still makes this call |
| 2 | `0084BB0A` | return when `[00E188A8+1FE4h] == 2` |
| 3 | `0084BB4A` | `00904470(&vector, centre, radius, ignoreFalloff, shot, sourceEntity)` |
| 4 | `0084BBA1`..`0084BBD4` | per record: `+28h = damage`, `00470350(shot)`, `+8h..+10h = centre`, `+2Ch = ignoreFalloff` |
| 5 | `0084BBF9` | `00926E80(record, (0,1,0))` unless `record+0h == sourceEntity` |
| 6 | `0084BC22`..`0084BC43` | destroy every record with `004704B0`, then free the buffer with `00BF65AC` |

The queued direction is a constant: `XORPS XMM0,XMM0` and `MOVSS XMM1,[00D7A24C]` at `0084BBD6`,
and `00D7A24C` reads `00 00 80 3F`, so every blast hit is queued with the impact direction
`(0, 1, 0)` regardless of where the victim is relative to the burst.

The `MOV ECX,[EAX+19CCh]` at `0084BB3B` looks like a `this` for the gather and is not one:
`00904470`'s first instruction overwrites ECX with its sixth stack argument and the entry value is
never read. The gather takes the spatial index from `0042E630` instead.

Two fields the apply pass overwrites matter to the consumer. `+8h..+10h` was the shape's hit
position and becomes the burst centre, and `+2Ch` is written twice, once by the gather and once
here with the same value. `+28h` is the only damage field a blast hit carries, which is why
`004705C0` (the part formula) is the blast formula and `00470510` (the hull formula, fed from
`+14h`) is the direct-hit one.

## The gather, 00904470

`__stdcall`, six stack arguments, `RET 18h` at `00904550`:
`(outVector, centre, radius by value, ignoreFalloff byte, shot, sourceEntity)`.

1. `00904476`: a null `sourceEntity` leaves the exclude null; otherwise
   `exclude = sourceEntity->vtable[B0h]()`, which returns a **collision node**, not an entity:
   `0098C510` compares it against the node pointer at `0098C535`.
2. `0090449B`: `0042E630` with the four `0098C630` arguments already pushed, so it takes none and
   returns the spatial index in EAX, which becomes ECX at `009044A0`. It is a lazy singleton
   (`[00F8A0D8]` tested at `0042E64D`, built under an SEH frame).
3. `009044A2`: `0098C630(index; centre, radius, exclude, outVector)`.
4. `009044B4`..`00904547`: per gathered record, `54h` apart: `00470350(shot)`, `+18h..+20h = centre`,
   `+24h = radius`, `+2Ch = ignoreFalloff`.

So the gather collects **collision nodes' owner entities**: one record per node that reports an
overlap, with the record's `+0h` set by `0098C510` from `node+4Ch`. It does not collect parts, and
nothing on this path allocates the part-hit array at `+3Ch`.

## The sphere query, 0098C630 and 0098C510

`0098C630` is `__thiscall(index; centre, radius, excludeNode, outVector)`, `RET 10h`. It is the
sphere sibling of the segment query `0098ADD0` over the same grid:

| step | site | detail |
| --- | --- | --- |
| bounds | `0098C639`..`0098C6A8` | `centre - radius` into `[ESP+28h..30h]`, `centre + radius` into `[ESP+34h..3Ch]` |
| cells | `0098C6AC`, `0098C6B9` | `0098AD60` on each corner gives an inclusive cell pair |
| clamp | `0098C6BE`..`0098C6F0` | the low pair is raised to `0` and the high pair lowered to `95h`, each one-sided |
| walk | `0098C6FC`..`0098C7A9` | `index+84h`, row stride `258h` bytes (`96h` dwords) |
| stamp | `0098C653`, `0098C753`, `0098C762` | `[00E188A8+648h]` against `node+4h`: a node in several cells is tested once per query |
| link | `0098C750`, `0098C77F` | the node is `[link+8h]`, the next link `[link+4h]` |
| result | `0098C77B` | AL is the OR of the `0098C510` results |

`0098C510` is `__thiscall(index; centre, radius, excludeNode, node, outVector)`, `RET 14h`:

1. `node == excludeNode` returns false at `0098C535`, which is how a burst skips its own source.
2. a stack record is reset by `00470470` and given `node[+4Ch]` through `00470370`.
3. `node->vtable[0Ch](centre, radius, excludeNode, &record)` at `0098C595`. **The third argument is
   the exclude node**, not the output vector: it is EBX, the same value the routine compared itself
   against. `docs/HIT_NARROWPHASE.md` records it as the vector; that is the correction this packet
   makes. The vector is the fifth stack argument and is only used at `0098C59B` as the `this` of the
   append.
4. on a hit, `0098C460(&record)` appends. The record is then destroyed by `004704B0` at `0098C606`,
   and that destructor frees `+3Ch`, so the append must deep-copy or take ownership; `0098C460` is
   `contract: unread`.
5. the children at `[node+FCh]` / `[node+100h]` are recursed over **whether or not this node hit**.

## The node and shape interfaces

The collision node's base vtable is `00CE89E8`: `004E6540` writes it at `004E6543`, calls the base
constructor `004E6480` and stores its one argument at `node+4Ch`, the owner entity. Four slots:

| slot | base implementation | role |
| --- | --- | --- |
| `+0h` | `004E6560`, a one-byte `RET` | unused in the base |
| `+4h` | `004E6590` | the deleting destructor |
| `+8h` | `0098B9C0` | the **segment** entry: an AABB reject through `0085CAD0` then `0098AC20` |
| `+Ch` | `0098AAE0` | the **sphere** entry, the blast path |

`0098AAE0` is `__thiscall(node; centre, radius, excludeNode, record)`, `RET 10h`:

1. it builds the sphere's AABB and rejects it against `node+13Ch..+144h` (min) and
   `node+148h..+150h` (max) with six `FCOMIP` tests at `0098AB4B`, `0098AB62`, `0098AB72`,
   `0098AB82`, `0098AB92` and `0098ABA2`. Each rejects on a strict separation, so touching bounds
   still overlap.
2. it then walks the inline shape array at `node+D0h`, count `node+F8h`, and calls
   `shape->vtable[4](centre, radius, &record)` at `0098ABE3` per shape, returning the OR.

The shape interface is three slots wide. Its base vtable is `00CE89DC` (`004F1206` writes it into
the subobject at `entity+344h`, which is exactly the `this-344h` `docs/HIT_NARROWPHASE.md` records
for `0087FF80`), and the sphere slot in the base is `__purecall` (`00BF698E`), so every concrete
shape must define one.

| shape vtable | slot 0, segment trace | slot 4, sphere test | slot 8 |
| --- | --- | --- | --- |
| `00CEA050` (`entity+344h`, the subobject shape) | `0087FF80` | `004F13A0` = `XOR AL,AL; RET 0Ch` | `004E6470` = `RET 4` |
| `00D194B8` (`entity+1A4h`, the transformed box) | `00929B80` | `00929FF0` = `XOR AL,AL; RET 0Ch` | `004E6470` |

The `RET 0Ch` of both stubs is three stack arguments, matching the three pushes `0098AAE0` makes,
which is the corroboration that slot 4 is the sphere test and slot 0 the segment trace.

### Shape trace geometry (the segment slot, from docs/HIT_NARROWPHASE.md plus the vtable owners)

| shape | body | geometry | record writes | sphere sibling |
| --- | --- | --- | --- | --- |
| `0087FEC0` (extruded) | `0087FEC0`-`0087FF7x` | `[this+3D0h]->vtable[3Ch](from, scaledEnd)` | `+8h..+10h` hit point, `+0h` entity, `+30h = 0Ah`, `+34h = -1` | not resolved: its only data reference `00CEA174` sits inside a large vtable whose neighbouring slots take zero and one argument, so it is not the three-slot shape interface |
| `0087FF80` (subobject) | `0087FF80`-`0087FFFx` | `[this+8Ch]->vtable[3Ch](from)`, entity is `this-344h` | the same four | `004F13A0`, a false stub |
| `00929B80` (transformed box) | `00929B80`-`00929C7x` | endpoints transformed by `this+B0h`, `0085CDB0` against a physics-library box, hit point back through `this+70h`; entity is `this-1A4h` | the same four | `00929FF0`, a false stub |

## Who could write the hull segment index, `record+34h`

Nothing found on the blast path does, and the reason is structural rather than incidental.

* Every gathered record starts at `00470470`, which sets `+30h` and `+34h` to `-1`.
* The blast path's only writer opportunity is `node->vtable[0Ch]` and, under it,
  `shape->vtable[4]`. Taking the data references to the two default node slots and differencing
  them (`0098B9C0` appears in `00CE89F0`, `00CEA064`, `00CF8B20`, `00CFD7C0`, `00D194B0`;
  `0098AAE0` in `00CE89F4`, `00CEA068`, `00CF8B24`, `00D194B4`) leaves exactly one class that
  overrides the sphere slot: `00CFD7B8`, the unit part's node vtable, built by
  `BSP_UnitPartInstance_Construct` at `007135FA`.
* That override, `0070F090`, re-pushes its four arguments, calls `0098AAE0` at `0070F0A7`,
  normalises the result with `SETNE` and returns. It writes nothing.
* Both concrete shape classes whose vtables are reachable from the known segment traces stub the
  sphere slot to false.

So on the evidence read here the blast path cannot produce a hull segment index or a part-hit
array, and `docs/UNIT_HIT_PATH.md`'s part rows stay consumer-derived.

**Labelled gap.** The shape classes were enumerated from the three known segment traces, and both
resolvable ones return false, which cannot be the whole story: a burst that gathers nothing would
damage nothing. A shape class whose slot 4 is a real test and whose slot 0 does not write the
`+30h = 0Ah` kind code would be invisible to both this enumeration and the byte search that found
the three traces. The way to close it is the producer of `node+D0h`: until the code that installs
shapes into a node is read, the shape set is partial. `BSP_UnitPartInstance_Construct` does not
install one itself.

## The callers of 0084BAD0

Eight call sites in six containers. `0085F80C` has no containing Ghidra function.

| site | container | explosion | centre | radius source | damage source | ignore | source entity | shot |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `0084BEE3` | `0084BC60` `BSP_Projectile_OnImpact` | shell or bomb impact | hit point minus the direction scaled by `[00D7A270]` | `classDesc+70h` | a local, randomised from `classDesc+B4h`/`+B8h` | `classDesc[+8h] == 10h` | `buffer+4h`, the owner | `buffer+20h` |
| `0070C280` | `0070C210` `BSP_FlakProjectile_Detonate` | flak burst | `proj+FCh` | `classDesc+70h` | `00BD2F10(classDesc+B4h, classDesc+B8h)` | `1` | `[proj+238h]` | the projectile |
| `006FD032` | `006FCD20` | a projectile detonation with a pose refresh and a kill: bomb, torpedo or depth charge | `proj+FCh` | `[proj+314h]+70h` | `00BD2F10(...)` at `006FCFF9` | `0` | `[proj+3D8h]` | the projectile |
| `0085F80C` | none (raw listing `0085F7C0`-`0085F80F`) | the same shape as `006FD032` | `ebx+FCh` | `[ebx+314h]+70h` | `00BD2F10(classDesc+B4h, classDesc+B8h)` | `0` | `[ebx+3D8h]` | `ebx` |
| `00819C14` | `00819A20`, called by `BSP_UnitInstance_HandleMessage` | the unit's own death explosion | `unit+FCh` | `[unit+538h]+518h` | `[unit+538h]+514h` | `0` | `0` | `0` |
| `0053F5FA`, `0053FB45`, `0053FE37` | `0053F2D0`, called by `00540390` | a staged, table-driven burst (the container also drives point effects and traces its own segments) | an object field or a local `float3` | `element+14h` of a `30h`-byte table | a parallel `10h`-stride cursor | `0` | `0` | `0` |

Two consequences of the last three rows: with `sourceEntity == 0` no record is skipped and every
gathered node is queued, and with `shot == 0` the record's `+4h` stays null, so the consumer's
weapon scale falls back to `1.0f` and `+14h` is never set.

The radius field `classDesc+70h` and the damage pair `classDesc+B4h`/`+B8h` are the projectile
class descriptor's; `docs/SHIP_CLASS_FIELDS.md` and `docs/PLANE_CLASS_FIELDS.md` describe the
vehicle descriptors, which is a different record, and the unit death explosion's pair at
`[unit+538h]+514h`/`+518h` belongs to that one. Neither pair is named in those documents yet, so
the mapping from a class file field to `+514h`/`+518h` is `contract: unread`.

## Host table

One row per native call site the reconstruction turns into a virtual method. The full list with
the argument detail is in `reports/blast_damage.json`.

| site | callee | method | this / args | ret | gate |
| --- | --- | --- | --- | --- | --- |
| `0084BB00` | `00428800` | `call_00428800` | none; radius and damage as floats | — | always, before the mode test |
| `0084BB4A` | `00904470` | `gather_hit_records` | none used; six stack args | — | mode `!= 2` |
| `0084BBB1` | `00470350` | `record_set_shot` | record; shot | — | per record |
| `0084BBF9` | `00926E80` | `queue_hit` | record; `(0,1,0)` | — | `record+0h != sourceEntity` |
| `0084BC32` | `004704B0` | `destroy_record` | record | — | buffer non-null |
| `0084BC43` | `00BF65AC` | `free_gathered` | none; the buffer | — | buffer non-null |
| `0090449B` | `0042E630` | `spatial_index` | none | the index | always |
| `009044A2` | `0098C630` | `query_sphere` | the index; centre, radius, exclude, vector | bool | always |
| `009044F4` | `00470350` | `record_set_shot` | record; shot | — | per record |
| `0098C776` | `0098C510` | `node_overlap` | the index; centre, radius, exclude, node, vector | bool | node not stamped |
| `0098C55D` | `00470470` | `record_reset` | the stack record | — | `node != exclude` |
| `0098C570` | `00470370` | `record_set_entity` | the stack record; `node+4Ch` | — | `node != exclude` |
| `0098C5A7` | `0098C460` | `append_record` | the vector; `&record` | — | the node hit |
| `0098C5EA` | `0098C510` | `recurse_child` | the index; the child | bool | per child |
| `0098C606` | `004704B0` | `destroy_record` | the stack record | — | `node != exclude` |
| `0098C595` | `node->vtable[0Ch]` | `node_overlaps_sphere` | node; centre, radius, exclude, `&record` | bool | always |
| `0098ABE3` | `shape->vtable[4]` | `shape_overlaps_sphere` | shape; centre, radius, `&record` | bool | per shape |
| `0070F0A7` | `0098AAE0` | forwarder, no Ghidra function at `0070F090` | node; the same four | bool | always |

`00428800` keeps its address as its method name: read only to `00428890`, it squares the radius and
sweeps a container at `[00E188A8+21D0h]+28h` comparing that square against a value from `0085BF90`,
which is enough to say it is a radius-scoped notification and not enough for a verb.

## Coverage

| routine | coverage |
| --- | --- |
| `0084BAD0`, `00904470`, `0098C630`, `0098C510`, `0098AAE0` | complete |
| `0070F090`, `004F13A0`, `00929FF0`, `004E6470`, `004E6560` | complete; read from the raw listing, no Ghidra function |
| `0042E630` | partial: the singleton test and the return only, `0042E645`-`0042E657` |
| `00428800` | partial: `00428800`-`00428890`, the rest unread |
| `0098C460`, `00926E80`, `node->vtable[0Ch]` overrides beyond `0070F090` | `contract: unread` |
| the five caller containers | call shape only; `0053F2D0` and `006FCD20` are otherwise unread |
