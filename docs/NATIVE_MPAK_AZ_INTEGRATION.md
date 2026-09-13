# Actual MPAK open, enumeration and inflater integration

Addresses: 00bb40c0, 00bb4a60, 00bb4b20, 00bb5080, 00bb5bb0, 00bb5f40, 00bb68f0, 00bb79e0, 00bb79f0, 00bb7a00, 00bbbdc0, 00bbbdd0, 00bbbe10, 00bbbe50, 00bbbf00, 00bbc060, 00bbc140, 00bbc1c0, 00bbc1d0, 00bbc320, 00bbc3e0, 00bdb670, 00bdb680, 00bdbc70, 00bdbe00, 00bdd850, 00bee340, 00bef580, 00bf0fb0, 00bf4f40

AZ connects the existing actual MPAK factory/provider owners to open/search,
entry materialization, enumeration, directory-member selection, and device
routing. The raw-DEFLATE branch now has an actual 34h-byte owner and its numeric
D64400 stream methods. `NativeMpakRuntime` borrows the existing manager, pools,
registry, caches and lifetime contexts to connect these operations to
`NativeVfsRuntimeBindings` and the actual BEF750 conversion.

The batch reconstructs 30 complete native bodies, totaling 3,792 bytes. Existing
BDD0A0/BD90D0/BDBC00 dispatch and BEF750 conversion bodies gain source bindings;
they are not counted again. The C++ composition object is also not a recovered
native body. Detailed ABI, call sites, coverage, hashes and limitations remain
in the individual packet reports.

| Component | New bodies | Native bytes | Evidence |
| --- | ---: | ---: | --- |
| Entry materialization and raw owner constructor | 2 | 749 | `native_mpak_entry.json` |
| Open, contains and custom file-index search | 3 | 488 | `native_mpak_open.json` |
| Enumeration, member-directory selection, filter and defaults | 7 | 769 | `native_mpak_enumeration.json` |
| Actual raw inflater stream methods | 10 | 870 | `native_raw_inflate_stream.json` |
| Device selection, callback/lifetime and provider logical-name resolution | 6 | 898 | `native_vfs_device_route.json` |
| Memory and physical position leaves | 2 | 18 | `native_mpak_runtime.json` |

The D641F8 table identifies BB5BB0 as open (+08), BB4B20 as contains (+10),
BB5F40 as enumeration (+14), BB40C0 as the false probe (+18), BB5540 as the
already reconstructed name-copy method (+1C), and BB68F0 as member-directory
selection (+24). BB79E0 is a four-argument false default (+0C), BB7A00 clears
five DWORDs, or **14h bytes** (+20), and BB79F0 is a bare return (+28).
The older proposed lookup/open roles for BB79E0/BB7A00 were incorrect.

BB4A60 performs custom provider file-record search, despite its old medium
`stl_probable` tag. The tag/bookmark is removed with its old value retained in
the integration report. BB4140 and BB4F40/5EFBA0 remain explicit STL contracts.
Enumeration prepares a slash-appended temporary but filters with the original
prefix. Directory selection searches for a directory containing the requested
member. Raw stream reset preserves stale buffer cursors; no clean-rewind or
malformed-input progress behavior is invented.

The combined Win32 Release build and both existing CTests pass. Focused fixtures
compare the actual original entry, enumeration and inflater bodies with source,
using generated payloads and explicit shared library/support bindings. The
source runtime fixture exercises numeric inflater conversion, VFS reads, owner
release and ordinary nested binding restoration. Independent device and
compressed-entry composition evidence is recorded separately. The promotion
gate rebuilds a clean exact commit and replays fixed captured fixture inputs
against that commit's objects/libraries; prior attempts and expected results
are preserved. See `reports/native_az_integration.json` and retained `local/`
promotion evidence for the completed checks and their exact scope.

These actual-storage APIs are not original x86 ABI/FH3 replacements. Original
stack-spill aliasing, original exception machinery, general STL behavior and
installed archive/gameplay execution remain separate work. Readable original
numeric profile storage and the borrowed service graph are explicit runtime
preconditions. This packet does not establish real startup reachability or a
runnable game rebuild.
