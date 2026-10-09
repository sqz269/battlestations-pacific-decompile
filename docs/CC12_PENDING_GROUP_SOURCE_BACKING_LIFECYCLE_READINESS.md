# Pending groups: current Source backing and lifecycle

At Source baseline `b5b3773626bd37c32fae56f6336de65e04ec7f89`, the production
fixed-step host has no mutable backing for the five groups and no group
initializer call. Its iterator always returns false. A later borrowed initializer
can reproduce the accepted twenty link writes, but that alone cannot make the
production registration, flush, and wave paths share actual group identities.
This packet changes documentation and its evidence report only.

## Accepted Native evidence and its boundary

The accepted entry receipt is
`reports/cc12_pending_group_initializer_entry_readiness.json`: the complete
`00CD27C0..00CD2858` body is 153 bytes / 22 instructions, SHA-256
`ef403713f0319a915381e64216765bf0c561637508f0519db8e3cea0031649c2`.
For each ascending group, its four ordered DWORD writes are:

1. `head+4 = 0`;
2. `head+8 = tail`;
3. `tail+8 = 0`;
4. `tail+4 = head`.

| Group | Head | Tail |
| --- | --- | --- |
| 0 | `00F876C0` | `00F876F4` |
| 1 | `00F87728` | `00F8775C` |
| 2 | `00F87790` | `00F877C4` |
| 3 | `00F877F8` | `00F8782C` |
| 4 | `00F87860` | `00F87894` |

Thus `head = base + group*0x68`, `tail = head+0x34`. The body performs no
profile, count, payload, other-field, or general zero-fill store. Its minimum
written span is `00F876C4..00F8789F`, with holes. For a borrower whose pointer
denotes the first head, the highest write ends at offset `0x1DF`; a contiguous
accessible span through offset `0x1DF` is sufficient for these direct stores.
That is not proof of a complete `5*0x68` object, nor permission to access all
bytes in the span. The Native backing is reported as writable PE `.data` beyond
raw file backing; the loader's treatment and current Source storage are separate
claims. No Native bytes or Ghidra queries were repeated for this packet.

The newer accepted
`reports/cc12_pending_group_crt_initializer_route_readiness.json` resolves the
older entry receipt's outstanding static CRT table question. The PE entry
`00BFD2BD` reaches `00BFD0DD`, whose call at `00BFD222` reaches `00BFBC47`.
After successful integer initialization, the latter walks the half-open DWORD
table `00CE2734..00CE36E4` (1004 slots). Slot `00CE3020`, index 571, contains
`00CD27C0`. A nonzero current word is called without a pushed argument; the
void walker does not test its EAX result. This conditional call precedes the
Native WinMain call at `00BFD24F` on the accepted normal path.

Invocation still requires normal earlier returns, the integer initializer
walk over `00CE36E8..00CE3708` returning zero, preserved ESI/EDI and usable
stack, and the freshly read selected word still naming `00CD27C0`. Earlier
callback mutation, exceptional routes, and execution were not established.
Static Native table membership does not create a Source initializer or a
Source startup call.

## Actual current Source providers and route

The report records exact, case-insensitive `rg` searches over current
`include/bsp` and `src`, tied to the baseline. Neither `get_tick_group` nor
`get_tick_group_storage` occurs there. They are not available provider APIs in
this Source baseline. Absence is limited to that searched Source scope; no
claim about generated artifacts or the whole Native program follows.

| Source evidence | Actual behavior and implication |
| --- | --- |
| `include/bsp/fixed_step_job_waves.hpp:21` and its nearby offset constants | Constants describe five groups, `0x68` stride, head offset zero, head-next offset `8`, tail offset `0x34`, and tail-previous offset `0x38`. These are layout evidence, not allocated storage or an owner. |
| `FixedStepJobWaveHost` and `JobWaveElementView` in that header | The host exposes a positional iterator into projected element fields. The view is not a raw `0x68` group or a pair of writable sentinel objects. |
| `src/fixed_step_job_waves.cpp:75` | A group first calls `host.next_element`; zero accepted elements returns before dispatch. The phase helper loops over five group indices. |
| `include/bsp/game_hosts_fixed_step.hpp:292` | The complete private member list contains log/dynamics references, attached subsystem/game/entity pointers, counters, flags and summary state. There is no group container, base pointer, or sentinel owner. |
| `src/game_hosts_fixed_step.cpp:90` | The constructor initializes only `log_` and `dynamics_`. It does not initialize a group pair. |
| `src/game_hosts_fixed_step.cpp:128` | `next_element` discards group, position and view, optionally logs once, then unconditionally returns false. Its old empty-group comment is qualified by this actual implementation. It is not a traversal that observed initialized empty sentinel pairs. |
| `src/game_hosts_fixed_step.cpp:143` | Queue and dispatch methods count/log unimplemented work. They do not provide mutable group backing. |
| `src/game_hosts_fixed_step.cpp:401` | `flush_pending_tick_registrations_00874c90` increments a summary counter and logs a bound row when its compile-time gate is true. Its body performs no group or pending-list splice. |
| `src/game_hosts_mission_frame.cpp:205` and `:312` | `GameMissionFrameHost::Impl` creates and owns a `unique_ptr<GameFixedStepHost>` with only the log and dynamics arguments. This establishes host lifetime, not a group-storage lifetime. |
| `src/game_hosts_mission_frame.cpp:802`, `:880`, and `:965` | The actual `FixedStepBinding` delegates step waves/interpolation to that same owned host; the frame path passes this binding to the fixed-step driver. No mutable group provider is inserted along the inspected route. |

