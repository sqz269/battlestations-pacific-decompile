# Damageable category/FireID capture primary review

Root accepts the local saved-capture and typed-storage contract from worker
commit `3cbee046202c78f0841e84051a849c6cb16b2cd6`. The next authorized Source
packet is only the guarded saved-pointer borrow on the existing successful
Msh owner. It must require state 17 and the exact scratch identity, return
false without writing output when rejected, and expose only the original
saved data pointer on success, including a legitimate null capture. It must
not reload current S18, read saved length, copy/own text, call a provider,
change state, or close/reenter the owner. The caller must preserve the original
buffer's lifetime/current contents and keep output storage disjoint.

Root independently replayed 1,357 pin occurrences, 162 complete baseline Git
inputs, 151 full Source/header snapshots with their quoted header closure,
739 retained Source735 inputs/artifacts, and 30 exact archived provider members.
Eighteen members correspond to Root's prior selected whole-object evidence;
the other twelve add archive/Source provenance without a new COFF decode claim.
The retained 193-byte category/FireID region was fully decoded as 59 instructions.
No fresh Native image window or live Ghidra query was consumed.

The proposed full sequence additionally requires genuine typed void-pointer
S28 and a distinct pending full DWORD flag. Flag one must be stored after field
lookup and before the numeric getter; the existing ID wrapper captures the
output/ID, unconditionally gets the manager, then reads the full flag. The
numeric getter still requires its own live bool conversion-mode object, which
cannot alias that DWORD or be presented as Native DWORD-mode parity.

The raw sixteen-byte manager owner/getter/destructor and application probe now
exist, but the ID wrapper's lower API still uses a separate std::map projection.
No compatible raw overload or callable rebuilt current slot-zero adapter was
found. The prebound definition companion is not a raw adapter usable at zero;
Native profile words alone are not callable rebuilt vtables. Consequently the
full category/FireID fragment and application activation remain conditional.

This review changes no C++, build, tests, Ghidra, ABI, startup or gameplay state.
The complete Source sequence and independent evidence are retained in the
worker contract and accompanying primary report.
