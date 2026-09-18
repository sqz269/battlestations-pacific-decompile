# Convex/ray dispatcher owner storage

## Correction and scope

`DynDispatchGlobalsStorage::convex_ray` previously reserved only its four-byte
vtable word. The original `00C44780` method writes through the double at
offset `98h`; using that method with the old aggregate would overwrite later
members. The member now uses `DynConvexRayIntersectionStorage`, an eight-byte
aligned, `A0h` record. The aggregate remains a new C++ layout, not a reproduction
of the entire contiguous native globals region.

This corrects the seven-one-word-owner claim in
[the original initialization packet](DYN_DISPATCH_INITIALIZATION.md).
Its test compared the initialized table words and never called the collision
methods. That evidence did not establish the sizes of their owners.

R143 supplies storage and binding evidence only. `00C44780` is still a copied
original reference in the fixture; this packet does not reconstruct its body
or publish a source convex/ray table. Terrain/convex, world task dependencies,
application admission and gameplay remain open.

## Native evidence

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
The complete `00C44780..00C47A8A` body is 13,067 bytes / 4,160 instructions.
The collection verifies 13,400 bytes against both the PE and live Ghidra,
including three consumed native math helpers, five jump-table entries,
constants, the vtable and the owner image through the next two static words.

| Offset | Storage | Evidence |
| --- | --- | --- |
| `00` | vtable pointer | PE `00E17448` contains `00D7A1F4`, whose sole slot is `00C44780` |
| `04` | padding word | zero in PE; no identified method access; fixture preserves poison |
| `08..67` | four triples of doubles | `00C44C74` selects `owner + 8 + count * 24`; three double stores follow |
| `68` | simplex count | cleared at `00C44A00`, incremented at `00C44C7B`; `00C44CB2` bounds the five-way switch |
| `6C` | padding word | zero in PE; no identified method access; fixture preserves poison |
| `70..87` | current ray point | initialized from start at `00C44A10/15/1A`, advanced at `00C44BFB/00C44C00/03` |
| `88..9F` | search direction | writes at `00C44A58/60/66`, followed by normalization and simplex updates |

Field names describe observed use; they are not recovered symbols. ECX is
saved in EDI at `00C4479E`. The root aligns its private stack to 64 bytes;
that is not an owner alignment requirement. The native owner address is
eight-byte aligned. Its table word is followed by `9Ch` zero bytes, and
the next object begins at `00E174E8`, exactly `A0h` later.

The native method receives owner in ECX, then result, convex shape, start
and end pointers on the stack. Its normal returns pop `10h`; only AL carries
the hit result. It composes the shape/body transforms and calls the shape's
double support slot at `+0Ch`. The source shape support table and recovered
CRT numerical service are shared fixture dependencies, not independently
revalidated implementations in this packet.

## Binding behavior

`bind_dyn_dispatch_static_objects` still publishes the seven PE table words.
For convex/ray it leaves all other bytes, including scratch and padding,
unchanged. Fresh value-initialized storage supplies the original zero scratch
image. `dyn_scene_dispatch_objects` returns the full owner's address.
Compile-time offset, size and alignment assertions enforce this layout in the
required MSVC Win32 build.

## Focused validation

One ignored, archived probe exercises the concrete overflow risk. It invokes
the complete relocated original ray method and its three copied native math
helpers on both guarded `A0h` storage and the corrected aggregate. The original
five-way jump table is relocated to exact instruction starts. Only CRT sqrt
is bridged to the existing recovered service; original internal math calls
remain original. A non-FP memory store marks each native return.

The probe uses the existing body and convex shape constructors, real cube
adjacency and the production double-support method. It checks 32 ray/transform
scenarios with zero and poisoned scratch: **64 pairs, 60,288 identical bytes,
50 hits**, and final simplex counts zero through four. Both sides use masked
x87 control `037Fh` and MXCSR `1F80h`. Compared bytes cover return value, full
owner, guarded output, body/shape records, support seeds, and FP control/status,
tag and MXCSR; FP instruction pointers are excluded.

Every pair checks neighboring aggregate bytes, outer guards, padding words,
all eight borrowed owner identities and scratch preservation after rebinding.
The other seven table bindings use uncalled data identities solely as guards;
they are not callable dispatcher implementations. Only support is invoked on
the borrowed shape runtime; pool allocation and shape destruction are outside
this fixture. Strict Win32 build and all three existing CTests pass.

These checks establish the owner/binding contract for the exercised native
paths. They do not prove the ray algorithm's correctness on arbitrary inputs,
native exception ABI, concurrency, ordinary application startup or gameplay.
Exact artifact hashes, live evidence, annotation history and integrated-build
receipts are in [the report](../reports/dyn_convex_ray_owner_r143.json).
