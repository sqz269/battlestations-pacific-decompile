# Font effect cache ownership

Addresses: 00a82250, 00b2e940, 00b2fa10, 00b31090, 00b31750, 00b31ff0, 00b32010, 00b320f0.

The renderer's material-effect call returns a caller-owned effect reference. A
new registry entry adopts the reference produced by its loader and adds another
reference for the caller. A hit adds a caller reference to the existing effect.
Aliases share one entry and one retained reference; separate entries may retain
the same fallback effect. Text-context invalidation releases the caller's
reference, leaving the registry's reference alive.

`include/bsp/effect_cache.hpp` and `src/effect_cache.cpp` implement the owning
fragment for the renderer's actual `0, 1, 1` argument combination. They use actual
effect-owner tokens supplied by the loader, the existing mutable VFS lookup, and
the established resource-name normalizer. They do not supply shader compilation,
renderer initialization, or scene state. Build and installed-probe integration
are recorded separately by the primary integration batch.

## Evidence and original ABI

`reports/font_effect_cache_ownership_audit.json` records fresh full-span hashes
against the installed executable, original ABIs, address-level proposals, and
explicit boundaries. Each Ghidra CLI batch verified project `bsp`, program
`/battlestationspacific.exe`, x86 language, and image base `00400000` before
reading. Project file is `C:/Users/sqz269/bsp.gpr`.

| Address | Complete matched span | Contract |
| --- | --- | --- |
| `00b31090` | `00b31090-00b315d9` | ECX registry; four stack arguments; EAX effect; RET10h |
| `00b318b0` | `00b318b0-00b319ad` | ECX renderer; name wrapper stack; EAX effect; RET4 |
| `00b2ebb0` | `00b2ebb0-00b2eed2` | Registry load slot+8; name/unused context stack; RET8 |
| `00b2e940` | `00b2e940-00b2eba0` | Resolver slot+4; hidden result/name/unused context stack; RETCh |
| `00b31ff0` | `00b31ff0-00b32004` | Retain slot+Ch; nonnull effect stack; EAX same pointer; RET4 |
| `00b32010` | `00b32010-00b3202e` | Release slot+10h; nonnull effect stack; RET4 |
| `00b301a0` | `00b301a0-00b3020e` | ECX registry+4 vector; source entry stack; RET4 |
| `00b2fd10` | `00b2fd10-00b2fdb1` | ECX destination entry; source entry stack; RET4 |
| `00b2fa10` | `00b2fa10-00b2fa87` | ECX entry; destroy alias/name storage; RET |
| `00b31750` | `00b31750-00b317b9` | ECX registry; reverse clear; RET |
| `00b320f0` | `00b320f0-00b3214e` | ECX registry; clear and release vector storage; RET |
| `00b32230` | `00b32230-00b3224d` | ECX registry; scalar-delete flags stack; RET4 |
| `00a82250` | `00a82250-00a82252` | Actual effect size slot: XOR EAX,EAX; RET |

Read-only context functions `00ab8ce0-00ab8e6c` and
`00ab8c30-00ab8cd4` were also matched. Vtables `00d5f074`, `00d5f04c`,
`00d61a00` and descriptor/fallback string bytes were matched separately.

The initial saved Ghidra body for `00b2fa10` stopped at `00b2fa48`, after
CALL `00b2fa44 -> 00bf65ac`. The saved body for `00b320f0` stopped at
`00b3213b`, after CALL `00b32137 -> 00bf6989`. The primary integrator's live
readback confirmed erroneous per-instruction `CALL_RETURN` overrides at both
call sites. Later analysis also reinstated an incorrect no-return flag on the
actual `_free`. The primary corrected that flag from its complete returning
assembly, cleared the audited overrides and rebuilt both complete bodies.
Final saved readback is recorded in `LIFETIME_EFFECT_CACHE_INTEGRATION.md`.
Full bytes, including subsequent string cleanup/SEH restoration and RET, were
matched and decoded with Capstone. These complete spans, rather than the
truncated pseudocode bodies, support the destructor conclusions. The evidence
packet records annotation proposals without modifying Ghidra. Leaf `00a82250`
initially had no Ghidra function definition.

## Request identity and resolution

Renderer `00b318b0` optionally enters its renderer guard, copies and ASCII
lowercases the requested name, and calls `00b31090` on renderer+1A98h.
`00b31090` calls platform helper `00beccd0`, copies the request, and normalizes it
with `00bee690`: ASCII lowercase, backslash to slash, and ASCII-space trim.
This does not invoke lexical slash/dot canonicalizer `00bee390`.

The registry stores entries of 2Ch bytes:

| Offset | Meaning |
| --- | --- |
| +0/+4 | Owned canonical native-string length/pointer |
| +8 | Alias-list object, with sentinel pointer at +Ch |
| +14h..+24h | Five DWORDs copied from VFS metadata query `00bdd340` |
| +28h | Retained effect pointer |

The first lookup walks every entry and each alias. It requires equal native
string lengths and then CRT `stricmp` equality. A nonnull match immediately
uses registry slot+Ch to retain for the caller (`00b3142a-00b31436`).

On a miss, resolver slot+4 is `00b2e940`. It constructs both the original name
and a copy with **every** case-sensitive `.mshd` substring replaced by `.shfx`.
It first asks `00bdf4c0` to resolve the changed name. If that succeeds, it returns
the changed/resolved name. Otherwise it resolves the original and returns its
mutated value even when that second lookup fails. This second lookup is an
identity-resolution fallback; it does not establish that the loader accepts
compiled `.mshd` data.

