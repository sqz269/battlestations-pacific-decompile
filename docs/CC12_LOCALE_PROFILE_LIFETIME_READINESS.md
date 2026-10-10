# Locale profile lifetime readiness

The first DWORD of profile `00CFF140` selects scalar deleting entry
**`0073CAF0`**, which always calls destructor **`0073C9F0`** before optionally
freeing its captured receiver. The destructor's complete normal body is now
bounded. Its final `007365E0` unregister call and exception-map actions remain
unopened dependencies, so this packet does **not** close the complete lifetime
contract or authorize application activation.

Only three Native spans were admitted: the four-byte profile slot, the 30-byte
selected entry, and its one additional 251-byte destructor. No other profile
slot, callee body, EH handler or cleanup map was opened. No C++, Ghidra mutation,
build, CTest, probe or Original runtime execution was performed.

## Selected scalar entry and listing gaps

The four bytes at `00CFF140` are `F0 CA 73 00`. Typed metadata identifies
`0073CAF0..0073CB0D`; the full disk/live span is 30 bytes and decodes to eleven
instructions. Its schedule is:

1. Save `ESI`, then capture incoming `ECX` in `ESI`.
2. Call `0073C9F0` on the same receiver.
3. After the destructor returns, test bit zero of the flag slot's low byte at
   `[ESP+8]`. This is a **four-byte stack argument**, as the final `RET 4` shows;
   a future Source interface should retain `uint32_t` flags rather than truncate
   the argument to the decompiler's inferred `byte`.
4. If the bit is set, call `00BF65AC` on the captured `ESI` and discard the pushed
   argument. Other flag bits do not select another path in this complete body.
5. In both cases, set `EAX=ESI`, restore `ESI`, and execute `RET 4`.

Thus the scalar result is the captured receiver value even after its storage
has been freed. It is not the free function's return. No flags are forwarded to
the destructor. A destructor exception does not reach the scalar's later free;
the scalar body has no local EH setup. Original arbitrary stack aliasing and SEH
are separate from an added C++ parameter interface.

Ghidra's current scalar listing has ten instructions because it omits the three
bytes `83 C4 04` at `0073CB05` (`ADD ESP,4`). The destructor listing similarly
omits that instruction at `0073CA2F`, `0073CA54`, and `0073CABE`. All four missing
instructions are present in both fresh live bytes and the configured original
PE. The scalar pseudocode's `extraout_EAX` return and destructor pseudocode's
premature returns after `free` are therefore not used as the control-flow model.
Complete byte decoding supplies eleven and 73 instructions respectively,
compared with Ghidra's ten and 70. These are retained analysis qualifications;
the saved listing and function metadata were not repaired.

Fresh typed flow metadata at all four callsites, captured at unchanged program
modification 55, records `CALL_RETURN` overrides and effective
`CALL_TERMINATOR` flow. Default/effective fallthrough are null and the separate
`fallthrough_overridden` flag is false. The `_free` thunk at `00BF65AC` and its
direct target `00BF9DC8` both have `no_return=false`. The stored scalar body is
27 addresses in two ranges; the stored destructor body is 242 addresses in four
ranges. Their holes exactly match the four three-byte instructions above. This
qualifies current metadata without attributing the gap to a no-return flag or
claiming to recover the historical cause. Only target metadata was read; no
additional callee body was opened.

## Destructor normal order

`0073C9F0..0073CAEA` is 251 bytes, below the admitted 512-byte expansion limit.
It captures `ECX` in `ESI`, stores a receiver spill at `[ESP+0Ch]`, writes profile
`00CFF140`, and preserves `EBX/ESI/EDI`. The body has no stack arguments and ends
in plain `RET`; it does not establish a meaningful EAX return contract.

| Order | Recovered receiver operations |
|---|---|
| 1 | Arm EH state 2. Capture `this+4034h`; if nonnull, free that buffer. Clear `+4034,+4038,+403C`. |
| 2 | Capture `this+4024h`; if nonnull, free that buffer. Clear `+4024,+4028,+402C`. |
| 3 | Capture language data at `+401Ch`; if nonnull, read the current length at `+4018h`, add one with DWORD arithmetic, and push factor `1`, captured size and pointer. Call `00419CC0`, then pass its returned pool in `ECX` to `00BD1510`. |
| 4 | Set `ECX=this+14h`, arm EH state 1, and call `0073C240`. |
| 5 | Read current table-name first pointer at `this+8h`; form `EDI=this+4h`, arm state 0, and skip range release if first is null. |
| 6 | Otherwise reload the saved receiver spill, read end from `[EDI+8]` into `EDX`, and call `00432050` with `ECX=first` and the two shown stack arguments (receiver spill and `EDI`). |
| 7 | **After the range call, reload `[EDI+4]`**, then free that current pointer. This is not necessarily the previously captured range beginning under mutation. |
| 8 | Clear `[EDI+4,+8,+C]`, set state `-1`, and call **`007365E0`** with `ECX` set from the captured `ESI`. Restore the EH link and return. |

The body does not explicitly clear the language size/data header after returning
its buffer. It does not write the allocator words at `+4`, `+4020` or `+4030`.
It contains no direct access to publication `00F8BC4C`; any unregister or
publication effect must be established from the unopened `007365E0`, not inferred
from its existing descriptive name. Provider side effects and mutations are not
erased by the local field inventory.

