# Concrete Layer release from particle resources

The complete AF4280 source now composes the recovered Layer lifetime bodies
when a zero-reference child has profile D5DC38. Its unchanged forward loops
still reload the current signed counts after each child callback.

Live D5DC38 contains slot0=BD30E0 and slot4=AFACE0. The source decrements once,
captures the current terminal profile, and implements BD30E0's second table
read before calling the genuine Layer scalar destructor with flags1. The
resource's actual raw string-pool context is forwarded. No callback facade or
second reference-count decrement is introduced. Other child profiles retain
the callable current terminal boundary; numeric emitter identities remain open.

This extends the i9 resource lifetime report and j10 Layer lifetime report.
It does not change original body coverage, loop order, name ownership, or
unwind states. The complete AF4280, BD30E0 and eight-byte table were rechecked
against live Ghidra and installed PE bytes. Original Layer construction,
destruction and scalar bodies already passed their copied-original comparison.

The current strict Win32 build and three CTests pass. One temporary composition
probe constructs a real resource, two native Layer objects, and a foreign
callable child. It verifies terminal Layer cleanup and pooled material/name
returns, a surviving Layer reference, exactly one foreign terminal invocation,
cleared counts, and the final resource base identity. Independent source and
assembly review found no discrepancies.

The added composition fixture executes source, not a copied AF4280 with numeric
image tables. Original AF4280's callable-child comparison remains the earlier
i9 evidence. This update does not claim original exception execution, arbitrary
concurrent vtable mutation equivalence, binary ABI replacement, or gameplay.