After normalizing the returned canonical name, if it differs from the request,
`00b3127b-00b31424` checks only the **first alias** of each entry. A canonical
match appends the request alias before retaining the effect. When creation is
necessary, the alias list begins with canonical and adds request if different.
There is no deduplication by effect pointer. Different failed names can therefore
have separate cache entries retaining the same `error.shfx` effect.

## The four stack arguments

| Stack argument | Renderer value | Established meaning |
| --- | --- | --- |
| 1 | Name wrapper | Request copied and normalized |
| 2 | 0 | Opaque context forwarded to resolver and loader; unused by both concrete callbacks |
| 3 | 1 | Retain newly loaded nonnull effect for caller |
| 4 | 1 | Permit creation after request/canonical misses |

Argument 4 is checked at `00b312eb`, after both lookup opportunities. False
returns null without loading/insertion. Argument 3 is checked at `00b314af`,
after insertion and accounting. It has no effect on a hit: hits always retain.
With argument 3 false, a new result is returned without an additional reference;
its loader reference has already been adopted by the cache. That generic
non-owning return combination is audited but not exposed by the owning host API.

## Reference transfers and teardown

Registry vtable `00d5f074` resolves retain slot+Ch to `00b31ff0` and release
slot+10h to `00b32010`. Retain calls `InterlockedIncrement(effect+4)` and returns
the same pointer. Release decrements that field and, at zero, invokes effect
virtual+0 using ECX effect with no stack argument. This is the game's intrusive
reference-count layout; it is not a COM `IUnknown` vtable interpretation.

On a new load, `00b3148a-00b3149a` inserts an entry containing the loader's
pointer. Entry-copy `00b2fd10` copies the effect DWORD at `00b2fd97-00b2fd9e`
without retaining. Temporary-entry destruction `00b2fa10` destroys alias/name
storage and never releases the effect. Thus the entry adopts the loader's
reference. The renderer's third argument then adds the caller reference.

The original effect's size virtual+Ch is queried after insertion. Teardown
`00b31750` walks entries in reverse, queries the size again and subtracts it,
releases the effect through registry+10h, destroys entry storage, and decrements
the vector count. Size accounting uses DWORD arithmetic. Material effect vtable
`00d61a00+Ch` points to `00a82250`, the matched three-byte zero-return leaf, so
this concrete material-effect registry accounts **zero** per effect. Supplying
zero for this concrete callback is supported by native code, not an estimate of
COM object or bytecode memory.

Registry destructor `00b320f0` installs base vtable `00d5f04c` before clear.
That base vtable uses the same retain/release slot functions. It clears and then
frees vector storage. Text-context `00ab8ce0` stores the renderer-returned effect
at +1ECh without another increment. On changed font name, `00ab8c30` decrements
and clears that field; the cache reference remains.

## Failed loads and recursion

Concrete loader `00b2ebb0` again rewrites all `.mshd` substrings and performs
mutable VFS resolution. If resolved, it allocates 178h, constructs the effect,
calls `00b46950`, and sets the original supplied name through `00b18f70`. It
does not inspect the variant-loader return value. A compilation failure is not
established as a cache fallback trigger.

The failure path emits a debug message and calls renderer virtual+48h with
`error.shfx` (`00b2ee27-00b2ee4a`). This recurses through the same cache wrapper.
The outer entry is inserted only after the callback returns, and there is no
pending-entry marker, recursion-depth check, or special self-fallback check.
If `error.shfx` cannot resolve, that path can recurse indefinitely. If it is
cached or loads, the nested caller reference becomes the outer cache entry's
reference, and the outer caller receives another reference.

Generic `00b31090` can insert a null loader result. Initial alias lookup skips
null matches and can retry them, but `00b31750` unconditionally dereferences
each effect to query its size. A null cache entry is therefore outside the
native clear path's valid assumptions. Allocation-null handling in the concrete
loader also invokes effect methods before the later pointer test; no graceful
out-of-memory behavior is claimed.

## Host API and remaining work

`EffectCache` accepts three explicit callbacks: actual mutable existing-name
resolution, actual owned-effect creation, and actual effect-size querying. It
implements the concrete resolver algorithm itself using the existing `.mshd`
rewrite helper. The owner is moved into an entry and copied for each returned
caller handle. `clear_00b31750_fragment` releases entry owners in reverse while
external caller handles remain valid. `entry()` exposes canonical/alias identity
for bounded inspection; those pointers are invalidated by vector growth/clear.

Host guards accept only NUL-free ASCII keys of at most INT32_MAX bytes, require
complete callbacks, and reject a null loader result before insertion. These are
explicit supported-domain limits; they do not redefine CRT locale/NUL identity
or native null-entry semantics. Calls are serialized. Resolver/load callbacks
may acquire recursively; size callbacks must not throw or reenter. No pending
sentinel or fallback recursion guard is invented. The actual effect loader must
own all shader/COM dependencies and may implement the native error-effect path.

The fragment leaves platform helper `00beccd0`, optional renderer guards, the
five opaque VFS metadata DWORDs, native vector allocation/exception guarantees,
descriptor compilation, reload/eviction paths beyond clear, and renderer/scene
lifetime external. Shared-pointer ownership is a new C++ interface, not a
drop-in binary replacement. Assembly/disk evidence does not establish game or
full-renderer runtime validation.
