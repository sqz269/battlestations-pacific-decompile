# Native player-unit killed tail

`native_unit_on_killed_00779af0` reconstructs the complete normal body
`00779AF0..00779B5C` (109 bytes). Native ECX is the unit, there are no stack
arguments, and EBX/ESI/EDI are restored before the final jump to `00928C80`.
This source interface is not a binary replacement.

The view borrows the existing `NativeUnitObserverAlias` and three actual recon
records. Existing `recon_slot_lists.hpp` supplies their `+1E8h` base, `34h` stride,
count and field offsets. Constructor `0077EED0` supplies three `34h` elements to
the existing library array constructor at `0077EF22`, using `00805A70` as their
constructor. Its `00805AB3..BE` stores zero to normal level `+4h`, forced level
`+8h` and force byte `+10h`. `00805AF0` subsequently writes the normal level.
The view adds references, without a replacement owner or native layout cast.

The exact order is:

1. Query the current primary slot `+18h`. When nonnull, reload the primary table,
   capture the actual `00E188DC` publication, then query again. Compare the second
   result with the captured value, even if that call changes the publication.
2. On equality, call `004BCA80` with ECX zero. Its complete provider must publish
   that value and invoke `00B0D7B0` on current `00F8D39C`, including listener work.
3. Read current `00E188A8` after that callback and clear its `+193Ch` byte. This
   is the existing unit-list latch, not a new world owner or cached bool.
4. Visit all three live records in order. A nonzero force byte selects the signed
   forced level; otherwise read the signed normal level. Call `00803BA0(index)`
   only for a positive value. Earlier providers may change later records.
5. Tail-delegate to complete `00928C80` on the original unit. Do no work afterwards.

| Site | Native boundary | Coverage / required behavior |
| --- | --- | --- |
| `00779AFA`, `00779B0D` | primary `+18h`, ECX=unit | Dynamic query; second is conditional |
| `00779B15` | `004BCA80`, ECX=0 | Actual publication and render/listener delegation |
| `00779B43` | `00803BA0`, ECX=index | Optional actual slot `+25h` mark, then current-player context test and possible world latch clear |
| `00779B58` | **JMP** `00928C80`, ECX=original unit | Complete Lua/log/detach handler |

All provider bodies were reviewed. Existing `mission_entity_on_killed_00928c80`
implements the gated Lua updates and log only; it excludes the `+134h` detach
virtual and `00923050`. It cannot satisfy the complete tail alone. The other
existing host interfaces also do not supply complete actual providers here.
Providers are required, with no fallback, shadow publication or library port.

Three focused original-byte/source cases passed with different owner layouts,
dynamic query dispatch, changing publications, live record mutation, signed
levels and unchanged unrelated bytes. Five live/disk spans total 285 bytes;
the complete body requires two absolute-cell and three relative-provider fixture
relocations. Exact inputs, outputs, source, tools, headers, libraries and objects
are retained under `local/` with hashes; build and call checks are in the report.
The fixture uses recording providers and does not execute their native bodies.
Native exceptions, actual scene binding and gameplay equivalence remain unproved.
