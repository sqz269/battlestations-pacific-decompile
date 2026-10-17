# CC13 water 1: reconstruction pass over segments 92-94 (cc13-water1)

Packet `cc13-water1`, Claude worker in the cc13 lineage, 2026-10-12. It covers 00B999F0-00BD2E60:
ocean, coast, water tracer, shore-wave texture source, the archives under them, cloud, waterdrops,
and thunder and lightning. This is presentation code. Nothing here is wired into a host and no game
was run. A tree named `agent/cc13-water1` existed from an earlier packet of the same name (2026-10-11,
docs/CC13_WATER_RENDER.md); that branch was fully merged into main, and this packet reused the tree.

Status words follow AGENTS.md:
- **Reconstructed, build-tested** means C++ exists in `src/water_weather_cc13.cpp` /
  `include/bsp/water_weather_cc13.hpp` (target `bsp_core`, one appended line in `cmake/startup.cmake`)
  and passes the strict Win32 build.
- Nothing is fixture-tested, ABI-compatible or game-validated.

Names are hypotheses, not recovered symbols. Each reconstructed function's ledger record
(`config/reconstruction/00b9..00bd*.jsonl`, document `docs/CC13_WATER_1.md`) carries its native ABI
and the evidence that was checked.

## Method

- **Input.** I queried `local/bsp_index.sqlite` (`local/w1_list.py`) for functions in the range that
  have a ledger name but no reconstruction record and no tag: 403 of 927. `local/w1_grep.py` then
  listed which of them `src/` or `include/` already mention: 72 are mentioned.
- **Drafting.** Four read-only helper agents worked on disjoint sub-ranges under one brief
  (`local/w1_helper_brief.txt`). Each wrote a part `.hpp`/`.cpp` and a records file in `local/`.
  - Each helper checked its part with a standalone MSVC Win32 C++17 /W4 compile (`local/w1_cc.ps1`).
  - The helpers made no ledger, Ghidra, git or source-tree writes.
  - The sub-ranges were a 00B999F0-00BA2FFF, b 00BA3000-00BBBFFF, c 00BBC000-00BC2FFF and d
    00BC3000-00BD2E60.
- **Merging.** I concatenated the parts into one header and one source (`local/w1_merge.py`), one
  section per part. Every function name ends in its address, and every type carries an area prefix,
  so nothing collides.
- **Reading code.** Pseudocode was always checked against Capstone over the disk bytes
  (`bsp.py disasm-raw`). DC-form x87 was read from the bytes, never from Ghidra's listing. Every
  numeric constant was read from the PE, and its address is in a comment. Calls outside the model
  became inputs or methods on small host interfaces named after the native callee.
- **x87.** The model is 53-bit precision. Every float store is a cast, and intermediates are double.
  NaN paths are noted per function.
- **Ledger.** `local/w1_records.py` validates every record and applies it with `ledger add-function`:
  - every name exists in the cited source;
  - no record appears twice;
  - instantiation suffixes move from the name into the evidence;
  - missing default fields are filled in.

## Counts

| | count |
|---|---|
| todo at start (named, no record, no tag) | 403 |
| reconstructed (new records, src/water_weather_cc13.cpp) | 201 addresses, 202 records (some partial) |
| already reconstructed elsewhere, record added | 25 (16 whole functions, 9 optics setters recorded as fragments of `apply_authored_optics_0078c9b0`) |
| skipped: container plumbing | 105 |
| skipped: constructor / destructor / factory / runtime plumbing | 56 |
| skipped: D3D or resource builders (and two teardowns in part a) | 16 |
| left without a record | 177 |

The 25 existing reconstructions are in `src/water_render.cpp`, `src/ocean_wave_field.cpp`,
`src/ocean_coverage_regions.cpp`, `src/storage_pool.cpp` (the second SizedStoragePool instantiation at
00BD1000..00BD1620), `src/ng2_cc13.cpp` (00BB8580) and `src/band_0090_cc13.cpp` (00BB9F40, 00BBA030).
00BA0C00 has no standalone function: its logic is inside `ocean_coverage_add_landscape_00ba1370`.

## What was reconstructed

- **Ocean (part a).**
  - Base vtable stubs and the fixed 512x512 reflection target (00B9A3B0).
  - Optics and foam setters (00B9DB50..00B9E8B0).
  - The water-ring list: removal and the triangle mesh (00B9DED0, 00B9E4A0, 00B9CB80).
  - Projective matrices and foam constants (00B9AB60).
  - Cell-grid resize and assign, floating-object removal, the horizontal displacement sample
    (00BA2B50).
  - Fragments of 00BA0090 (depth-sample offsets, wave scale, the +24h copy and the +274h/+278h
    swap), 00B9F0A0 (the reflection camera) and 00B9FA60 (the "terrain" name test).
