# Process input publication for settings shutdown

Addresses: `008D4950`, `008D78D0`, `00CDEEC0`.

## Result

The ordinary application's E198E8 input publication now belongs to the retained
`GameNativeInputSettingsProcess`. `GameSingletonHost` borrows that same cell and
the existing process manager cell. It binds and retires the application's actual
input lifetime context together with its CF81CC deletion binding.

`NativeGameSettingsContext` holds a `NativeGameSettingsInputReference`: stable
publication references plus either the original fixed input context or a live
context binding cell. The native `008D4950` null-publication path reads no
application context. A nonnull input retains the original captured-manager lock,
recheck, unregister, CURRENT publication reload, scalar flags1 deletion, clear,
and leave order. The context is resolved only for the scalar call.

This removes the application-lifetime dependency from the null-input path of a
later settings CRT callback. No input table, Lua interpreter, or placeholder
service is created for CRT shutdown. Binding a different publication domain,
replacing another live context, or retiring while E198E8 remains nonnull fails
explicitly. These are source binding guards, not additional recovered branches.

## Native evidence

Live project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, checked
against the installed original PE: 345 bytes across the complete existing bodies
`008D4950` (164), `008D78D0` (171), and `00CDEEC0` (10).

At `008D4968`, the first E198E8 test precedes the manager call `008D4972`.
After unregister `008D49B4`, the routine reloads E198E8 at `008D49B9`, calls its
current slot0 at `008D49C9`, then clears E198E8 at `008D49CB`. `008D78FC` reaches
the release routine before clan-string and vector cleanup. `00CDEEC0` is the
settings static shutdown wrapper. The report contains ten direct CALL rows and
one tail JMP; indirect lock/scalar calls remain separately identified.

Existing descriptive Ghidra names are preserved. This packet appends source
lifetime evidence; it adds no newly recovered native body or binary ABI claim.

## Validation

- Strict MSVC Win32 build and all three existing CTests pass.
- The unchanged R157 original/source fixture still passes 13 pairs with 3,076
  matching observed bytes, including raw owner padding, input release, scalar
  flags, static registration order, and the retained-failure/replay boundary.
- A focused component links the actual game host objects and uses installed,
  mounted VFS data, canonical Lua globals, and actual pooled strings. Both child
  processes load 4 devices, 16 input names, and 12 controller names.
- One child releases populated input through `008D4950` before shared drain.
  The other uses the shared singleton drain. Both reject premature binding
  retirement without changing the host deletion binding, then retire successfully.
- In each child, real `std::atexit` registration runs the complete settings
  `00CDEEC0` callback after the application input/Lua/VFS/host scopes have ended.
  E198E8 is null and the live binding is retired. Calling the input release at
  that point leaves the already-null manager untouched; full settings cleanup
  then completes. Exit status and per-child CRT receipts are checked.

The component's BCh settings owner is a retained fixture with null online/game
publications and an SDK boundary that rejects any unexpected read. It is not the
ordinary application's canonical settings owner. No game window or renderer is
started by this component. The report records the separate ordinary application
launch outcome and exact tested/combined artifacts.

## Remaining work

Bind the actual BCh settings process owner, its online/game publications, and the
complete R167 options loader to ordinary startup. The legacy projected game
settings block remains in that path. Original FH3/SEH, binary replacement ABI,
concurrent mutation, full application startup for this packet, and gameplay are
not established by the component checks.
