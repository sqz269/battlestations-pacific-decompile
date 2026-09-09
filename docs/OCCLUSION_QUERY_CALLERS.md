# Native occlusion-query scheduling and flare consumption

This bounded audit identifies three concrete sites: renderer frame-end polling,
flare query construction, and flare consumption of cached counts. It does not
recover the draw submission that calls query Begin/End. Names below are proposed
interpretations, not recovered symbols. No source implementation, Ghidra mutation,
build, or new test was performed for this audit.

## Renderer frame-end polling: 00b2d8e0

The existing `BSP_D3D9Renderer_EndFrameAndPresent` contains the polling loop at
`00b2dadb..00b2db14`. ECX is the renderer. After the scene-ending path, binding
cache cleanup and `00b0ccb0`, it clears renderer `+1998h` and traverses the
borrowed query array `+19a0h`, count `+19a4h`.

At `00b2daf9`, it compares query `+8h` against zero. Only zero-state queries
receive a virtual `+10h` call (`00b2db03..00b2db08`), which is the verified
`00b5fca0` poll method for query vtable `00d62ad0`. The AL result is ignored.
There is one attempt per qualifying entry per traversal: no retry loop, wait,
timeout, or HRESULT-specific branch here. The loop increments its unsigned
index, reloads the count at `00b2db0d`, and reloads the array base at
`00b2daf0`. It is an index traversal, not a captured iterator range.

This occurs before the subsequent optional renderer work and device Present.
The enclosing frame-end path has other gates; this is not a claim that every
invocation of the function polls. There is no lost-device gate inside this
polling loop. Later Present loss handling must not be moved into the query
method. Successful End sets state zero, so frame-end polling continues across
eligible frames until a successful GetData changes state to two. Failed or
pending GetData leaves state unchanged. A state-two entry is skipped until a
later successful End changes it back to zero.

The concrete poll performs only a D3D GetData call and state write; it does not
mutate the borrowed registry. Reloads alone do not establish thread safety or
authorize arbitrary callbacks which append/remove entries.

## Flare construction: 00b8dac0

This ECX-owner method has no stack arguments (ordinary RET at `00b8dd51`).
It initializes material/geometry resources, including the referenced string
`flareocclusion.mshd` at `00d63484`, then creates two query owners:

| Call | Dispatch | Result |
|---|---|---|
| `00b8dd24` | renderer singleton `[00f8d394]`, virtual `+24h` | owner `+1d0h` at `00b8dd26` |
| `00b8dd37` | same renderer virtual `+24h` | owner `+1d4h` at `00b8dd3e` |

The concrete renderer slot resolves to factory `00b27c20`. Its existing audit
establishes allocation, construction, and borrowed listener registration; this
caller stores the returned owned objects. The surrounding owner constructor
`00b8d9c0` references `flare`, installs vtable `00d63420`, and initially zeros
both fields. The observed destructor prefix `00b8d8c0` drops and clears both
owners; its exported body is truncated, so this audit makes no claim about
the complete destructor tail.

The two pointers are not evidence of which draw has depth testing enabled,
nor of a visible-versus-total sample assignment. Their physical draw scopes
remain unrecovered.

## Cached counts influence flare output: 00b8d150

This ECX-owner method takes a pointer as its first stack argument and a float
delta as its second, and ends in RET 8 at `00b8d57c`. The pseudocode mistypes
the first argument as float; assembly dereferences it (including `+198h`).
It accumulates delta into owner `+1c4h`, has an early gate on the supplied
object's `+198h`, and computes projection/attenuation before the query block.

At `00b8d320..00b8d33a`, both owner query fields must be nonnull. Calls at
`00b8d340` and `00b8d34f` invoke virtual `+14h`, the cached-DWORD getter
`00b5fce0`. There is **no readiness check, GetData call, or wait** in this
consumption block. A previous cached result can therefore influence the next
update while another query is pending.

