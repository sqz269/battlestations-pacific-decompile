# Native wreck effect release providers

Addresses: `008674C0`, `00484620`. These are new C++ interfaces over actual
existing owners. They close two required provider bodies used by the bounded
`00824F39..00824FE4` wreck fragment; they do not complete `00824B60` or install
a game runtime binding.

| Routine | Coverage | Original ABI |
|---|---|---|
| `008674C0..008674F1` | Complete 50-byte normal control sequence | ECX actual manager; one stack filter; RET4; no defined result |
| `00484620..0048465F` | Complete 64-byte normal sequence, canonical existing provider reuse within the required nonthrowing reference domain | ECX actual pointer cell; one stack replacement; EAX same cell; RET4 |

`008674C0` reads count+14 first, then data+10, and forms a captured end using
DWORD arithmetic. ESI is the current address, EDI the fixed end, and EBX the
filter. A null filter visits every current entry, including null. A nonnull
filter compares the current cell, then reloads that cell for the call. After
the callback it advances the captured address by four. Header mutations do
not resize this walk; modifications to later cells are observed. The original
backing must stay live until the walk ends. There is no empty-owner recovery,
range sanitization, null-entry skip, or queue cleanup invented by this provider.

The actual `NativeLiveEffectManagerStorage` and its pointer array header already
exist. Producer `004CF700` publishes CE789C, creates the existing list sentinel,
publishes it, clears count, then clears both pointer/count/capacity headers.
X adds no owner, allocator, manager singleton, or authoritative count.

| Call site | Containing function | Required callee and evidence |
|---|---|---|
| `008674E0` | `008674C0` | `008673B0`, ECX freshly loaded entry, no stack arguments, plain RET |
| `0048462E` | `00484620` | IAT `00CE2220`, `KERNEL32.dll!InterlockedDecrement`, old owner+4; stdcall one argument |
| `0048463E` | `00484620` | Current old owner's virtual0, ECX captured owner, no stack arguments |

All 238 bytes of `008673B0` were read before the address-named contract. It
captures the `00866440` lock, releases/clears the child pointer span at +0C/+10,
removes auxiliary entries through `00867210`, writes byte+09=1, and releases
the captured lock. It carries native EH state. The superficially similar
`00867B10` has different predicates/fields/calls and is not substituted. The
required `NativeWreckEffectStopAccess::call_008673b0` has no default body.

All 64 bytes of `00484620` are identical to `0054D510` and `0042D9A0`, including
the same absolute IAT operand, internal branches, current virtual0 load/call,
and both RET4 exits. There are no relative calls or native EH records in these
leaf bodies. X calls the existing generic `adopt_sound_reference_0054d510`
provider. Root commit `4286736f` added a volatile-reference overload containing
the same canonical body; the existing nonvolatile overload adds qualification
and delegates. No qualification is stripped and no library body is copied.

The X adapter delegates reference release to existing
`release_native_render_actual_owner`. It decrements the captured owner's actual
aligned atomic at +04 with sequential consistency, then resolves the canonical
companion only when the new count is zero. Signed atomic fetch-sub has defined
two's-complement arithmetic, including zero to -1 and INT_MIN to INT_MAX. The
companion must borrow that exact atomic and dispatch the current native profile.
The destination remains a volatile pointer lvalue across the canonical call;
it is cleared after release returns, then overwritten with the replacement.
There is no incoming retain or same-pointer shortcut. Volatile accesses retain
the cell access schedule; they do not make racing pointer writes synchronized.

The `VoiceReferenceHost` domain requires a nonthrowing terminal callback. A
missing owner/profile or mismatched count is an invalid required binding, and
its exception terminates through that existing noexcept interface. X does not
claim equivalent native exception transport for such failures or a throwing
native destructor. No unknown owner is treated as successfully released.

The fixture uses canonical 28h manager construction and actual 18h context
storage initialized by `00B1EDD3..00B1EDF1`, with companions borrowing its actual
+04 count. The retained tables D5E5C4 and CEB130 both select BD30E0 at virtual0;
its complete 14-byte body calls current virtual+04 with literal1 (deleting
destructor). The table-specific targets remain observed fixture boundaries.
No native destructor or invented native vtable is executed by this fixture.

Verification and exact retained artifact hashes are in
`reports/native_wreck_effect_providers.json` and the local proof manifest.
The fixture compares eight seeds for each provider. Its scan copy relocates
only the direct required call. Its adoption copy relocates the IAT operand and
replaces `00484638..0048463F` (eight bytes) with a virtual-dispatch capture
bridge. That bridge is a fixture contract, not execution of the original
dynamic dispatch/destructor. The remaining native decrement, branches, stack
cleanup, return cell, and callback-following stores execute. The separate
static proof covers complete byte identity of the original 64-byte aliases.
The bridge loads ECX from saved EDI, calls a fastcall no-stack-argument bridge,
and pads with NOP; ESI/EDI and the native stack are preserved. EAX/EDX/flags are
caller-saved and not consumed after this boundary.

No full wreck behavior, original ABI replacement, native EH transport,
concurrent pointer mutation, or installed gameplay validation is claimed.
