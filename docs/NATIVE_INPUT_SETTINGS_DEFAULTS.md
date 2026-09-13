# Native default input bindings

Addresses: 006AB820; downstream body-ownership repair006AA640.
Packet `orch4_input_defaults_am`. Names remain hypotheses. Source:
`native_input_settings_defaults.hpp/.cpp`.

006AB820 is a complete547-byte normal schedule through006ABA42. Native ECX
is the actual540h settings receiver, with no stack inputs. If byte+50 is
nonzero it returns immediately. Otherwise it ends with tail JMP006AA640,
passing the original settings receiver. Its caller is00699B08, after the
settings singleton getter005547D0. This packet does not supply that singleton's
raw constructor/script population or the loaded-keyboard branch of006AA640.

The source uses the actual24h action owner,30h actions,34h bindings and14h
modifiers shared with the native Inputs parser and slot operations. It captures
the real004BEC00 owner once before any tree allocation. Settings+54 is a12h
signed-int/DWORD tree header: opaque+0, sentinel+4, count+8. Nodes are18h with
signed key+C, original-binding-count+10, color+14 and nil+15. This is separate
from the projected `InputSettings` object, which cannot receive this pointer.

The routine ensures keys4A,4B,46,47,4C,4D,1 in that order. It then traverses
every current node in sorted order, including preexisting keys outside that
list. Each mapped word is replaced by the action's current binding count.
That mapped count is reloaded during the inner loop; it is not the growing
action-vector count. For each ascending slot, A92790 reads the descriptor and
scale, another x87 FLD/FSTP spills scale for the call, and A93750 installs the
descriptor at slot+4. A second installation clears the original descriptor to
{-1,0,0,-1,flag} and scale to positive zero. Both calls rebind the entire action.

Only descriptors and scales move. Required/forbidden modifier allocations remain
owned by their individual slots. The cleared flag's low byte is zero; its high
24 bits come from incoming private native stack contents. The source exposes
that preimage explicitly, alongside the reader's missing-slot stack preimage.
No private-stack alias, zero-padding or complete native SEH promise is made.

Ascending processing matters when an action initially has more than four
bindings: an earlier write to slot4 is observed when iteration later reads
slot4. The routine also has no once-only guard. A repeated call records the
new counts and shifts again. Source code preserves these behaviors, current
tree endpoints, returning CRT validation and the captured action owner.

The final engine call is a required `NativeInputSettingsKeyboardApplication`
provider operating on the same raw540h receiver. No successful fallback is
provided, and the existing projected keyboard implementation is not used as
an ABI substitute. Full raw006AA640 and settings loading remain required for
complete production startup. A failed provider leaves preceding mutations
intact, as this routine has no rollback or local owning cleanup.

## Shared library storage

No new STL implementation or library rename is introduced. The existing scalar
subscript adapter supplies raw mapped storage without float conversion. Native
6A1460 inserts a zero DWORD with MOV, while its deadline counterpart4D6900
uses XORPS/MOVSS for the same bits. Both preserve signed comparisons, late key
reload, the generated lower-bound hint, and node+10 result validation.

That valid hint reaches the same five insertion sites as the shared unique
driver: empty, new minimum, new maximum, or either vacant interior successor
site. Actual69F210 uses18h allocation through69D2D0, limit1FFFFFFE, current
count increment, color/nil14/15, and the existing link/rebalance schedule.
The original increment69C370 and reused869A20 are byte-identical after only
their relative CRT targets are normalized. Original library ABI, arbitrary
hint input, malformed trees, private spill aliases and EH identity are excluded.

## Evidence and validation

All nine fixture spans match live Ghidra and installed PE bytes. The owned
routine has169 instructions and no flow gaps. The report verifies90 CALL/tail
rows, including its actual loader caller. A downstream audit initially failed
at006AAC18: Ghidra's full-range listing showed decoded instructions there,
but `get_function_by_address` returned no function. The standard flow check
reported zero gaps, so it did not prove ownership. After verifying the1540-byte
006AA640..006AAC43 span and no intervening entry, the function was recreated
under its lease and the write lock. The receipt preserves prior metadata;
the cleanup call is now owned and the full audit passes. No CRT no-return
annotation was changed.

One ignored manifested Win32 fixture compares1680 words. It executes original
6AB820,6A1460,6A0BA0,69AB10,69C370,A92790,A92820,A93750 and the zero-loaded
early return in006AA640. Native tree linking, raw resize/rebind and owner lookup
share established concrete source providers. Unreached general-hint and loaded
keyboard paths fail if invoked. The test starts with actual valid fixture
headers/records, not a reconstructed540h constructor. It checks seven new tree
insertions, extra existing keys, balanced tree shape, signed mapped counts,
ascending overlap, repeated shifts, retained modifiers, full clear flag bits,
descriptor/scale state and DEVINPUTS gating. The native branch performs50
resizes and144 rebinds; four original keyboard guard returns cover both runs.
Device calls are zero. Allocator-dependent untouched padding is masked.

Strict Win32 build, both existing CTests and all eight disk/live seed checks
pass. Final combined-source/archive/fixture hashes are pinned in the report.
No permanent tests were added. Hardware polling, native ABI/FH3, loaded saved
keyboard settings, the full loader tail and gameplay remain unvalidated.