- **Ocean particles, generators, tracer, coast, archives (part b).**
  - Ocean particle update and height-texture packing.
  - Projected-grid UVs and strip indices, the six-tap blur weights, viewport sizes and the readback
    swap.
  - The water tracer: width profile, bone update, water contact, vertex rebuild, conform to water,
    render constants and ring stitching.
  - The (unreachable) coastline extractor and coast mesh leaves: edge crossing, nearest segment,
    clipping, chaining, resampling, marching squares, the "MitCoast" header and vertex decode.
  - MSAR header, CRC, TOC and validation; the MPKG decode; the memory block; the inflate rewind.
- **Cloud, atmosphere, sky, waterdrops (part c).**
  - The cloud manager, cloud instance (lighting, alpha/scale, type token), the cloud class statics,
    and group visibility.
  - Sky box rotation and luminance.
  - The lightning storm teardown.
  - The waterdrops constructor and the drop fill (00BC2340, traced through all four corners). The constructor
    (00BC2030) is waterdrops_construct_00bc2030 on native storage: the pooled "waterdrops" name,
    GeneratedModel base 00B75030, vtable 00D64760, then its own fields (cc13-fixes2; the earlier version
    omitted the base constructor and the vtable). waterdrops_construct_fields keeps the field stores for
    the semantic 00BC2D00 model.
- **Cloud puffs, render models, pools, lightning strands (part d).**
  - Puff geometry and sorting helpers.
  - The cloud render model.
  - One model of the three paged model pools (16, 4 and 8 slots per block), including the trim
    policy and the 2n+2 block growth.
  - The lightning strand model: segments, strands, bursts and thunder sprites (00BC84A0, 00BC9160,
    00BC9390, 00BC7170), and the 300-entry quad index pattern of 00BC7460.

## RNG (the RNG rule)

No reconstructed function draws. Native draws, and where they come from:

| address | native source | in this packet |
|---|---|---|
| 00BC84A0 lightning strand bursts | `00BD2F10` **stream 0** (shared gameplay stream): 1 draw per spawn for the burst count; 6 per burst; 4 per segment; 3 per thunder sprite; 1 to re-arm the timer | reconstructed; the values come through a host callback |
| 00BC9390, 00BC7170 | stream 0 indirectly, through 00BC84A0 | reconstructed, same callback |
| 00BC7C10 | stream 0, one Uniform(0, 2pi) roll; unreferenced | reconstructed, the value is an input |
| 00BAE2E0 water spray builder | `00BD2F10` stream 0 | skipped (D3D builder) |
| 00BA2FF0 ocean beams | private generator `00E130C0` (`BSP_RandomState_NextU32`) | existing (src/water_render.cpp) |
| 00BA44F0 | its own inlined Mersenne Twister on private state `00E13A88` | skipped (D3D builder) |

If a host ever wires the lightning strands, it must give them a separate presentation stream, or
keep them behind an OFF switch with a comment recording that the image shares stream 0.

## Findings worth a second look

1. **Existing header disagreements.**
   - `include/bsp/water_render.hpp` declares `WaterRingRecord::b_10` as uint32. 00B9CB80 multiplies
     it as a float.
   - `include/bsp/weather_render.hpp` puts `CloudInstanceState::alpha` at +1C4h. SetAlpha (00BBE490)
     writes +1D8h and SetScale writes +1C8h; +1C4h is only set to 1.0 by the constructor.
2. **SetFoam argument order.** 00B9E8B0 stores FadeRange at +8Ch and ScrollSpeed at +88h (checked
   against the caller at 0078CA14).
3. **Six cloud "instance" setters write class-wide statics.** The ambient, diffuse, specular and
   inside colours, the density and the blur radius are stored at 01090970/80/90/A0, 01090908 and
   01090904. ECX is unused.
4. **Native quirks kept in the model.**
   - 00B9E4A0 frees a ring even when the erase did not find it, and every removal reorders the rings.
   - 00BC84A0 and 00BC9160 remove expired records by copying the last record over each slot in
     ascending order without re-checking the moved record. With {1, 3} of 4 expired, record 3
     survives and live record 2 is dropped.
   - A strand segment's alpha falls by life x dt per tick, so at expiry it is 1 - life^2, not 0.
   - The cloud group node is shown when flag == 1, but its registered nodes get float(flag).
   - 00BBF1A0 converts degrees with the double 3.141592025756836 (00D64610), a float-rounded pi.
   - 00BAA8F0 discards the tracer curve's virtual result, so the wake squeeze is a fixed 0.05.
   - 00BAEB20's crossing interpolation never subtracts the water level (correct only at level 0).
     This is from the pseudocode only.
   - 00BBB890 never uses the result of its "TOC" tag compare.
   - The MSAR CRC-32 chain starts from the literal 61746973h.
