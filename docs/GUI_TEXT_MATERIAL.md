# Actual Text shader and material operations

`gui_text_material.cpp` supplies the actual-storage material operations used by
the nonempty `00ABA8D0` stages. These consume the existing canonical shader,
material, parameter pool and native string owners. They create no clone-state
snapshot, texture wrapper, parameter cache or second Text shader slot.

| Entry | Native ABI and inclusive body | Implemented domain |
| --- | --- | --- |
| `00AB8CE0` | ECX Text; no stack arguments; AL attempted; final RET at `00AB8E6C`, length1 | Full supported normal selection/cache path. Current callable renderer48 must return its actual owned effect reference. Native Text string/SEH ABI remains external. |
| `00B19210` | ECX actual material; shader pointer stack; RET4 at `00B192C6`, length3; end `00B192C8` | Complete valid-storage sequence, including equal shader and nonpositive parameter count. |
| `00B189F0` | ECX actual material; unsigned index and texture pointer stack; RET8 at `00B18A3D`, length3; end `00B18A3F` | Complete array indices0..8/count0..9. Corrupt extents are explicitly rejected. |
| `00B18AA0` | ECX actual material; name/source stack; RET8 at `00B18AB3`, length3; end `00B18AB5` | Full wrapper of actual `00B17E10`, count4, matrix byte0; preserves returned parameter pointer. |
| `00B18B00` | Same, RET8 at `00B18B13`, length3; end `00B18B15` | Full wrapper, count2, matrix byte0. |
| `00B18B20` | Same, RET8 at `00B18B33`, length3; end `00B18B35` | Full wrapper, count1, matrix byte0. |

All complete bodies were read, including every register occurrence in the
assembly; no bytes in these six entries remain unread. Names are descriptive
hypotheses. Their new C++ interfaces are not drop-in native ABI replacements.
The Ghidra wrappers checked `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` for every batch; no Ghidra writes were performed.

## Shader selection and publication

The only direct caller of `00AB8CE0` is `00ABA9E4` in `00ABA8D0`. It saves AL
at stack+3C (`00ABA9EB`) and uses it to gate both material-registration arms;
AL must not be interpreted as shader-load success. A nonnull existing Text+1EC
returns false without inspecting names, configuration or font. Otherwise a
nonempty override name calls the current renderer48 and publishes its returned
owned reference, including null, then returns true. The existing lifetime's
same raw slot owns this reference; no added retain occurs on publication.

With an empty override, read the current configuration pointer `0109CF04`,
convert its signed DWORD+28 with CVTSI2SS and compare to float720 (`D5C56C`).
Only an ordered equality reads current font+18 and compares it to float1
(`D7A24C`). Two matches select `GuiFont.mshd`; every other comparison selects
`GuiFontBilinear.mshd`. A height mismatch must not read a missing font.

Each default name uses actual `NativeString` allocation and cleanup. The
returned shader is stored at Text+1EC **before** temporary cleanup at both
default-name sites. The override passes a borrowed eight-byte length/data
header over the existing string bytes, adding no allocation. Actual renderer48
body `00B318B0` was read: it copies the supplied name before lowercasing its own
copy and asking its retained cache, and does not retain/mutate the supplied
header. Its cache/load/resource implementation remains the required current
callable factory, also used by the retained renderer infrastructure; the
original numeric code address is not called in a reconstructed process.

Full register filtering proves ESI=entry ECX from `00AB8CF9`; EDI is the
existing name header at Text+1C0 (`00AB8D1C`); BL carries the nonempty result
from `00AB8D2A`; no EBP input exists. Return paths restore saved registers and
write AL independently of the shader pointer. The empty comparison wrapper
has a null buffer and no allocation, so the C++ empty test adds no pool event.

## Actual material state