Let `A` be the DWORD from owner `+1d0h`, `B` from `+1d4h`, and `t` the
attenuation already computed by the preceding code. When `B != 0`:

- `r` is the single-precision stored ratio `A / B`. Both DWORDs are treated
  as unsigned: negative signed FILD inputs receive a `2^32` correction from
  float constant `00ce3978`.
- Owner `+ach` receives a float result of `r * t * 0.699999988079071` through
  `00b6da70(value, false)`. The multiplier is the double at `00ceffa0`.
- Owner `+184h` receives `2 * t * t * max(r - 0.5, 0)` with the native x87
  operations and intervening float spills. The double `0.5` is at `00d7a280`.

These expressions describe dataflow, not a promise of equivalent reassociated
floating-point code. There is no upper clamp on the ratio in this block.
Missing query owners or zero `B` produce zero in both fields. The immediate
setter `00b6da70` writes owner `+ach`; its false second argument disables
recursion through the child list. The meaning of that field is not independently
established here, so it is not named alpha or visibility as a recovered fact.

Later code invokes the flare geometry helpers `00b8c8a0`, `00b8cc90`, and
`00b8c6b0`, then restores `+ach` to 1.0 through `00b6da70` at `00b8d570`.
This establishes that cached query results affect flare construction/update
inputs. It does not establish general scene occlusion culling, a draw skip,
or exact final rendered opacity without auditing those downstream consumers.

## Proposed annotations and remaining boundary

Candidate names are `BSP_Flare_InitializeOcclusionResources` for `00b8dac0`
and `BSP_Flare_UpdateFromCachedOcclusionSamples` for `00b8d150` (provisional,
because each body also performs other work). The existing frame-end function
can receive a comment describing its state-zero, single-attempt polling loop.
Useful query-field comments are `first cached sample source` at `+1d0h` and
`ratio denominator cached sample source` at `+1d4h`; do not assign unproven
depth-tested/unoccluded roles.

Direct xrefs for the four query methods and factory principally expose vtable
entries; dispatch-site inspection was necessary. The observed slot `+10h`
and `+14h` calls are concrete query uses. Actual call sites for Begin `+8h`
and End `+ch` remain a gap. Getter helpers `00b8b6b0` and `00b8b6c0` return
the two query pointers, but no direct caller was established; these alone
do not close that gap. The smallest next audit is the flare draw submission
that obtains these owners and brackets geometry with Begin/End, including
its depth states. A generic scene visibility interface would be premature.

## Byte evidence

Live analysis batches verified project `bsp`, program
`/battlestationspacific.exe`, x86 image base `00400000`. The following exact
bounded ranges matched the installed PE byte-for-byte; they are not hashes
of complete caller bodies. Assembly continuation was inspected around these
ranges rather than relying on pseudocode alone.

| Inclusive range | Bytes | SHA-256 |
|---|---:|---|
| `00b2dadb..00b2db14` | 58 | `b1bf5091fbecc066dcc2596a65749d2948b7d112553d3c557e0cf6a70f49e13e` |
| `00b8dd19..00b8dd43` | 43 | `009be5ffba22a12d2d85d9240839dc3227488360e05096b11e4b57fd930a74ec` |
| `00b8d320..00b8d400` | 225 | `7c2de2e1da3f8fa8fd96ec5e3d4169d32d38170412945c9173bde22d11a2c334` |

Representative dispatch bytes: poll `8b 42 10 ff d0`, factory
`8b 42 24 ff d0`, cached getter `8b 42 14 ff d0`. State gating is the
explicit `39 59 08` compare in the frame-end range, not an inferred COM
readiness property. Query method ABIs and their complete-body hashes are
recorded separately in `OCCLUSION_QUERY_USE.md`.

Parent integration subsequently applied the two provisional flare names and
extended the existing frame-end comment with the polling evidence. Prior values
were recorded locally, existing comments preserved, the project saved and all
three function exports refreshed. This does not implement these caller bodies.