The visible handler address is `00C8696E`. The normal body visibly moves through
states 2, 1, 0 and -1, but the FuncInfo, unwind map and cleanup action bodies were
outside this packet. Consequently no cleanup action, retry behavior, publication
repair, or destructor exception guarantee is supplied here. The generic semantic
meaning of the six named callees is a dependency, not fresh whole-callee proof.

## Current Source lifetime comparison

The exact `4040h` `LocaleTableManagerImage` remains in `gui_startup.hpp`; its
derived-field initializer is still dormant and does not implement base
publication/registration. The accepted preceding constructor/base contract is
retained as a **pin dependency**, including two actual manager lookups,
publication reload and captured-section cleanup without publication repair.
Eight relevant Source files are byte-identical to that accepted packet. Its
old Native proof and recursive Root review tree are not replayed or copied here.

The application still owns a semantic `GameLocaleHost` of compiled size `4064h`.
Its ordinary destructor releases the lazy GUI owner and `LocaleTables`. Its
compiler-generated scalar delete performs the same semantic destruction and
sized C++ delete; it does not invoke either recovered Native locale entry.
`LocaleTables` owns standard vectors, strings and `unique_ptr` nodes. Its
`clear_0073c240` clears that semantic storage and zeroes its bucket projection;
it is not a raw call on the native map subobject at `this+14h`. Application
destruction deletes the semantic locale after the shared raw singleton drain.

Relevant existing building blocks are distinct from complete locale lifetime:

| Current Source | What remains for this locale owner |
|---|---|
| Actual `00419CC0` pool getter and `00BD1510` return helper | Bind the same actual pool publication, return gate and manager; preserve captured language pointer/size across the getter. No locale binding is added here. |
| `destroy_global_config_name_range_00432050` | Existing source interface walks eight-byte string headers through `NativeStringStorage`; its `noexcept` and storage interface are not proof of a locale-specific raw exception/context contract. |
| Semantic `LocaleTables::clear_0073c240` | Actual raw chained-node/map cleanup still needs a separately bounded review and implementation contract. |
| Raw singleton deletion dispatcher | No admitted `00CFF140` locale profile case or locale context exists. Its fallback rejects unsupported profiles. |

The complete frozen Core archive and 70 application symbol indices contain no
address-named implementation of `0073CAF0`, `0073C9F0`, or `007365E0`. This is an
exact symbol-inventory observation, not a claim that an address string alone
proves behavior. Relevant source bodies, whole COFF objects and relocations are
also retained and compared.

Seven complete selected COFF sections are reviewed:

| Source section | Bytes |
|---|---:|
| Semantic `GameLocaleHost` ordinary destructor | 23 |
| Semantic `GameLocaleHost` scalar delete | 52 |
| `LocaleTables` destructor | 143 |
| `GameStartupHost` destructor normal body | 1831 |
| Raw singleton profile dispatcher, complete section | 3545 |
| Existing `00432050` range helper | 41 |
| Semantic `0073C240` clear | 57 |

Six sections are fully decoded as code. The dispatcher's complete 3545-byte
section contains 3280 bytes of code/alignment, 259 bytes of labeled switch data
and six bytes of alignment between those tables. The replay verifies all six
table labels, their code references, complete target relocations, index bounds
and all section bytes. It does not decode those tables as instructions or use a
code-prefix match as whole-section evidence. Source unwind funclets and Native
unwind actions are separate from the selected normal code sections.

## Evidence and next bounded work

The current epoch is **`Source3352-particle-model-typed-views`**, frozen under
Root's `local/cc12_particle_model_view_Source_primary`. All 3352 inputs and four
artifacts match that freeze and the live integrator at capture. All 3352 Git
blobs match the worker; 3345 match Source bytes exactly, with seven retained
CRLF/LF-only differences. Those cohorts are separate. The current Core archive
has 1991 entries and 1988 COFF objects; all their full symbol indices and all 70
frozen App objects are reparsed during replay. Retained prior test logs are
artifacts, not new test execution.

Fresh typed `bsp.py` calls verified project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base before every query,
using the configured `C:/Users/sqz269/bsp.gpr`. Each of the three admitted Native
spans also matches original PE disk bytes. No live gameplay or binary ABI claim
follows from that equality.

Bundle: `local/cc12_locale_profile_lifetime_readiness/evidence.zip`,
42,875,668 bytes, SHA-256
`7e46c1147721b72c74a650dff3f8b8723d06772ec2d33beae518549d0ec550d4`.
Both guarded ZIP-only replay and replay from a separate physical copy pass.
The guard rejects an intentional external Source read and forbids process/network
operations after runtime imports. Replay validates retained evidence, not later
live checkout state.

The smallest next read-only packet is the single **`007365E0` unregister body**:
recover its actual receiver, manager lookup/removal, publication and profile
ordering, plus only separately authorized cleanup evidence. After that, bound
the `0073C240` raw map dependency and the destructor's EH actions. Existing raw
pool and range providers need explicit compatible locale context and exception
contracts before composition.

The eventual dormant lifetime implementation would accept a captured actual
`LocaleTableManagerImage` and full flags, preserve the normal-body ordering and
post-range pointer reload, and compose those completed dependencies before
adding the scalar wrapper. A wrapper around an unresolved destructor would not
complete this frontier. No application activation GO, complete lifetime parity,
fixture, ABI or runtime validation is claimed.
