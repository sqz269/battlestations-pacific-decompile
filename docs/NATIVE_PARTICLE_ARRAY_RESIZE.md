# Native particle-model cookie array replacement

Addresses: `00AFD130`, `00AFD220`, `00AFCD90`, `00AFDAC0`.

The model190 owner's actual byte and108h-record headers now use complete
concrete destructive replacement functions. They reuse `NativeParticleArrayStorage`
from `native_particle_model_construction.hpp` and the existing concrete cookie
destruction from `native_particle_model_lifetime.cpp`. No second record layout,
owner registry, constructor callback, or zero-filled record substitute is introduced.
Names are descriptive hypotheses, not recovered symbols.

| Routine, inclusive span | Original ABI | Coverage |
| --- | --- | --- |
| AFD130..AFD1CE | ECX8h byte header; stack signed count; RET4 atAFD1CC length3 | complete for valid native cookie storage |
| AFD220..AFD2D4 | ECX8h record header; stack signed count; RET4 atAFD2D2 length3 | complete for valid native cookie storage |
| AFCD90..AFCD95 | ECX byte; EAX same address; RET atAFCD95 length1 | complete |
| AFDAC0..AFDAE0 | ECX108h record; EAX same address; RET atAFDAE0 length1 | complete |

AFD2E0 is the checked producer of the actual18h owner: model00 is borrowed,
bytes04 and records0C are8h headers, and word14 is initialized separately.
Its only direct calls to these replacement functions are AFD329 and AFD331,
passing the same signed count. The native iterator takes AFCD90 and AFDAC0
through the immediate constructor pointers atAFD19D and AFD2A0. Their live
xrefs show no other callers. Existing AFD9F0 is the literal-RET destructor for
both model-update temporaries and these108h records.

Both replacement bodies always destroy the old allocation first, including
when the requested count equals the current count. They capture data00 and
the signed DWORD cookie atdata-4, reverse-destroy exactly the cookie's elements,
and free that captured cookie address. The descriptor's count04 is not read.
The original header remains unchanged until new allocation and construction
have completed. The implementation passes a local header containing the captured
data pointer to concrete AFD0F0/AFD1E0, so their final header resets stay local.
Those helpers call the concrete AFCDA0/AFD9F0 literal-RET destructors.

AFD130 computes unsigned count+4 with carry saturation toFFFFFFFF. AFD220
first saturates unsigned count*108h on multiplication overflow, then saturates
the addition of4. Both request these exact bytes through the canonical
`singleton_lifetime_allocate` CRT domain, also used by the owning constructor.
This boundary performs real malloc/new-handler retries and throws bad_alloc;
BF55BE's checked native body uses the same retry/throw contract. It is a host
CRT service, not a reproduction of the game's CRT heap globals or exception
object. A zero count still allocates a4h cookie. The native null-result branch
is retained, although the canonical allocator returns storage or throws.

On successful allocation, write the requested count cookie, construct elements
in ascending order using the signed iterator comparison, publish data00, then
publish count04. No elements are copied from the old allocation. AFCD90 writes
one zero byte. AFDAC0 first captures the CURRENT DWORD atD7A24C with MOVSS,
then writes recordA4=0, recordA0=0, recordB4=captured bits. The other252 bytes
remain untouched. Installed D7A24C is3F800000, but the source consumes the
actual borrowed volatile DWORD for every element and preserves arbitrary bits,
including signaling NaNs and signed zero. This path contains no x87 arithmetic.

The valid storage domain requires an actual allocated cookie span, signed
nonnegative old cookies, and writable element extents. Requested counts are
not sanitized; native unsigned allocation saturation and signed construction
conditions are retained. A failed new allocation leaves the old header stale
after freeing its data, as in the original. No stronger rollback is added.

