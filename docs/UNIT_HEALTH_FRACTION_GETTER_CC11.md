# Unit health fraction and numeric getter (cc11_health_fraction_getter)

Addresses: `00876260`, `00923BE0`. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; the standard client verified the target before
read-only prototype/export/byte queries. No Ghidra or ledger mutation was made.

The two existing typed APIs now use explicit Win32 instruction kernels for the
unit health division and numeric getter floor/cap. They retain their existing
`UnitHealth` snapshot and release-flag interface. **They remain unit-specific
numeric projections: no native entity cache or arbitrary virtual provider is
represented by that interface.**

## Caller and provider boundary

Before the change, source inspection found `health_fraction_00876260` used only
by the typed getter, and `entity_health_00923be0` used by `set_health_00877b90`
when computing a host replication result. The two public wrappers now share the
private unit provider. The native setter reads the getter after its callback; the
typed setter still computes from its explicit snapshot before the host consumes
the result. This packet does not change that sequencing.

Native `00923BF7` calls the actual object's vtable slot `+110h`. Live bytes at
`00CFC4E0` are `60 62 87 00`, confirming `00876260` for the known unit vtable
`00CFC3D0`. The new private kernel explicitly specializes to that provider. It
does not replace arbitrary slot overrides or claim to cover all native callers.

The separate `lua_binding_get_hp_percentage` route still obtains its provider
through `LuaBindingMissionHost::unit_health_vtable_110_00923bf6` and writes cache
through `unit_health_cache_store_00923c16`. That API and its existing helper
`unit_health_value_00923be0` are untouched. The method-name suffix is historical;
the native indirect CALL itself is at `00923BF7`.

`UnitHealth` contains maximum/current/invincibility values, not native `+164h`
cache storage or an entity pointer. The new kernels neither add a cache member
nor invent a mutable cache outside the object. Actual provider dispatch and
cache integration remain required work for a native entity binding.

## Recovered numeric instructions

`00876260..00876274` is the seven-instruction sequence:

```text
PUSH ECX
FLD  float [ECX+370h]
FDIV float [ECX+36Ch]
FSTP float [ESP]
FLD  float [ESP]
POP ECX
RET
```

It has no zero guard. Division uses ambient x87 precision/rounding, then rounds
through the explicit binary32 store. The private `unit_health_fraction_x87`
retains that sequence, adapting field offsets to the typed snapshot's current
at 4 and maximum at 0. Compile-time offset assertions guard this mapping.
Its private calling convention is ECX snapshot pointer, ST0 result. Public
signatures and object layouts do not change; this is not native entity ABI.

The numeric getter kernel preserves the following order:

| Native sites | Numeric operation |
| --- | --- |
| `00923BE4..00923BEE` | If released, return `FLDZ` positive zero before provider calculation; native cache stays untouched. |
| `00923BF7..00923BF9` | Obtain the known unit provider's ST0 result and spill it to float again. |
| `00923BFD..00923C07` | Load result, load zero, `FCOMIP 0,result`, pop result, `JBE` past floor for equal/positive/unordered. |
| `00923C09..00923C12` | Ordered negative input selects positive zero and loads it for return. |
| `00923C21..00923C34` | `MOVSS` raw result and float 1.0, `COMISS result,1`, `JBE` keeps equal/lower/unordered; only ordered greater selects 1. |
| `00923C37..00923C4B` | Store/reload chosen float and return ST0. |

The native writes at `00923C16` and `00923C41` store the selected float bits to
entity `+164h`. They are deliberately not performed by this snapshot API. The
release branch performs neither a provider calculation nor a cache write. The
ceiling at `00D7A24C` was reread as `00 00 80 3f`, float 1.0.

NaNs survive both numeric clamps under masked exceptions. The original x87
load/divide/spill determines quieting and payload selection. Signed negative
zero remains negative zero because it is not ordered less than zero. Positive
infinity caps to 1; negative infinity floors to positive zero. Division by zero
and invalid division remain hardware operations, without a substituted policy.

## Evidence and focused validation

Before the change, this compiler happened to emit x87 division/spill for the
standalone C++ fraction function, which passed all 192 fraction comparisons.
The getter's inlined division emitted `DIVSS`, however, and the getter failed
73 of 192 cases. The new private naked kernels prevent that context-dependent
instruction choice. Generated assembly shows public wrappers tail-jumping to
the kernels, the host result paths calling the numeric kernel, and the retained
x87 division/spills and mixed x87/SSE clamp sequence.

Examples at masked PC24 nearest:

| Case | Old getter bits | Native / corrected getter bits |
| --- | --- | --- |
| Current 200 / maximum 100 | `40000000` (2) | `3f800000` (1) |
| Current -100 / maximum 100 | `bf800000` (-1) | `00000000` (+0) |
| Current bits `00800004` / maximum bits `40000001` | `00400001` | `00400002` |
| Current qNaN `7fc12345` / maximum qNaN `7fc54321` | `7fc12345` | `7fc54321` |

The subnormal witness exercises division rounding followed by the separate
float-store rounding. The payload witness exercises the actual x87 operand
behavior; the implementation does not synthesize a generic NaN propagation rule.

The prior single health probe was reused, with only 16 fraction/getter input
pairs added across PC24/53/64 and four rounding modes. Cases cover floor/cap,
one-third, a division/spill boundary, zero denominators, signed zero,
underflow/overflow, infinities, and quiet/signaling NaNs in both operands.

- 192 fraction, 192 numeric getter and 192 released-result comparisons pass.
  The previous getter failure count was 73; it is now zero.
- 576 added state checks preserve the complete x87 control word, MXCSR control
  bits, TOP and two preloaded exact sentinels (11.25 and -3.5). The caller
  consumes each ST0 return before the stack check. Released calls return positive
  zero and set no FP exception flags even for the invalid-division/NaN inputs.
- The existing 336 prefix/marker, 672 client/host data-gate, 240 byte, and 336
  setter control/stack checks still pass. The reference getter's cache capture
  is part of the probe only, not newly added production cache state.

Command: `cmd /c local\cc11_health_fraction_getter_check.cmd`. MSVC Win32
`/std:c++17 /EHsc /O2 /Gy /W4 /WX /fp:strict`, linked with
`/MANIFEST:EMBED /OPT:REF`. The before run exits 1 on the exposed getter gaps;
the corrected run exits 0. Logs, generated before/after assembly and executable
hashes are in `reports/unit_health_fraction_getter_cc11.json`.

## Remaining limits

The numeric matrix uses all exceptions masked, DAZ/FTZ disabled, and matching
x87/MXCSR rounding. It does not establish all binary32 combinations, independent
rounding domains, unmasked trap/resume behavior, full status/FIP/FDP parity or
native ABI compatibility. The kernels do not install a control word or clear
exception state; they require adequate x87 stack space and leave one ST0 result
for the caller to consume. Overflow/underflow of the x87 stack was not tested.

Full native getter binding still needs the real release field, actual virtual
provider, float cache write and their object lifetime/timing. Setter callback
ordering/ownership, finite byte-conversion overflow behavior and the generic
Lua getter's separate implementation remain outside this packet. No policy or
host switch, callback/setter flow, tracked test suite or CMake file was changed.
Full build/CTest belongs to the primary; no game run was requested or performed.
