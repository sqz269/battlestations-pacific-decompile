# Sampler stack inputs at consumption (R98)

The full `00B3B280` sampler path can now omit a release-stack capture when a
subsequent native write replaces that word before anything reads it. This removes
an unnecessary evidence requirement without inventing or dropping native bytes.
The compiler's first source0 sampler still needs its caller's original entry word.

## Native data flow

With `B` the sampler loop's steady ESP, `00B5F100` writes only the stage byte at
`B-16`, then copies the **full DWORD** into the pass's eight-byte texture row.
The source keeps that behavior. Draw code's byte reads do not establish that the
other three bytes can be discarded: the native pass-copy path preserves them.

| Event | Effect on B-16 knowledge |
| --- | --- |
| Caller supplies a captured entry word | Known until a later native write/release. |
| Source1 wrapper returns | Its `CBC760` push establishes the word. |
| Source1 binding returns | The parent return PC is `B3B2E4`. |
| Source1 resource release returns without a capture | Unknown; old host scratch contents cannot be consumed. |
| Source3 effect-name call | Saved ESI establishes the actual sampler pointer. |
| Sampler-state call at `B3B369` | Its return PC `B3B36E` overwrites the word before setter entry. |
| Texture-reference call at `B3B328` | Requires known bytes before entering unchanged `B5F100`. |

`NativeMaterialDescriptorSamplerStackKnowledge` describes the same borrowed
scratch word. It adds no native storage or ownership domain. A null pointer keeps
the prior strict API, including one captured residue after each successful
source1 release. The optional path accepts ordered sparse captures; matched
captures still require the exact sampler index and released-owner identity.

Uncaptured releases mark the word unknown. They do not substitute zero, the old
value or a guessed return address. Later established writes restore knowledge.
A reached unknown read throws with the parent at `B3B328`, before creating the
reference child or appending its row. Earlier binding/refcount/counter effects
remain in their native domains; failed operation state cannot replay or silently
retire. Carrying knowledge between calls requires proof that their native stack
slots are the same.

## Validation

- Strict MSVC Win32 build and all three existing CTests passed. No permanent
  tests or extra build targets were added.
- Five affected native contracts (`B3B280`, `B5F100`, `B5ED60`, `B44CF0`,
  `B18FB0`) have 724 live Ghidra bytes matching the original PE. All nine native
  bodies embedded in the reused diagnostic also match the original PE.
- The existing nine-sampler native/source differential still passes with its
  explicit release capture. It covers source0/1-hit/1-null/2/3/unknown and live
  descriptor/list/stage/counter mutations through the established child adapters.
- One variant adds a sampler-state write after the successful source1 release.
  The optional path consumes **zero release captures** and matches all 25 output
  words. The following reference contains exact DWORD `00B3B301`: the return
  address's upper bytes and the later stage byte, without masking comparison.
- Separate process cases reject unknown entry and consumed-release words at
  `B3B328`; they verify no reference child/row was published, earlier effects are
  retained, and replay fails. Those failed native frames remain until process exit.
- The unmodified application exits 0 after two ticks and one Present, joins its
  worker and releases D3D device/API to 0/0. This is a startup smoke check.

The first new comparison exposed the old diagnostic's relocated return PC.
Its executable/logs were preserved. The final harness runs the parent at actual
`00B3B280`; eight child bodies remain relocated with existing adapters. Reserving
that band after startup collided with a process heap, and moving the reservation
shifted the heap into a fixture data band. Those attempts were also archived.
The final launcher creates its own suspended child, verifies each requested band
is free, reserves all five fixture-owned bands before loader initialization, and
has the child verify them before use. No occupied allocation is replaced.

See [the report](../reports/native_sampler_stack_liveness_r98.json) for call-site
checks, original bytes, raw source/build hashes, exact process exits, preserved
attempts, saved Ghidra annotations and immutable artifact receipts.

## Limits and next dependency

The original/source fixture shares pooled allocation, resource/cache and atomic
adapters, uses null-online/input guards and retains its arena. It does not prove
the actual Windows decrement implementation, native FH3/SEH, nonnull online/input
paths or full failed-owner cleanup. Ordered sparse later captures are inspected
in source; the fixture does not enumerate malformed capture lists.

Production compiler calls at `B3C018` and `B3C024` still need a proved entry-stack
producer whenever their first observed reference precedes an established write.
The complete application preload/compiler/factory graph, scene rendering and
gameplay remain unfinished. This packet adds no unique native function.