`00B19210` compares effect identity, publishes a changed incoming pointer,
retains its actual atomic+04, then releases the captured old effect through
the canonical actual-owner domain. Its terminal virtual0 may reenter. It then
reloads material+7C and sets the **current** effect's byte+B4. Even for equal
effect identities it clears parameters: capture each live slot, destroy its
native name, return the same slot to the actual F8D3E4 pool, reload count after
each return, and finally set count+100 to zero. Existing slot cells stay stale;
null slots are skipped. A negative native count also reaches zero.

The consumed `00B17AF0` pool-return body (`..00B17B57`, RET4 at `00B17B55`,
length3) was read in full and matches the existing parameter pool's inline
`00B193FA` return implementation: same critical section/recursion state,
88h stride, slot+84 slab index, +4400 free words, +4500 count and +34 scan
cursor. The helper calls that existing concrete pool implementation. It does
not free parameter storage with a generic C++ delete or invent a pool adapter.

Register provenance: EDI=entry material at `00B19217`; ESI first captures
old effect at `00B19219`, then captures each parameter at `00B19270`; EBX is
the zero/index value from `00B1924E`; EBP addresses material+80 at `00B19264`
and advances four bytes per visited slot. Count is re-read at `00B192A4`.

`00B189F0` first sign-extends count+34, compares its bits unsigned to the
incoming index, and grows the lowword count before testing texture identity.
Within the supported valid array/count domain, changed slots publish first,
retain incoming, then release captured old. It never clears intervening slots.
ESI captures old texture at `00B18A08`; EDX is the incoming index, EAX the
incoming texture after `00B18A03`. All inputs come from ECX and the two stack
words; RET8 establishes arity independently of caller push windows.

The float wrappers push matrix0 and counts4/2/1, then source and name, and
call actual `00B17E10` at `00B18AAE`, `00B18B0E`, `00B18B2E`. Native parameter
records borrow source addresses and own their names; no float values are copied
into an auxiliary dictionary. Existing B17E10/B44D60 resolves actual effect
metadata, updates matching records and preserves absent-selector behavior.

## Caller evidence and verification

All 311 material-family direct call sites within 83 live Ghidra bodies were
inspected using full saved listings and their filtered call setup. They cover
texture null/equal cases, fixed indices0..8 and register indices; the generic
leaf is not specialized to Text's slot0. Float callers borrow widget/global,
material and stack fields. The JSON lists their exact call address, target and
live containing function. All four shader-setter sites were included; the
non-Text callers are `0093D611` in `0093D2D0` and `00B9FCAA` in `00B9FA60`.

Eight additional xrefs have no live Ghidra containing function. Raw disk
instruction boundaries establish separate bodies, not the preceding candidate:

| Raw body (inclusive) | Final instruction | Material calls |
| --- | --- | --- |
| `00ABCC70..00ABCECB` | RET at `00ABCECB`, length1 | `00ABCD4D`, `00ABCD9D`, `00ABCDED`, `00ABCE3D` |
| `00ABF770..00AC0272` | RET at `00AC0272`, length1 | `00AC01A6`, `00AC01DD` |
| `00B54CE0..00B54E69` | RET at `00B54E69`, length1 | `00B54DA1`, `00B54E45` |

These rows are explicitly `no_ghidra_function`; the direct-call checker cannot
verify them, and they are listed as raw evidence separately from checked call
rows. Their argument setup was read, with the receiver provenance followed
through the whole filtered body. None is attributed to ABCA10, ABF6F0 or
B54CD0. The first body also contains `00ABCE7D -> 00AA9F10`, independently
confirmed by the integrator; this is not another material-family count.

MSVC Win32 C++17 compilation with `/W4 /WX /EHsc /permissive-` passed. The
strong integrator call-site verifier and ordinary worker checker both passed:
326 direct rows, zero failures; nine current-slot/IAT rows have separate
assembly evidence. Eight raw orphan material calls remain outside the checker.
The final compile used local copies of the two changed public headers first,
then integration headers and peer dependency headers, so the same canonical
raw shadow-enable byte was compiled. No tests were added. The
new functions are not wired into `bsp_game.exe`, and no executable-path, game,
rendering or ABI-equivalence validation is claimed. Combined registration and
build remain with the integrator.
