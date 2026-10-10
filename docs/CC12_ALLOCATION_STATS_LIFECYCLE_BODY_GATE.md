# Allocation statistics lifecycle: bounded body gate

Root accepted the physical ordinary destructor and scalar-delete schedules.
The raw owner Source API remains held on base-profile deletion and constructor/
destructor failure cleanup. No C++ implementation, build, test or game execution
is credited by this packet.

BE27F0 has a complete single-range 153-byte/40-instruction body. It captures the
incoming receiver in EDI, stamps D685E0 before state 0, obtains the first manager
and captures its section+10. If nonnull it enters/increments that section+18.
Under state 1 it gets the manager again, reloads current 0109CEFC and unregisters
that value through the second manager. Only after normal unregister does it
unconditionally clear 0109CEFC. It decrements/leaves the original captured section
and then stamps CE3818 on the captured receiver. Handler CC6AB0 is referenced;
its failure cleanup was not opened or inferred. Budget/startup words are unused.

BE2930 physically has 30 bytes/11 instructions. It captures ECX in ESI and calls
BE27F0 before testing stacked flags bit0. If set it frees that captured receiver,
consumes the free argument, then returns the original address in EAX and RET4.
Other flag bits do not change this observed path. Original ABI/EH/fault held.

The saved scalar AddressSet is 27 bytes in two ranges: 2930..2944 and2948..294D.
Root first opened only these exact ranges plus the 153-byte destructor. Typed
metadata then identified CALL_RETURN at 2940 despite both the genuine_free thunk
and direct target having no_return=false; 2945..2947 were undefined/unowned.
A separately admitted three-byte physical/live read proves 83C404=ADD ESP,4.
The remaining saved listing gap/override is preserved, not treated as repaired.

The existing ghidra_flow_repair.py explicitly disables --apply pending an
attested bounded atomic route. A future exact repair must preserve complete
preimages, clear only 2940's stale override, decode only 2945's three bytes, union
only those bytes into the same body, preserve callee flags/prototype/comments/
other flow, verify the complete 30-byte/11-op body and save or roll back. Typed
read capability and physical bytes do not by themselves attest that mutator.

All 183 selected original bytes equal live Ghidra memory; the unchanged original
whole-image hash was freshly checked. Capstone consumes every byte. All 51
physical instruction starts were queried: 50 match listing lengths; the one
missing gap instruction remains explicitly separate. Root replayed 71 strict
typed responses at preannotation epoch 12 and postannotation 17 plus 22 raw body/
identity HTTP hashes. Actual GPR/program/language/base/space were checked for
each batch. No callee body, data, table or handler window was opened. Loaded
Java CodeSource remains unattested. Complete raw receipts are pinned locally.

Provisional descriptive names BSP_AllocationStats_Destroy and
BSP_AllocationStats_ScalarDelete and qualified evidence comments were applied
through the annotation write lock. Full old names/absent-comment preimages were
saved and exports refreshed. No body, flow, prototype or callee flag changed;
postannotation typed metadata confirms the same body/override/gap. A receipt
collector hit an absent old comment key after the successful saved write;
finalization resumed only reads/exports and did not replay either mutation.

Next evidence remains the actual D685E0 deletion prefix, only its real target,
BE2750/caller allocation ownership and required destructor ordinary failure
cleanup. A genuine allocated three-word owner requires profile dispatch bound
before publication/registration and retained through the same canonical drain.
No fake table/manager/metric or neighboring-family inferred destructor is
admitted. Source636 counts/tests are unchanged context. Startup/cache composition,
full Native ABI/FH3/SEH/longjmp/fault and gameplay remain held.
