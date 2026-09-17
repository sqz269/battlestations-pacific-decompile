# Raw online application consumers

Addresses: 00A3F3E0, 00A3F440, 00A409F0, 00BEE080, 00BECB20, 00BECE70.

## Result

The application now borrows one raw online publication and its existing raw
frame-clock service. Its input/load cursor calls the recovered raw BECB20 body.
The platform loop also executes the previously omitted BECB20(false) after the
application frame, retaining its platform receiver across that frame as BECE70
does. This is application integration of existing reconstructed bodies: zero
new unique original functions and zero newly recovered body bytes.

The actual online publication remains null until full native online startup is
composed. R103 supplied the constructor, SDK and IPC bodies; this packet removes
consumer type mismatches without constructing a second manager or clock.

## Contracts and evidence

| Native body | Established contract | Application integration |
|---|---|---|
| A3F3E0 | Reload AB0; sample virtual+20 before debounce subtraction | Sign-in runtime accepts the existing raw clock publication context |
| A3F440 | Refresh, set pending, reload AB0, sample and copy all 16 timestamp bytes | Same borrowed clock provider; no timestamp snapshot |
| A409F0 | Reload AB0 independently at A40A02 and A40A3D | Both calls can use the same raw-clock sign-in adapter |
| BEE080 | Fixed+69 copies current+20 pair; normal mode samples QPC and frequency+60, ignores QPC BOOL | Existing raw sampler and current D68D50 slot admission reused |
| BECB20 | Online/input/mouse guards; optional pump; captured platform+41 and current online+3E8 | Raw 3F0 owner cell, actual input cell and existing device services |
| BECE70 | Application virtual+10, then BECB20(false) on the captured platform | Application loop restores that final cursor call |

All six live Ghidra normal-body listings are gap-free and match the installed
PE: 937 body bytes plus 44 bytes of the D68D50 clock profile, 981 bytes total.
The report enumerates every CALL instruction with its containing function;
indirect calls remain separately identified. Original clock sample and cursor
entries have RET4; online methods and BECE70 have RET. Source contexts are not
claims of native binary entry compatibility, FH3 or arbitrary fault equivalence.

`NativeOnlineSigninRuntime` keeps the older projected-clock overload for its
existing source users. Only that overload reports QPC failure. The application's
new overload retains the raw context, reloads its publication on every call,
checks the current native profile and +20 slot, then invokes the existing raw
sampler. Its context and loaded XLive library outlive the provider.

`NativeInputCursorDeviceCalls` contains only device operations. The legacy
projected cursor entry requires `NativeInputCursorCalls`; passing a device-only
provider there raises a source contract error. The application implements the
device interface and does not manufacture `PlatformManagerFlags`.

`GameInputRuntime` borrows the same raw owner publication as load services and
the renderer's online guard. Its pump pointer is a source-service binding,
not another native owner cell. It refreshes that binding before every cursor
operation; the recovered raw body admits only a pump for its captured manager.
The operation is retained in the runtime, recreated only after completion, and
cannot be replayed after interruption. The cursor-only entry never reads MSG;
no guessed message preimage or per-frame allocation is introduced.

Load callers capture the current platform publication. The application loop
passes its retained platform receiver explicitly, so an application-frame call
cannot redirect the subsequent cursor operation through a different publication.
Only the established raw focus storage at +41 is used; no claim of a complete
raw Win32 platform allocation or native application virtual dispatch is made.

## Validation

- Strict MSVC Win32 build and all three existing CTests pass. No permanent
  test suite or production fixture provider was added.
- One diagnostic links 52 current application objects and the three current
  libraries. All project compiler inputs resolve inside this worktree.
- The actual application's sign-in provider samples its real raw clock twice;
  timestamps lie between real QPC observations and retain the raw frequency.
- Repeated actual cursor calls preserve the genuine null-online guard. With a
  temporary diagnostic raw online publication, the actual mouse lookup is null,
  and both loading values return through that guard. The publication is restored.
- An isolated raw-storage case supplies a controlled device lookup, then checks
  that BECB20 copies online+3E8 value `0x80` without Boolean normalization. It
  performs no online pump, OS cursor, focus-reset or COM calls. All 3F0 manager
  bytes remain unchanged. This is fixture evidence, not a live online UI test.
- The raw-clock fixture switches between two 80h clock identities with distinct
  16-byte fixed timestamps. Null publication, wrong profile and wrong sample
  slot each reject the call before changing the caller's output.
- Both ordinary and diagnostic application runs exit zero after two ticks and
  one Present. Each logs two post-application cursor calls, joins the renderer
  worker and ends with device/API COM references 0/0. Diagnostic singleton drain
  returns normally.

The first diagnostic expected a bound application mouse and exited 104 after
its clock/null-guard checks passed. Its exact source, executable and logs are
retained under `local/online_consumers_r104/initial-no-bound-mouse`. The final
diagnostic records that limitation and tests the UI byte separately. The
production implementation was not changed to manufacture a mouse binding.

## Remaining work

Full online construction still needs canonical SDK/IPC process lifetime,
profile/locale/string services and explicit caller preimages. The actual
application mouse binding is absent at diagnostic entry. Therefore a non-null
online pump and real UI/focus transitions remain unvalidated. No installed SDK
initialization, account sign-in or network endpoint was attempted here.

Interrupted cursor recovery, native FH3/SEH and binary ABI remain open, as do
the initial material sampler stack word, full compiler/preload startup and
gameplay. Two ticks and one Present are startup evidence, not a rebuilt-game
completion claim. The existing 0073DD12 comment is corrected to its actual
0073BF80 material-preload call; device creation is not attributed to that site.

Evidence: `reports/native_online_application_consumers_r104.json`; local probe,
live/PE snapshots and immutable tested/integration archives referenced there.