| Containing routine and direct site | Native callee | Checked contract and cleanup |
| --- | --- | --- |
| AFD130: AFD15F; AFD220: AFD252 | BF7C6E | reverse vector destructor; stack(data,stride,cookie,dtor), RET10h |
| AFD130: AFD165; AFD220: AFD258 | BF6989 | returning free thunk through BF65AC/BF9DC8; ADD ESP4 atAFD16A/AFD25D |
| AFD130: AFD180; AFD220: AFD283 | BF55BE | real allocation/new-handler/throw; one word, ADD ESP4 atAFD185/AFD288 |
| AFD130: AFD1AB; AFD220: AFD2B1 | BF7CD1 | ascending signed vector constructor; stack(data,stride,count,ctor,dtor), RET14h |

The vector iterator bodies were checked in assembly: BF7CD1 compares its signed
index against count, dispatches constructor through ECX and advances by stride;
BF7C6E computes the end pointer and predecrements count before reverse dispatch.
All eight direct calls in the four reconstructed functions have numeric
address/native rows in the report. These source functions expose a new C++ ABI.

AFD130's FH3 handlerCBB1BB uses descriptorDF3158 with mapDF3150, whose state0
actionCBB1B0 frees the newly allocated raw block. AFD220's handlerCBB1DB uses
descriptorDF3184/mapDF317C and cleanupCBB1D0. State0 is armed after allocation
returns. Allocation failure therefore neither clears the old header nor frees
it again. The real leaf constructors cannot throw for valid writable storage,
so their native iterator/outer partial-construction unwind has no C++ exception
path in this domain. Hardware-fault handling and the native FH3 ABI are not
reproduced by the source interface.

The worker made no Ghidra changes. Current AFD130/AFD220 bodies already include
the returning-free continuations and correct final RET4 bytes. The following
unowned EH evidence is supplied to the integrator for coordinated repair;
preserve existing unwind functions and comments.

| Existing or missing entry | Required inclusive end | Final instruction |
| --- | --- | --- |
| CBB1B0 existing unwind body endsCBB1B8 | CBB1BA | RET atCBB1BA length1; missing POP ECX/RET suffix |
| CBB1BB missing FH3 handler | CBB1C4 | JMP BF6B43 atCBB1C0 length5 |
| CBB1D0 existing unwind body endsCBB1D8 | CBB1DA | RET atCBB1DA length1; missing POP ECX/RET suffix |
| CBB1DB missing FH3 handler | CBB1E4 | JMP BF6B43 atCBB1E0 length5 |

All four routine spans and all four EH spans were compared byte-for-byte
between the verified `C:/Users/sqz269/bsp.gpr` program
`/battlestationspacific.exe` and the installed PE. Reported SHA256 values cover
the inclusive spans. The installed binary SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Strict MSVC Win32 `/std:c++20 /EHsc /MD /W4 /WX /O2 /Gy /fp:strict /permissive-`
compilation passed. One local probe executed relocated original byte leaves and
both complete array bodies against the new source:31 comparisons passed,
covering sentinel preservation, raw signaling-NaN/signed-zero constants, empty
and populated old storage, zero/equal/growing/shrinking counts, unsigned size
overflow, negative signed requests, and null allocator returns. The probe's
explicit malloc/free and vector-iterator fixture boundaries redirect the native
calls; the source uses the unchanged canonical cookie destructor code with
instrumented CRT symbols. Two additional source-only allocation-throw checks
passed, verifying free-before-allocation and the stale original header.
These do not execute the original FH3 exceptional path or prove native CRT
heap compatibility. The local probe and generated bytes remain under
`C:/Users/sqz269/bsp-am-arrays`; no permanent test suite was added. The combined
repository build is the primary integrator's separate verification. Full
AFD2E0/model construction and gameplay execution are not validated here.

## AM combined validation and saved analysis

The combined strict MSVC Win32 build and both existing seeded CTests passed.
Eight call reports check200 direct CALL rows without failures. All seven focused
replays pass within their documented boundaries. Saved names, native signatures,
full body ranges and old-comment preservation were read back; affected exports
were refreshed. The report embeds the earliest annotation preimages and repair
records. Earlier worker pending notes describe isolated snapshots. No original
exception ABI, complete application composition or gameplay claim is added.
