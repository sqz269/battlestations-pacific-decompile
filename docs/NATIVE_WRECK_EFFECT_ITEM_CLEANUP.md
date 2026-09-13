# Native wreck effect item cleanup

Address: `008673B0`. This supplies the item cleanup required by X's actual
manager scan `008674C0`; it is a new C++ source interface, not an installed
runtime binding or completion of the whole wreck handler.

| Routine | Coverage | Original ABI |
|---|---|---|
| `008673B0..0086749D` | Complete 238-byte normal control and store sequence; canonical getter, raw-array erase and reference providers reused | ECX actual item, no stack arguments, plain RET, no defined result |

The view lends byte+09 and the two actual raw pointer/count/capacity headers at
+0C and +18 of the same item. No new owner, count, array, flag copy, registry or
lifetime domain is introduced. The application must supply a proven mapping to
these actual fields. Existing `PointEffectInstanceStorage` arrays contain
canonical companion pointers; they are not cast to this raw specialization.
Native constructor `008680B0` establishes EBP=ECX, EBX=0 and EDI=1. Its
`008680D9..00868192` stage publishes CEB130, count1, D0D3EC, writes byte+08 from
the argument, clears bytes+09/+0A, then clears both three-word headers in order.

The complete incoming-site set was read. `004661AA` supplies the current +44
pointer after a nonnull check; its caller clears that field after this returns.
`008674E0` supplies each selected current cell from the captured live manager
span; a null filter can forward a null entry. This routine has no null-item
recovery. An application mapping must not silently treat an unknown/null item
as a successful cleanup.

| Call site | Function | Callee and contract |
|---|---|---|
| `008673CE` | `008673B0` | Existing `00866440`, no inputs, EAX current lock owner; plain RET |
| `008673E7` | `008673B0` | IAT CE2218, EnterCriticalSection on captured owner+04 |
| `0086741A` | `008673B0` | IAT CE2220, InterlockedDecrement on captured child+04 |
| `0086742A` | `008673B0` | Current child virtual0, ECX captured child, no stack arguments |
| `00867467` | `008673B0` | `00867210`, ECX actual auxiliary header, one stack pointer to the fixed iterator cell, RET4 |
| `00867485` | `008673B0` | IAT CE2210, LeaveCriticalSection on the captured section |
| `00C94E63` | `00C94E60` | Tail jump to `00411EE0`, native state0 lock cleanup |

The getter's complete 189-byte body was read. Y calls its existing concrete
source with the caller's same publication and lifetime access. Its returned
owner must exist; a null section is valid. Y captures the canonical section
projection once, enters its real Win32 section, and increments that section's
actual DWORD+18. Callbacks can replace the publication or owner's section slot;
normal and C++ exceptional exits release the original captured section.

The first pass reads data+0C before count+10 and fixes the end. ESI walks the
captured slots; each current nonnull value is captured in EDI and decremented
before terminal lookup. The canonical actual reference helper validates that
the companion borrows that exact +04 atomic and requires current-profile
virtual0 behavior. After release returns, the actual cell is cleared once in
the nonnull branch and once unconditionally. Both stores remain volatile,
preserving callback writes being overwritten and the native two-store path.
The first header/count is not changed by the routine; callback changes remain.

The second pass reads current auxiliary data+18 before count+1C and captures
its first address. The native iterator cell never advances and `00867210`
does not overwrite it. After each removal, Y reloads count then backing for the
end test. Full 131-byte `00867210` and `0081B010` bodies are identical, including
the same Interlocked import operands, two current virtual0 sites, current-tail
reload after callbacks, and RET4. Y reuses existing
`erase_live_effect_reference_unordered_0081b010`; no library body is copied.
The owning array retains the incoming tail before releasing the replaced owner,
then releases/clears the freshly reloaded tail and decrements the current count.
Finally Y writes byte+09=1 before decrementing the captured lock depth/leaving.

The native EH handler C94E68, FuncInfo DC6CC8, one-entry unwind map DC6CC0,
cleanup C94E60 and complete 25-byte guard destructor 411EE0 are retained.
Native state0 is armed after the first span is captured. The cleanup decrements
the captured section's actual depth and calls LeaveCriticalSection. C++ tests
cover corresponding partial-state/lock effects on a binding failure; native
FH3/SEH transport and arbitrary throwing native destructors remain unproved.

The focused fixture uses canonical raw headers, actual context reference
storage and companions, canonical lock construction/getter/OS operations, and
one actual 28h manager for an X-to-Y source composition. Its physical item is
fixture input only; it does not create a production item owner or runtime map.
Eight seeds compare the copied 238-byte body plus copied 131-byte erase against
the source. The getter bridge returns the actual section from the real getter,
so the following three-byte owner-slot load is replaced by MOV EBP,EAX;NOP.
Each eight-byte virtual0 sequence is replaced by an explicit terminal capture
bridge. Original/replacement bytes and ABI register/stack effects are retained;
original dynamic dispatch/destructor bodies and native EH unwinding are not
executed. The copied body's normal EH setup/restoration does execute; the
stored original handler address is never invoked by the fixture.
The remaining native lock/depth, branch, array, count and field-store sequence
runs. Exact inputs, outputs, compiler/link inputs and results are recorded in
the report and local proof manifest. No gameplay or concurrency proof is claimed.
