# CC13 water 2b: container plumbing left by cc13-water1 (cc13-water2b)

Worker cc13-water2b on packet cc13-water2 (branch `agent/cc13-water2b`; the name `agent/cc13-water2` was taken by an
older, merged pass), 2026-10-13. This continues docs/CC13_WATER_1.md over segments 92-94, `00B999F0`-`00BD2E60`.

Status: **reconstructed, build-tested** only. That means `./scripts/build.ps1` passed and ctest passed 2 of 2.
Nothing here is ABI-compatible, fixture-tested or game-validated, and nothing is wired into a host. No test was
added. The code is "Part e" at the end of `include/bsp/water_weather_cc13.hpp` and `src/water_weather_cc13.cpp`.

## Counts

| | count |
|---|---|
| left without a record by cc13-water1 (index at the start) | 177 |
| **reconstructed and recorded** | **78** |
| left | 99 |

## Method

> **Lead correction (2026-10-13):** the "drops memory stores" claim below is wrong. A raw
> `python tools/bsp.py ghidra decompile 00BA2D20` and `... 00B9D1E0` both show every store the
> listing has (`*param_1 = 0; *param_1 = param_2;`, the copy loop, `*param_1 = new; param_1[2] = cap`).
> The omissions came from this worker's own reading pipeline, not from Ghidra. The live decompile is
> fine; the listing-based bodies below are still correct.

Every body was read from a capstone listing over the PE (`local/w2tools/w2_fn.py`, `w2_dis.py`). The live Ghidra
decompile of these bodies **drops memory stores** (2026-10-13). For example:

- `00B9A760` clears its slot after the free, and the decompile omits the clear;
- the RefHandle assigns omit the store of the new pointer;
- the POD reserve loops show empty copy bodies and no data/capacity stores;
- `00BB64A0` and `00BBA8F0` lose their zero and byte stores.

Twin groups were confirmed by comparing normalized instruction streams (`local/w2tools/w2_twin.py`). Bodies
byte-identical to an `engine_services_cc13` shape call it:

- free-and-clear → `free_owned_block`;
- reference release, release-return-self and assign;
- pointer-array grow (`00BF2EE0`).

The model works on the native storage: `{data, count, capacity}` arrays (`engine_services_cc13::NativeArrayHeader`),
intrusive reference slots and eight-byte strings over `NativeStringStorage`. Operator new and free map to
`singleton_lifetime_allocate`/`free`. A vector routine this part does not reconstruct is bound by the caller as a
function pointer (`W2_Reserve`, `W2_Resize`), and the header names each one. Most of those routines have
semantic `std::vector` models from cc13-water1, so they cannot be called on native storage directly.

## What was reconstructed

- **POD vectors** (generic over element size; DWORD copy, or FLD/FSTP copy for the float records):
  - reserve with a minimum of 1: `00B9D1E0`, `00B9D3F0`, `00BC3180` (4 bytes); `00B9D240`, `00BC8070` (8);
    `00B9D2B0` (1); `00BC31E0` (16, as floats);
  - resize with zero-filled growth: `00B9D310`, `00B9E040`, `00BC3270`, `00B9D360`, `00B9DF80`;
  - push back, growing to `max(2c, 1)`: `00B9DE90`, `00BC3DE0`, `00B9DF40`, `00BC8210`, `00B9D3B0`, `00BC3E20`,
    plus `00B9DFD0` over the bound reserve `0073EAC0`;
  - destroy: `00B9E560`, `00B9E580`, `00B9E6D0`, `00BC4600`, plus seven over bound resizes;
  - byte assign `00B9E5D0`;
  - the inline count-down clears and destroys (14 functions) and `00BB0590`.
- **Storage and slots.**
  - Free-storage at `+8h`: `00B9D460`, `00B9E090`.
  - Free-and-clear: `00B9A760`, `00B9A780`, `00BA2AB0`, `00BA2AD0`.
  - Reference release, release-return-self, add-ref and assign-without-add-ref.
  - Node handle release `00B9AB40`, over the bound `00B6DFA0`.
- **Strings and archive records.** String releases (`00BBABF0` takes its argument on the stack, RET 4), archive
  name-record destroys, name-record copy, archive entry record construct and copy.
- **Streams.** Stream buffer read-whole and free; memory stream from a backing object (`00BB9100`). The listing
  dereferences a null allocation.
- **Lightning segment records.** 30h float copy and remove-swap-last.

## RNG

Nothing reconstructed here draws. `00BAE2E0` (water spray, which draws from stream 0) is not reconstructed.

## Left (99)

- The 16 D3D builders listed in docs/CC13_WATER_1.md.
- The constructor, destructor and factory shells: ocean beams and water particles, wave field and height
  generators, tracer and spray resources, coast mesh and coastline, archive providers, procedural texture
  factories, cloud classes, instances and render models, model pools, waterdrops, lightning models, and the
  application create helpers.
- The coverage-region and cell-grid vectors (`00B9E6F0`-`00B9FFF0`, element 30h with an embedded byte vector).
- The lightning segment reserve and append (`00BC7FA0`, `00BC8150`).
- The runtime plumbing at `00BD15D0`-`00BD2B30`: allocator destroy, counted critical section, event, string
  builder, singleton getter, log message, named string tree and FailWithLocation.

The live decompile is unreliable for all of them while it drops stores, so read the listing.

## Handoff

Branch `agent/cc13-water2b`. The next steps, in order:

1. The coverage-region vectors, which build on `w2_pod_*` for the byte vector.
2. The lightning segment reserve and append.
3. The shells.
4. The D3D builders.

### Continued by cc13-left1

docs/CC13_LEFT_1.md (Part f) reconstructs 88 of the 99 listed here. Eleven are still open, and its
Handoff section lists them: four plumbing bodies on the STL map, six builders and `00BAE2E0`.
