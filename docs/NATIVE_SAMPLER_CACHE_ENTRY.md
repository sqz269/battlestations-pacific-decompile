# Actual sampler cache entry and loading messages

Addresses: 00b1a4f0, 00b1b4d0, 00beccd0, 00becb20

The new entry composition closes the previously omitted B1A4F0 message-pump
prefix and B1B4D0 default-options wrapper. It calls the unchanged substantive
B1A51D..B1AA29 cache continuation with its real raw secondary cache. It also
provides the actual-platform BECCD0/BECB20 schedules, instead of adapting the
projected `Win32PlatformState` or `PlatformManagerFlags` to native offsets.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| B1A4F0..B1AA29 | ECX actual secondary cache; stack name/options/retain-new byte/load-if-missing byte; EAX resource; RET10 | Complete normal entry composition with unchanged actual continuation |
| B1B4D0..B1B5E7 | ECX actual 1Ch singleton; stack actual name; EAX resource; RET4 | Complete normal wrapper, persistent source failure state |
| BECCD0..BECD36 | ECX captured actual platform; RET | Complete message-pump schedule |
| BECB20..BECCC7 | ECX actual platform; stack loading byte; RET4 | Complete actual-field policy schedule in the stated provider domain |

B1A4F0 captures the current raw0109CF04 platform before BECCD0. The receiver is
preserved while the real thread-message queue drains. Requested-name data is
first read only after the pump returns. B1B4D0 initializes/copies its actual
eight-byte requested name, captures the resulting buffer in the native EDI
role, lowercases it through4BCC00, and constructs the pooled seven-byte Default
from the borrowed original CE3C5C bytes. It passes receiver+4, both actual
headers and flags1/1 to B1A4F0. Normal cleanup releases the current options
buffer with current length+1, then the captured requested buffer with its
current length+1. Source names are not converted into independent strings.

BECCD0 captures real PeekMessageA, TranslateMessage and DispatchMessageA
bindings, loops with null HWND, filter0/0, PM_REMOVE, and forwards the actual
MSG to XLiveLibrary ordinal5030 (BOOL stdcall(MSG*) through C2F1D2/CE25DC).
Unconsumed messages translate/dispatch. It does not interpret WM_QUIT or run a
frame callback. After draining, BECB20 receives the same captured platform
and loading1. The MSG preimage is caller-owned; the wrapper does not infer SDK
output. No pump is run against the user's live game by this packet.

BECB20 reads current raw online and input publications, then the real class1,
index0 device getter. It optionally pumps the captured actual online manager
through the supplied canonical `NativeOnlinePumpContext`; a reached mismatched
context is an explicit source-domain error. It then reads captured platform+41
before reloading current online+3E8. The full raw UI byte, including non-Boolean
values, is preserved when publishing previous-state DB90. It reuses actual
BECA40 reset and the existing raw input/COM providers, loading backend current
profile and x87-rounded D7A2F0 float at each reached virtual+4 call. ShowCursor
captures precede DB8E writes and the original signed repetition loops remain.
DB8F deferred reset and all current-publication reloads follow the listing.

The context's nullable input/online service pointers are permitted only before
native guards prove their services are needed. They do not create substitute
owners. The input context must borrow the exact supplied F8BBF4 publication.
The existing `NativeInputCursorCalls::call_004ba6d0` remains an explicit raw
getter dependency: this packet does not implement that provider or claim that
context plumbing proves it. Existing actual input device/COM/slot providers and
online SDK/localization/clock contracts remain required. No new sampler-loader
callback, fake current manager, policy observer or successful pump marker is
used in production.

Native FH3 differs from the source failure boundary. B1B4D0's two-state map is
DF4A9C and FuncInfo DF4AAC; CBC750/CBC758 tail to41DD20 on the two local headers.
B1A4F0's five-state map startsDF4964 with FuncInfo DF4940; CBC670..CBC697 contain
four string cleanups and one actual record cleanup. Full bytes, handlers and
normal-body evidence are pinned. The new source wrappers retain acquired names
and adopted nested frames on exceptions because the unchanged continuation
retains native acquisitions for diagnosis. They do not secretly retire that
frame while reproducing an outer cleanup. Explicit diagnostic resolution is
required before destruction; this is not native exception ABI equivalence.

Validation is shared with [NATIVE_MATERIAL_DESCRIPTOR_SAMPLERS.md](NATIVE_MATERIAL_DESCRIPTOR_SAMPLERS.md):
strict Win32 build, 8 verified seeds, 2 CTests, original/source full default-load
hit/null paths and real Win32/XLive pumping under the native null-online guard.
The nested profile-failure probe proves surviving continuation/requested/default
state and guard77. Nonempty online/input/ShowCursor execution, raw getter
provider closure, zero-reference resource terminals, native FH3 exceptions,
application wiring and gameplay remain unproved. See the machine-readable
[entry report](../reports/native_sampler_cache_entry.json) for numeric calls and
the frozen evidence/compile/link/runtime closure.
