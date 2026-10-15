# Movie factory bootstrap and decoder allocation

Owned addresses: `00737710`, `00737730`.

The two complete bodies are reconstructed in `src/movie_factory_followup.cpp`
and declared in `include/bsp/movie_factory_followup.hpp`. Their names are
descriptive hypotheses. The factory composes the existing CRT allocation
service and the existing decoder constructor; it introduces no codec,
allocator, singleton-manager or constructor stubs.

| Routine | Native body | Coverage | Original ABI |
| --- | --- | --- | --- |
| `construct_movie_factory_00737710` | `00737710..0073772d` | complete | ECX=owner; EAX=owner; plain RET; no stack arguments |
| `create_movie_decoder_00737730` | `00737730..00737798` | complete | ECX unused; one ignored DWORD stack slot; EAX=shared-base pointer or null; RET4 at 0073777c and 00737796 |

## Bootstrap and external contracts

The bootstrap first invokes the existing singleton-registration helper on the
same owner. That helper's inspected body installs base table `00cfea60`, obtains
the lifetime manager and captures its section at +10h, enters and increments the
section's tracked +18h recursion word when nonnull, publishes the owner to
`00f8aefc`, obtains the manager again, reloads the current publication for
registration, then decrements and leaves the captured section. Assembly confirms
that reload after the second getter; a binding must not substitute a captured
owner if the publication changed. This packet requires the whole helper as a host binding;
it does not claim a replacement for its locking, publication or unwind behavior.

After registration the bootstrap writes derived table ID `00cfea78` and calls
the Bink boundary with `BinkOpenDirectSound` and zero. Its result is ignored,
including failure; the function still returns the original owner. The prefix
type projects only the word actually written by this body. Its size is not
evidence of the complete factory object's allocation size.

| Host or existing source step | Call site | Native callee | Inspected contract |
| --- | --- | --- | --- |
| `register_movie_factory_007366b0` | `00737713` | `007366b0` | same ECX owner; complete helper body inspected as described above |
| `set_bink_direct_sound_system_00a4d356(0)` | `00737725` | `00a4d356` | two pushed DWORDs; stdcall Bink import; callback then zero; result ignored |
| `singleton_lifetime_allocate` | `00737748` | `00bf681b` | native one-argument cdecl allocation service; 38h pushed at 00737746, ADD ESP,4 at 0073774d |
| `construct_movie_decoder_00a4c660` | `00737763` | `00a4c660` | allocated ECX owner, hidden complete-object flag 1; constructor RET4, EAX=owner |

`00a4d356` is the established import thunk through IAT `00ce23d0`, named
`_BinkSetSoundSystem@8` in the original PE import table. The callback address
`00a4d35c` is not a Ghidra function or decoded instruction in the current saved
analysis. Original disk bytes `ff 25 d4 23 ce 00` decode to a jump through
`00ce23d4`; the PE import at that slot is `_BinkOpenDirectSound@4`. It is an
external import thunk, not an extra reconstructed address or a callable native
address in the C++ build. The required host method binds those actual imports.

## Allocation, construction and returned identity

The allocator callee was read before reuse. It loops over malloc/new-handler
retry and throws `bad_alloc` when the handler declines; the existing
`singleton_lifetime_allocate` is the repository's documented host CRT service
for this native entry. The source preserves the explicit null test even though
the supplied service ordinarily succeeds or throws.

The native `PUSH ECX` at `00737745` reserves a local slot; it is not a second
argument to operator new. The `ADD ESP,4` immediately after that call proves the
single 38h allocation argument. The source function retains the ignored source
stack slot separately; native `RET4` consumes it. The original ECX value has no
semantic use in this body.

On nonnull allocation, the hidden constructor argument is exactly 1. The
existing complete-object constructor returns its same nonnull input, so its
separate native post-constructor null branch is unreachable under that
established contract. No host call can replace the constructor with a nullable
factory in this wrapper. The result is then adjusted using the file vbtable's
signed word at +4. The constructor writes vbtable `00d24de8`, whose +4 is 10h;
the existing `NativeMovieDecoder::state` is at +10h. Live bytes at `00d24de8`
are `00 00 00 00 10 00 00 00`, confirming that signed offset.
Returning `&decoder.state`
therefore preserves the actual shared-base identity. The allocation is 38h and
remains owned by the caller's established decoder destruction/deallocation path.

The original factory installs an exception-registration record pointing to
`EHHandler_00c861eb`. The typed constructor is noexcept; the allocator throws before
publishing a successful allocation. Source behavior does not require a new
cleanup branch here. Original SEH metadata, CRT exception identity and binary
call compatibility remain unproved.

## Verification and limits

All live query/export batches use `bsp.py ghidra`, which verifies configured
project `bsp` and program `/battlestationspacific.exe` before the query. Local
configuration selects `C:/Users/sqz269/bsp.gpr`, language x86 LE32 and image base
00400000. Both anchor boundaries, complete listings, registration and allocator
bodies, existing decoder source/layout, and the original PE import table were
checked. Live xrefs show no references to either anchor; no invocation or
application wiring is inferred from that absence.

The exported callback attempt failed because `00a4d35c` is uncreated; all six
other requested exports succeeded. Disk bytes and the import table resolve that
external callback without changing Ghidra. The primary owns any later naming or
comments. No shared source/header or packet registry was changed by this packet.

Build and mechanical call-check results are recorded in
`reports/oct15_movie_factory.json`. The worker's full MSVC Win32 Release
`./scripts/build.ps1` completed successfully: `reconstructed_math` and
`tool_tests` passed, the two suites enabled in that checkout. Native differential
tests were not enabled there. The primary integrator separately reports that
combined revision `0db8158e2` passed the full build and all three suites,
including `native_math_differential`, before promotion to main. All four JSON
call-site rows pass the live mechanical check; the markdown checker has no
failures and one unpaired prose claim. No new test cases were added. The functions
are not wired into `bsp_game` by this packet, and no runtime/frame-timing claim
is made. Evidence remains complete bounded Source reconstruction and whatever
build/check results are explicitly recorded; original ABI, Bink execution,
singleton activation, decoder playback, visuals and gameplay are unvalidated.
