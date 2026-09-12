# Canonical Model clone transport for Text

Native `B752B0..B753F1` takes source in ECX and flags/parent on the stack,
returns the clone in EAX and executes `RET8` at B753EF. The composed interface
handles Text's flags26h/parent0 call at AB9D87. Other flags/parents and positive
point-light ownership remain explicit dependencies.

`GuiWidgetOwnerRuntime::create_model_clone_destination_00b752b0_fragment`
allocates from the existing Model pool01090054 at the B752D1 phase. Only then
does it borrow the source's same NativeString header+54 through B6D800 and
execute the existing B75030 constructor. No std::string snapshot, temporary
native name, extra native retain, or second model registry is introduced.
The source must be a live canonical Model in this runtime and survive calls.
The resulting owner/reference is registered in the same models_ map used by
ordinary widgets and shadow models, with one native creator reference.

The native construction unwind CC1C70 jumps to B748C0 to return the saved raw
slot; the new transport returns the slot on construction failure. Host map or
companion allocation failures are additional diagnostic boundaries. Native
null-allocation dereferencing, original EH frames and binary ABI are excluded.
After construction there is no clone-wide native rollback. Caller-visible
`NativeGuiTextModelCloneAcquired` retains the created Model and the mesh-copy
packet's unconsumed mesh/section/material creator references if later work
throws. The caller must release or finish them through their actual domains;
discarding this record does not run native cleanup or complete the clone.

`clone_native_gui_text_model_00b752b0` uses the existing base-copy fragment,
then reloads current source geometry+180. A positive point-light count returns
its established explicit boundary after destination construction and before
base-copy effects. The caller keeps the destination; restarting the whole call
would allocate again. This boundary is not a successful cloned result.

For a present geometry, the same actual registry resolves its NativeMeshReference.
The live Model profile+10 must be B752B0 (D62DF8 bytes B0 52 B7 00), and an
explicit live mesh profile with at least five DWORDs must have+10 B742A0
(D62D70 bytes A0 42 B7 00). The mesh owner environment's older two-entry
profile guarantee is insufficient for that read, so it is not assumed.
The real mesh clone creates sections/materials and shares streams according
to flags26h. After it returns, the existing association fragment reads source
17C then178, publishes geometry, then consumes its creator reference. The
final retained174 assignment and ten x87 source08..2C pairs execute afterward.
Callbacks may change the source before these late reads.

The owner runtime also exposes primary-null Text construction for AB98F0 and
admits Text through the ordinary AA6560 route when the supplied type factory
constructs the concrete implementation. Primary-null construction performs
neither ordinary model allocation nor current74/78. Generic current70 dispatch
now reaches the type implementation; unsupported profiles throw explicitly.
No rebuilt executable startup configuration is silently supplied.

These are new Win32 C++ interfaces over the existing owners. Native string
pool ABI, arbitrary reentrancy that destroys borrowed owners, allocation
failure parity, complete game reachability and gameplay validation remain
outside the current evidence. See `reports/gui_model_clone_transport.json`
and the final Text factory batch record for compilation and integration.

## 2026-09-12 timed and clip ownership integration

Positive point-light lists no longer return the previous owner-required boundary
when supplied with actual live light bindings and proven backing. The source
array holds raw light identities; each light's actual +1E0 descriptor holds raw
node identities. Clone copying uses the same physical lists and preserves live
count/reload and captured-source ordering. Five actual node destruction adapters
remove backlinks through this runtime. Full PointLight construction/type/pool
ownership and other Model clone branches remain external. See
NATIVE_POINT_LIGHT_LINKS.md and reports/orch5_timed_clip_batch.json.
