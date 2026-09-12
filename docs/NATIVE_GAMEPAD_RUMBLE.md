# Raw gamepad rumble enable and channel refresh

Addresses: `00A94C50`, `00A949A0`.

These additive raw-storage variants connect the existing canonical rumble flag, raw backend active vector, actual gamepad tree and concrete request getters. They introduce no owning map, copied device, private lifetime manager or SDK reference. Prior typed functions and descriptive names remain intact; names remain hypotheses rather than recovered symbols.

| Entry / complete inclusive range | Original ABI | Coverage |
|---|---|---|
| A94C50–A94D3D | CL enable byte, no other register input or stack args; void; RET0 at A94D3D | Complete normal schedule for all seven verified normalized0/1 call sites and valid native storage |
| A949A0–A94AAF | ECX actual gamepad; one channel DWORD stack argument; void; final RET4 at A94AAD, length3 | Complete normal refresh over the actual raw request tree with required output provider |

Both full byte spans match the installed executable. No missing function start or excluded tail exists. The listing has a6-byte alignment gap at A94CEA–A94CEF (`8D 9B 00 00 00 00`, LEA EBX,[EBX]), skipped by the preceding jump to A94CF0. It is not missing executable-path coverage. Ghidra was read-only throughout.

## State and source services

`NativeGamepadRumbleContext` borrows the sole F8BBF4 publication, a volatile reference to the SAME canonical bool read by existing `NativeGamepadContext`/`XInputDeviceGlobals`, a `NativeGamepadDispatch` for request reads and required `NativeGamepadRumbleOutput` for captured-profile output. The bool source interface deliberately models the established caller domain: all seven CALL sites write CL with XOR, SETZ, or BL=1. It does not claim arbitrary noncanonical byte storage can be read as a C++ bool.

The backend's actual class2 vector has its header at+B4 and begin/end at+B8/+BC. It is not the fixed3x8 attachment table. Existing raw backend group constructors/activation own that storage. A95D70 establishes the actual gamepad prefix: allocator word+20C, head+210, count+214, amplitudes+218/+21C. Nodes use the existing18h links0/4/8, key+C, request pointer+10 and nil byte+15. Existing raw request constructors and `NativeGamepadForceRequestDispatch` preserve request allocation identity and provide constant/fading/alternating value/channel getters.

Refresh uses the established checked-iterator leaf `increment_native_int_pointer_tree18_00869a20`. Its99 bytes and those of A94230 match after masking only the CALL and tail-JMP displacements to the same BF6713 service; both full listings were inspected. This is reuse of an existing recognized raw storage contract. No A94230 library port, rename or new ownership policy is claimed. `_invalid_parameter_noinfo` remains the real returning-capable CRT boundary.

## Exact schedules

A94C50 stores the enabled flag at A94C51 BEFORE loading the backend at A94C57, contrary to the pseudocode's apparent order. There is no added null-backend guard. It captures the initial signed active-vector count once, then checks the current cached backend's unsigned index bounds on each iteration. The diagnostic arm retains the old begin-field address while reloading the backend after a returning handler. Null elements are skipped without an extra publication reload.

For each nonnull captured device, it processes channels0 and1 and rereads the live enabled flag for each channel. If enabled, it calls A949A0. If disabled, it captures the current device profile and outputs positive zero before clearing the saved amplitude. It reloads the backend only after both channels, so callbacks can replace the backend used for later device indices while preserving the original loop count. Exceptions propagate; a throwing disable output leaves that channel's old saved amplitude intact.

A949A0 tests the flag only on entry. It starts with positive-zero maximum and the tree's current leftmost node. At each loop head it captures the current sentinel before iterator validation. It validates, calls the current request's value getter, rounds ST0 into binary32 with FSTP, validates again, then RELOADS the node payload before calling its current channel getter. A returning handler or value callback can therefore change which request supplies the channel. Increment occurs afterward; no tick, expiration, removal or reference change is performed.

Matching-channel values replace the maximum only for ordered greater-than using x87 FCOMIP/JBE semantics. Negative values and NaNs do not displace the initial positive zero. The final x87 FUCOMIP plus parity/equality test treats unequal or unordered saved amplitudes as changed, while positive and negative zero compare equal. On change, the native device profile is captured BEFORE the saved amplitude is written, and output follows the write. The output interface carries that captured profile explicitly. A throwing refresh output therefore leaves the newly stored amplitude visible.

Channel arithmetic remains unchecked32-bit address arithmetic; the caller must supply addressable native storage for the selected channel. The normal enable setter supplies0/1. The other four direct refresh callers forward the request's channel, including capture-before-delete in A95410. No guessed channel clamp is added.

## Callers and integration

The report carries all owned CALL rows and all direct callers. Setter sites are4CD319/4CD349,60D2DD,60D395,8ACC58/8ACCB8 and8D5DE2. Settings' EBX provenance was checked across the full listing: BL is1 from8D5CCB for the true arm. Refresh is called atA94CFC, A94C33, A954A5, A957B8 and A95C93; each pushes the channel consumed by RET4. Current containing-function extents and callee bodies were checked before assigning contracts.

The only new integration service is `NativeGamepadRumbleOutput::set_force_vslot38(actual_device,captured_profile,channel,value)`. The primary must select its real XInput/joystick/base body from that captured profile. Re-reading a potentially changed profile or reporting success for an unbound profile is not the contract. The existing finite request dispatch and raw iterator provide all tree/request dependencies. No application settings or shared runtime files are edited here.

## Verification and limits

Strict MSVC Win32 build (`/W4 /WX`, strict floating point), both existing CTests and all eight seed comparisons passed. The report's18 direct CALL rows passed with0 failures; four indirect value/channel/output sites were manually checked against stack cleanup and register provenance. Exact function spans, hashes, ABI details and library equivalence are in the report.

One ignored manifested probe links only the checkout's headers/archive. It executes the two relocated original owned bodies with global addresses and call boundaries redirected to the same concrete raw request getters, established iterator, real CRT handler and an output observer. Its four native/source comparisons cover:

* NaN request filtering and unordered saved amplitude:1 output.
* Disable output re-enabling rumble and replacing the backend:4 outputs across subsequent channels/devices.
* Value callback changing the head/flag, then a returning handler repairing the head and replacing the request before channel lookup:1 diagnostic and1 output.
* Negative/NaN requests with a saved negative zero:0 outputs and unchanged saved bits.

Source-only throwing-output checks verify the opposite write orders for refresh and disable. Existing concrete cleanup destroys four tree-owned requests, one spare request and two raw trees. These are controlled callback/handler fixtures, not production request producers. Logs and hashes are recorded under ignored `local/native_gamepad_rumble_af_*` paths; the runner accepts a primary worktree for an archive-only integration rerun.

No SDK force, GetState, SetState, device poll, game launch or application settings call was made. The fixture does not establish hardware delivery, original SDK or binary ABI compatibility, unmasked floating-point exception identity, FH3/SEH, malformed/concurrently mutated storage or arbitrary iterator-stack aliases. Complete application composition remains the primary's separate work.