5. **Possible overflow.** 00BA5240 sizes its index buffer as H*W*2 but writes (H-1)*(2W+1)
   indices. Whether that overruns depends on the factory's size unit.
   - Fixed by cc13-fix5 (2026-10-16): `ocean_wave_field_projected_grid_00ba5240` used to return
     the UVs and indices as data. It now takes the wave field and an `OceanProjectedGridHost` and
     makes every call at the image's point. In order: geometry 00B74640, 00B73C10, and 00B73B70(0);
     release of the old +124h and its clear; renderer vtable 38h/5Ch/60h; the vertex and index
     buffer lock/unlock; 00B85EE0/00B857F0; draw-section +8h..+18h; 00B864C0, 00B85B80 twice
     (+124h, then 00BA7080 on +2BCh), 00B86650 and 00B73C60; then both releases. The new +124h is
     stored. The UV and strip math is unchanged.
   - Also fixed by cc13-fix5: 00BB4B40 is now `pak_archive_registry_slots_push_back_00bb4b40`. It
     is the whole push_back: reserve through `reserve_native_pak_registry_slots_00bb46f0` when full
     (capacity*2, 1 when that is <= 1), the element store (skipped only for a null slot address,
     as in the image) and size += 1. It was previously only the capacity calculation.
   - Fixed by cc13-fix6 (2026-10-16): the coast vector helpers had the same shape. They now work
     on a `CoastNativeVector<T>` {data +0h, size +4h, capacity +8h}. The push_backs
     `coast_segment_vector_push_back_00baf350`, `coast_ptr_vector_push_back_00baf3e0` and
     `coast_point_vector_push_back_00bafc00` call their own reserve when full (capacity*2, 20h
     unless above it). Then each stores the element (skipped only for a null slot address) and
     does size += 1. The reserves `coast_ptr_vector_reserve_00baeeb0`,
     `coast_segment_vector_reserve_00baefb0` and `coast_point_vector_reserve_00baf450` had been
     capacity-only too. Each now floors the request at 20h. When the request exceeds the
     capacity, it allocates through `CoastHost::operator_new_00bf55be`, copies the live elements
     dword by dword, frees the old block through `CoastHost::free_00bf6989`, and stores data and
     capacity. None has a caller in src.
6. **Possible wrong names (not renamed).**
   - 00BBC690 `BSP_WaterTextureSource_GetDescriptor` is a runtime type-token test on 0109E9D4.
   - 00BC6430 `BSP_RefCountedRecord_Construct` is the cloud puff constructor (vtable 00D64940).
   - 00BC33B0 `BSP_WeatherLayerSet_ForEachLayerSlot8` runs on the cloud class and writes the 4
     vertices of every puff at slot*80h.
   - 00BC3410 `IndexThunk` returns the slot of the i-th sorted puff.
7. **Unreferenced code.** 00BC3580 (`PuffEdgeFactorAtPoint`: its running minimum starts at 0.0, so it
   returns 0 for every ordered input), 00BC7C10, and an unnamed stub at 00BC66A0 (`MOV [ECX],00D64958;
   JMP 00BC8380`).
8. **Decompiler drops.** At 00BAB3F0 Ghidra drops the sqrt argument; the listing has
   float((+1E8h - interp) * +1E4h). At 00BB9180 it drops the size term of the final block length,
   which was taken from the twin in `src/mpkg_archive.cpp`.
9. **Unresolved inputs.** At 00BAF8A0 the index stream seems to come from a local that is never
   written (possibly a stack-offset miscount; it is left as an input). 00BF85B0 (a CRT rounding
   call) is modelled as a host callback.
10. **Ghidra name drift.** Several functions with ledger names still show `FUN_` in Ghidra (for
    example 00BB4DC0, 00BB8530, 00BBBCF0, 00BBE760). This packet did not re-annotate them.

## Left without a record (177)

Container plumbing (vector, pool, list and NativeString helpers), constructor, destructor and
factory shells, runtime plumbing (allocator, critical section, event, log, error reporter) and the
D3D/resource builders. The per-helper lists are in the worktree's `local/w1_{a,b,c,d}` final reports.
- The D3D builders are 00B9DC20, 00B9EA50, 00BA1400, 00BA24B0, 00BA2E60, 00BA3BD0, 00BA44F0, 00BA4EB0,
  00BA7530, 00BAD150, 00BADC70, 00BAE2E0, 00BB3DB0, 00BC2D50, 00BC54F0 and 00BC6060.

## For cc9

Nothing here touches the live-sea host or any cc9 file. For the live sea:
- The header disagreements in finding 1 affect `water_render.hpp` and `weather_render.hpp` consumers.
- Finding 2 changes which foam constant is the fade range.
- The lightning strand RNG is stream 0 (see the RNG table) if weather is ever wired.

## Handoff

Branch `agent/cc13-water1`. Next steps, if anyone continues:
- Resolve the two header disagreements with the owners of those headers.
- Check 00BA5240's size unit and 00BAF8A0's index source against the listing.
- Re-annotate the ledger names that Ghidra still shows as `FUN_`.
- The skipped D3D builders above are the largest remaining bodies.
