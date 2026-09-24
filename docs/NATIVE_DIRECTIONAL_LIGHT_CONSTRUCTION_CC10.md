# Raw directional-light construction

This packet reconstructs the complete normal bodies of two constructors over
the same actual slot and raw pooled-name domain. It adds no allocation,
terminal owner, registry admission, or application camera-store acquisition.

| Address range | Bytes | Source entry | Coverage |
| --- | ---: | --- | --- |
| B7C4C0..B7C575 | 182 | `construct_native_light_raw_00b7c4c0` | Complete |
| B7C6B0..B7C6C8 | 25 | `construct_native_directional_light_raw_00b7c6b0` | Complete |

Native ABI: ECX is the actual slot, one stack pointer names the actual eight-byte
string header, EAX returns the same slot, RET4. The final instructions are
B7C573/B7C6C6, each three bytes. The new C++ interfaces explicitly borrow the
raw string-pool context, current node constants, and current CE7820 cell.
They require an aligned caller-owned extent covering the actual 1F0h slot.

| Containing body | Call site | Native callee | Source binding |
| --- | --- | --- | --- |
| B7C4C0 | B7C4C8 | B6F5A0 | Existing genuine raw-name node constructor |
| B7C6B0 | B7C6B8 | B7C4C0 | New raw light constructor |

The node constructor establishes the actual 174h prefix, including its single
count at +4 and raw name at +54. The light wrapper borrows that same prefix;
it writes its tail through raw instructions and adds no host member or second
count. The older `directional_light_owner.cpp` semantic-name overloads remain
available separately.

After B6F5A0 returns, B7C4CD captures the current D7A24C DWORD once with MOVSS.
It clears XMM1, stamps D62F58, zeros +174/+178/+17C/+180, then writes the
captured word at +184/+188/+18C/+190 and +1A4/+1A8/+1AC/+1B0. It zeros
+1C4/+1C8/+1CC before reading current CE7820 at B7C550. It then writes the
captured one to +1D0, CE7820 to +1D4, and the same one to +1D8. The derived
wrapper only stamps D62FB0 after successful base return. These instructions
preserve float bit patterns, including signaling NaNs and negative zero.

The allocation bytes at +194..1A3, +1B4..1C3, +1DC..1EB and pool slab ID
+1EC..1EF remain untouched. The existing raw node constructor retains its own
documented prefix preimages. Constant cells and the raw name may alias
accessible actual storage; their addresses and the context must stay valid.
Private source frame/argument storage must not alias the written slot.

Neither wrapper has an EH handler or an additional ownership cleanup state.
B6F5A0 performs its existing source prefix cleanup if construction throws.
Physical slot recovery belongs to the caller: AC59A0's construction-only
state11 returns the actual directional slot through B7B610, and disarms after
successful B7C6B0. This packet does not duplicate that return or claim native
FH3, CRT exception identity, arbitrary fault observation, or outer binary ABI.

The existing DirectionalLightPool borrows actual owner01090154 and allocates
1F0h slots with slab metadata at1EC. The raw prefix uses NativeStringRawPoolContext
and NativeNodeRawConstants; it never switches to SizedStoragePool. There is
no need for a new pool or semantic constructor mode. Runtime views and terminal
lighting/scene dispatch still require separately established actual identities.

Strict `scripts/build.ps1` passed, including both existing CTests. Live Ghidra
bytes match the installed PE for all207 constructor bytes; the two direct call
rows are mechanically checked. Ghidra remained read-only.

One ignored probe compares two original/source pairs using a genuine actual
directional-pool slot and raw8AD4A0h string pool. Original wrapper bytes call
the same genuine source B6F5A0 provider; that callee is an explicit shared
boundary, not an original native node-constructor comparison. The probe checks
all1F0 bytes with only the independently checked name-data pointer normalized,
actual name length/content, preserved tail and slab ID, and x87/MXCSR state.
The base pair uses signaling-NaN one and negative-zero CE7820. The derived
pair aliases one to +138 and CE7820 to +1C4, verifying the post-base value40h
and the post-zero value0. The actual raw input header is at untouched+194.
NDEBUG is rejected at compile time; exact `/MD /fp:strict /MANIFEST:EMBED`
build command and output are retained in the report's ignored evidence paths.

AC59A0 remains unadmitted as raw composition. Its required3Ch scene resource,
28h registry and actual ambient terminal/attachment domain are distinct missing
providers. The read-only handoff is `local/cc10_page_acquire_readiness.md`, with
the1467B page listing, current/captured schedule and14-state native unwind map
under `local/output/cc10_page_acquire/`. The existing raw98h B7C290 constructor
is reusable, but enlarged ConcreteSystemAmbientLight and host-layout
ConcreteSystemSceneResource cannot replace native allocations. This packet
does not claim those owners, complete page acquisition, application binding,
terminal lifetime equivalence, or gameplay parity.
