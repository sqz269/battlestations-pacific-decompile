# Sound effect loading and sample lifetime

Addresses: 0086EF60, 0086FA90, 0086FB00, 005B8EA0, 005B8FA0, 005B9AF0.
Dependencies and native cache ABI: `docs/SOUND_SAMPLE_CACHE.md`.
Names are hypotheses, not recovered symbols. The compiler scalar name at
0086FB00 is preserved. `reports/gameplay_effect_sound.json` records byte spans,
original prototypes, validation source/library hashes and integration.

## Concrete dispatch and storage

The acquisition dispatcher now recognizes current Sound tableD0DA18:
Lua slot14 is0086EF60, scalar slot4 is0086FB00. Slot0 is00BD30E0, which calls
current slot4 with flags1. Sound joins the preceding seven scalar families.
Particle, Tracer, WaterTracer, Flare and ThunderStorm remain required services.
The dispatcher requires `EffectSoundContext` explicitly.

The actual Sound allocation is3Ch. Name8/C and common fields come from the
preceding constructor/base-reader packet. Samples use a real12h header at
20/24/28 (pointer/count/capacity); Persistent is byte2C, class index30, type
index34, PitchRnd float38. Padding and unwritten bytes are preserved.

## Reader0086EF60

Call the common base reader, then retain SampleTable until the very end.
Create one empty pooled sample-name string and reuse it across entries.
If SampleTable is a Table, start at signed index1. Get the index solely to
test Nil and release the probe. Nil terminates immediately; otherwise get
the same index again, convert with bare lua_tolstring, resize the sample-name
string with preserve=false and copy exactly its current length. Release this
second reference before acquiring the sample. Non-string/non-number entries
produce a null C string and therefore an empty sample name. Numeric values
follow native Lua conversion. Index increment wraps as a DWORD. There is no
table-length bound or scan past the first nil. An empty Table suppresses the
single Sample fallback.

When SampleTable is not a Table, do the same nullable name assignment using
Sample, release that field, then acquire once. Both branches read current
cache[00F8BBE8], call concrete00A83FD0 with a real output slot, and append that
slot through005B9AF0. Read the temporary pointer only after append/growth
callbacks. InterlockedDecrement(sample+4), and current slot0 only on zero,
releases the temporary. The table branch clears its temporary after a nonnull
release; the single branch does not.

Next read PitchRnd as Number-only/default0, store38, release the field.
Category uses bare string conversion followed by0041E870 construction:
it requires a nonnull converted C string, as the unguarded native strlen does.
No missing-category fallback is invented. Release its Lua field, read current
sound owner[00F8BBD8], invoke concrete00A7ACF0 and store the class index at30.

SoundType accepts only Lua String, otherwise `Normal`, via the tracked
00B685C0 constructor contract. Release its Lua field before re-reading the
current sound owner and invoking concrete00A7B0A0, then store34. This second
owner lookup is independent of the category lookup. Persistent uses Lua
truthiness, writes2C, and releases its field. Cleanup order is type name,
category name, sample name, then retained SampleTable.

## Arrays, destruction and ABI

Assembly verifies005B8EA0/005B8FA0/005B9AF0 share normal-flow behavior with
concrete0086E770/0086EDD0/0086EB60. Addressed wrappers reuse those actual-header
implementations. Reserve retains ascending before old releases, clears
captured slots after callbacks, reloads data for free, and publishes data and
capacity. Append grows only at count==capacity and clears the destination
before reading/retaining its source. Resize decrements actual count before
descending releases and reloads it afterward. These are custom intrusive
arrays, not a copied STL port.

Destructor0086FA90 writesD0DA18, resizes samples0, reloads/frees buffer20, then
calls common0086B7E0. Name/array headers remain dangling; capacity survives.
Scalar0086FB00 destroys, frees only for flags bit0 and returns the original
pointer. Readers/array operations use ECX owner/header plus one stack
argument, RET4. Destructor uses ECX owner/RET. Scalar uses ECX owner, stack
flags, EAX original pointer/RET4. Rebuilt APIs are not native vtables or SEH.

The19-byte reserve and3-byte scalar fall-through gaps were repaired. The
35-byte Sound destructor tail was decoded and its override cleared, but the
stored body still ends atFACF. Reachable cleanup and RET atFAF2 come from
verified disk/live assembly. Two initial flow entries were overwritten by
overlapping report writers; the report transparently recovers captured
console evidence and fresh byte/listing checks, without inventing the
unavailable original response strings. Stored-body repair remains open.

## Validation and follow-ups

Win32 Release and both existing tests passed. A focused real-Lua fixture loads
three Sound components through acquisition: indexed samples, empty-table
suppression and single Sample. It checks separate probe/read calls, numeric
conversion, defaults, both current-owner lookups, cache matching, actual
reference counts and full component/definition cleanup. Lua iteration order
is observed, not assumed from numeric keys written as explicit hash entries.
The scalar fixture now uses Particle as its remaining-service case; it and
the full startup fixture pass with the mandatory Sound context.

Sample payload constructor00A84D70 and resource zero-reference behavior remain
required bindings. The fixture supplies controlled7Ch samples; metadata,
VFS/FMOD loading, native Sound differential, ABI/SEH, playback and gameplay
remain unvalidated. Continue with that constructor, its sample destructor,
and the five remaining component families under fresh leases.

## Correction from docs/SOUND_SAMPLE.md

The sample-construction service now has a concrete `SoundSampleRuntime`
implementation, with actual sample storage, settings, event parameters,
cache removal and final release. Installed FSB/FEV checks passed through the
existing VFS/resource/FMOD runtime. This adds a production composition for the
earlier Sound loader; it does not convert the earlier controlled fixture into
gameplay or audible-playback evidence.
