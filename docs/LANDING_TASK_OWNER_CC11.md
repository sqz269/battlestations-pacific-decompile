# Pilot-bot task ownership and destruction order (cc11)

This audit resolves the pilot-bot override used by the plane's destroyed hook.
It adds native evidence and annotations only. No runtime task FIFO or lifetime
adapter is supplied. Names are reconstruction hypotheses.

The target was the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified by the BSP client before live queries.
The original installation and analysis were preserved. The companion receipt
is `reports/landing_task_owner_cc11.json`.

## The pilot bot is plane+DF4h

The plane SEntityInit constructor slice `007CA28F-007CA2D4` allocates BCh bytes,
calls `0099A880` at `007CA2AE`, stores its EAX result at **plane+DF4h** at
`007CA2B7`, and binds plane+310h through the bot's vtable+4h. This slice is
outside Ghidra's current truncated `007C9770-007C985F` function body. Its
instruction starts were checked live and its complete bytes decoded from the
disk image. This audit does not repair or claim the whole initialization body.

Two property-bag producer arms independently allocate BCh, call `0099A880`
at `007D66E7` / `007D71FE`, then store the bot at +DF4h at `007D66F0` /
`007D7207`. The neighboring +6DCh producer calls **00864580**, not this pilot
bot constructor. Therefore the +6DCh generic disable body is insufficient to
identify the pilot-task shutdown path.

The full `0099A880-0099A934` constructor calls base `0072BBD0`, installs
vtable **00D1F348**, installs the callback subobject table at +1Ch, and zeros
the active task vector +58h/+5Ch/+60h and retired vector +64h/+68h/+6Ch.
It calls the real CRT array-construction helper `00BF7CD1` for two embedded
plan records; `0099A880` itself is the owning object constructor. Its previous
`CG_array_ctor_helper` name was a heuristic classification, not proof of role.

The table DWORD at **00D1F358 = 00D1F348+10h** is **0099A830**. The slot-zero
scalar deleting destructor is `0099A850`; its member cleanup is `0099A720`.

## Death removes membership before draining tasks

In `007BCAA0`, the plane destroyed hook:

1. Calls `00959450` at `007BCAD9`.
2. If current plane+9D4h is non-null, calls squadron removal `007F3970` at
   `007BCAEB` with that squadron, the plane, and flag 0.
3. Calls **plane+DF4h's vtable+10h** at `007BCAFF`, then clears +DF4h at
   `007BCB01`.
4. Later calls +6DCh's vtable+10h at `007BCB1A` and clears that field.

For the producer-established pilot-bot profile, step 3 resolves to
**0099A830**. Its complete body `0099A830-0099A847` sets bot+10h=1 and
bot+11h=0, calls retire-all `00999E40` at `0099A83B`, restores ECX=bot, and
tail-jumps to drain `00999EE0` at `0099A843`. This is a task-drain override,
not the common flag-only `0071C4A0` body. It does not delete the bot itself.

This establishes the call order when +DF4h still holds this profile. It does
not prove that every runtime plane retains that profile, or that all cached
task references survive the earlier base destroyed notification.

## The bot owns task deletion, with two distinct teardown paths

`00999E40-00999EDF` appends each active task pointer to the retired vector in
active order, preserving existing retired entries, and finally clears active
count +5Ch. Capacity grows as `old_capacity*2+2`. After freeing an old pointer
array at `00999E9F`, assembly continues at `00999EA7`; the current pseudocode
incorrectly terminates there because of a no-return annotation. No global
CRT flow property was changed.

`00999EE0-00999F41` repeatedly dispatches the retired head's virtual **+58h**,
then its scalar deleting destructor with **flag 1**, shifts the remaining
pointers, and decrements +68h. The head must be valid for the unconditional
hook call; the later null check does not establish a nullable-task domain.
The land-task table supplies `009B33F0` for that hook and `009B4100` for the
scalar destructor, as established by the retained-hook audit.

The separate member destructor `0099A720-0099A7F7` scalar-deletes each active
task with flag 1 **without** first invoking +58h, destroys the embedded plan
records, frees the retired and active pointer arrays, destroys the callback
subobject, then calls base cleanup. It does **not** iterate/delete remaining
retired task objects. Its prior `CG_vector_deleting_dtor` label is therefore
not the callable scalar/vector deleting wrapper: this member body takes ECX
and returns with plain RET. `0099A850` is the actual flag-taking scalar wrapper
and optionally frees the bot after calling this member destructor.

A source owner must preserve those different paths. Folding bot destruction
into the ordinary hook-before-delete drain would change behavior; freeing the
retired pointer array alone is not permission to invent deletion of its tasks.

## Consequences and remaining binding work

For a land task drained through the pilot override, native squadron removal
precedes the hook and task destruction. `009B3F50` subsequently checks the
retained plane's **current** +9D8h/+9D4h; a removed plane's null squadron does
not become its old cached squadron merely to force carrier dequeue. This
narrows the earlier death-order uncertainty.

The old task's +404h cache, base destroyed-notification observer effects,
bot/session-node deletion service, reentrancy, and stable host identities
remain separate lifetime contracts. The GameUnitsHost Boolean landing state
still has no matching active/retired owner. No death repair, landing-loop fix,
native ABI equivalence, or original-game runtime result is claimed.