The host's extra-step body calls the same flush method under its world gate at
`src/game_hosts_fixed_step.cpp:210`; a production route to a logging method does
not establish the missing splice. Existing separate motion paths and historical
comments about empty groups are not evidence of native sentinel storage.

The current interface is named `UnitInstanceCreationHost` in
`include/bsp/unit_instance_layout.hpp:175`. Its `construct_tick_node` is pure
virtual. `create_unit_instance` calls it with `(block+tick_offset, block, 0)` at
`src/unit_instance_layout.cpp:81`. The later result's `tick_group` field is
metadata, not a raw group write. The bounded current Source search finds the
interface declaration and generic implementation only, with no implementing
host or caller of `create_unit_instance`. The older readiness wording
`UnitInstanceLayoutHost` does not name the actual current interface.

The admitted raw `construct_native_tick_registration_00875890` API explicitly
borrows its receiver, registry/manager slots, fixed pending tail, and timer word.
Its body reads and updates the separately supplied pending tail and previous
link; it does not initialize these five group pairs or own their backing.
Current Source search finds only its declaration and definition, not a
production caller. The separate `00E0B6D0`/`00E0B704` pending pair remains a
distinct identity obligation.

## Existing canonical services do not fill the gap

`GameNativeReadOnlyData` declares `00CE2000..00E07B23` and returns `const void*`
only within that mapped range. Its loader protects copied spans as
`PAGE_READONLY`. The `00F876C0` group base is outside that domain; casting a
return value cannot supply the missing writable group allocation.

`GameNativeMutableCrtData` actually commits only the three `0x1000` pages at
`00E15000`, `00E16000`, and `0109E000`. Its exposed cells are the security cookie,
complement, NLG descriptor, failure block, feature word and debugger hook.
`GameNativeCanonicalDataOwner` joins this CRT service and the read-only service;
that process owner does not add a mutable F8 group API or page. Its stable
lifetime therefore cannot be treated as an existing group owner.

## Smallest later Source change and unresolved integration

A bounded later packet can add a borrower that takes the actual mutable first
head address and writes the twenty link DWORDs in the accepted order, using
the existing `0x68` stride and `0x34` tail relation. It must store actual borrowed
head/tail pointer identities, require writable backing for the selected fields,
preserve every other byte, and avoid counts, profiles, allocation, general
zeroing, guards or calls invented from the current abstract host. This would
close the missing Source link-initialization operation. A new callable Source
interface would still need emitted-code review and would not prove the Native
fixed-address entry ABI, register/flag behavior or startup execution.

It would leave these concrete production dependencies open:

- Select and establish the actual owner and writable backing; define its
  allocation and teardown lifetime without fabricating a Native complete type
  or assuming `std::array` storage already exists.
- Bind the same actual head/tail identities to the registration/flush producer
  and wave iterator; preserve the separate pending-global pair and registry.
- Select a production initialization call before first possible list use,
  and prove that repeat initialization cannot reset live links. The accepted
  Native CRT call order alone establishes none of these Source choices.
- Replace the inspected logging/always-false boundaries with supported
  consumers, then separately verify startup and gameplay.

No borrower, owner, storage array, cast, production provider or initialization
call is added here.

## Receipt versions and validation

All 61 Source input pins in the latest owned-message primary build receipt
match the current baseline. Its three existing checks and build from
`2026-10-09T13:39:47Z` to `13:40:03Z` remain reported prior evidence; this packet
does not rerun a build, inspect artifacts, or execute anything reconstructed.
The older default-constructor 57 pins reproduce at
`fccaff11e849bd760a6384c25a3762345addfe41`, with 56 matching current Source;
the older failure-object 38 pins reproduce at
`f0b3160e89ae4330564b1dcc9b145d1fec7b6348`, with 37 matching current Source.
Both differences are `CMakeLists.txt`. All three associated document pins also
match their historical revision and current content.

Those Source pin sets contain no ledger shards. Git path comparisons record
four changed ledger shards since the 57-pin revision and seven since the
38-pin revision; old ledger/admission counts must stay version-qualified.
No current count is inferred from those build receipts. The report also
replays the accepted initializer/CRT-route repository pins and the older
pending-global binding Source pins against their recorded revisions, without
reopening captured Native data or analysis artifacts.
The CRT-route document's historical pin is its 5033-byte CRLF representation;
the current 4949-byte LF Git text reproduces that hash when expanded to CRLF.
The report records this line-ending distinction explicitly.

The accompanying JSON records exact current Source excerpts, hashes and Git
blobs, bounded search results, receipt replay differences, and the full
twenty-write contract derived from the accepted receipt. Validation is limited
to those Source/document/report relationships and owned-file scope. Source,
Original ABI, startup, gameplay, Ghidra mutation, build and test credit are all
zero for this packet.
