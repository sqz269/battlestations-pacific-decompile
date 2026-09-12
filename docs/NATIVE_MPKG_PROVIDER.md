# Actual MPKG factory and provider lifetime

Addresses: 00736A90, 00737090, 00735D00, 00735800, 00BB9CB0, 00BB9D90, 00BB9E70, 00BB9EE0.

`native_mpkg_provider.hpp/.cpp` reconstructs eight complete logical bodies over actual Win32 storage. The factory borrows raw singleton publication01090AA0 and factory publication010904F4; the provider borrows the application's actual string pool and explicit archive operations. It creates no semantic lifetime, private pool, parser fallback or implicit shutdown dispatcher. Source interfaces are not native ABI/FH3 replacements.

| Native span, inclusive | Bytes | Source operation | Native ABI | Coverage |
| --- | ---: | --- | --- | --- |
| 00736A90..00736B5D | 206 | MPKG factory getter | no arguments, EAX factory, RET | complete |
| 00737090..007370C9 | 58 | factory scalar deletion | ECX primary, flags stack, EAX captured owner, RET4 | complete |
| 00735D00..00735D07 | 8 | secondary deletion adjustment | ECX secondary, SUB4/JMP737090, inherited RET4 | complete |
| 00735800..00735828 | 41 | temporary-base scalar deletion | ECX base allocation, flags stack, EAX captured owner, RET4 | complete |
| 00BB9CB0..00BB9D20 | 113 | provider construction | ECX owner, original name stack, EAX owner, RET4 | complete with explicit BB9920 operation |
| 00BB9D90..00BB9E62 | 211 | factory Create | ECX factory unused; system/virtual headers stack, virtual unused; EAX provider/null, RET8 | complete |
| 00BB9E70..00BB9ED2 | 99 | provider destruction | ECX owner, RET | complete with explicit BB9C10 operation |
| 00BB9EE0..00BB9EFD | 30 | provider scalar deletion | ECX owner, flags stack, EAX captured owner, RET4 | complete |

The 766 installed instruction bytes equal the live Ghidra bytes. The primary repaired 737090's omitted7370C1..C3 `ADD ESP,4` continuation and recreated its body through7370C9. The primary also defined the previously missing exact41-byte735800 body. Existing names/comments were preserved; this worker made no Ghidra mutation. The auxiliary EH actions still have unowned `POP ECX; RET` tails at CC48E1..CC48E2 and CC48F9..CC48FA; their complete raw bytes are retained, and neither tail is presented as a defined Ghidra function.

## Factory storage and order

736A90 is the actual producer of the8h factory. It writes temporary secondary CFE9FC at+4, primary CFEA14 at+0, then final secondary CFEA10 at+4. The immutable tables resolve CFEA14+0 to737090, CFEA14+4 toBB9D90, CFEA10+0 to735D00, and CFE9FC+0 to735800. There is no provider cache.

The getter preserves its first publication read on the fast path. Otherwise it gets the current raw manager, captures its physical section+10, enters and increments that section's recursion+18, then rechecks the factory publication. After allocation/profile stores/publication, it reloads the factory and captures factory+4 (or null) before the second manager getter and BD0C30 registration. Registration does not acquire a reference. Both normal leave and C++ cleanup use the original captured section. Registration failure leaves the factory publication/allocation intact.

737090 clears the publication, writes secondary CE3818 and primary CFE9F4, then optionally frees the captured primary. 735D00 adjusts the actual registered secondary by-4. 735800 has no adjustment and writes only CE3818 at its received allocation. None of these routines unregisters itself; callers must compose the real raw manager's lifetime order. Null deletion is not made safe: native null tests still lead to writes through null storage.

## Provider storage and archive boundary

BB5590, consumed from the established provider base, produces reference count+4=1, copied name length/data+8/+C and device+10=FFFFFFFF. BB9CB0 then installs D64390, allocates34h and calls BB9920 with the captured allocation and the original actual8h name-header pointer. It stores the returned EAX at provider+14 only after successful return. The resulting provider is18h. BB9920 and BB9C10 are primary-owned dependencies, not reconstructed here.

`NativeMpkgArchiveOperations::construct_00bb9920(actual_archive, actual_system_name)` and `destroy_00bb9c10(actual_archive)` expose those exact boundaries. The constructor forwards the original header, including changes made by earlier operations; no normalized name, byte-buffer substitute or bounded `MpkgArchive` parser is used. The destructor operation never owns the outer allocation's free.

BB9D90 first captures the unsigned system length. Only length>5 calls the existing complete469840 substring helper with start=length-5 and count=7FFFFFFF. This matters for null data, signed-start interpretation and DWORD wrap; a direct last-five-byte pointer comparison would change the contract. The returned actual header is compared to D1D818 `.mpkg` through425850, consuming AL and preserving the established CRT case-insensitive behavior. The current suffix data/length are released before any18h provider allocation. The `.mpkg` name alone declines. No header, embedded-NUL, length or path validation is added. Incoming factory and virtual header are unused.

BB9E70 rewrites D64390, captures archive+14, calls BB9C10 when nonnull, frees that same captured archive, then invokes BB5380. It leaves stale archive/name pointers and the reference/device words. BB9EE0 frees the captured provider only after successful destruction and flags bit0. No reference decrement or cache cleanup is added.

## Exception evidence

Getter FuncInfoDB5510/unwind mapDB5508 has state0 C86040 ->411EE0, armed only after section entry. Constructor FuncInfoDFE544/mapDFE534 maps state1 to CC48D8 (free captured allocation), then state0 CC48D0 ->BB5380. Factory Create FuncInfoDFE570/mapDFE568 arms only the provider allocation cleanup CC48F0; its temporary suffix is a normal-path guard, not an EH owner. Destructor FuncInfoDFE59C/mapDFE594 maps state0 CC4910 ->BB5380; an archive-operation exception skips archive free and still destroys the base.

The source expresses these C++ cleanup orders. Existing `NativeStringStorage::release` is noexcept, so throwing native lazy-pool cleanup, source double failure, original CRT/FH3 identity, hardware faults and aliases into compiler EH spills remain outside this interface. Allocation-null branches remain present, although the established source new-handler service normally throws when exhausted.

## Verification

Strict MSVC Win32 `/W4 /WX /fp:strict` build and both existing CTests passed (`local/native-mpkg-provider-build02.log`). The initial configure failure from a nondeferred registration is retained in build01; the owned registry line now uses the required deferred form.

The reusable ignored driver is `local/mpkg_provider_ax/run_fixture.py --repo BUILT_REPO --attempt NEW_DIRECTORY --build-log BUILD_LOG`. Attempt04 passed three original/source comparisons and two source-only checks (1297 assertions). All eight bodies execute byte-identical from private pages. Existing actual string/base/singleton helpers are explicit source bridges; the actual raw manager and pooled string storage are created and drained. Win32 section calls use the real APIs.

Archive construction/destruction are explicitly recorded call boundaries in this fixture. The34h allocation is opaque marked storage, not a successful archive parser or valid loaded archive. Comparisons cover cold/fast factory publication and secondary registration, primary/secondary/base deletions, mixed-case suffix acceptance, exact original header forwarding, provider field/name ownership, stale fields after deletion, flags bit0, short/nonmatching/null-data/embedded-NUL/large-length decline cases, pool counters and allocation/free schedule. The fast getter is checked with an unreadable manager publication after factory publication. Every observed allocation is freed.

One source-only archive-construction exception verifies captured34h/18h allocation cleanup and base-name release. One source-only CRT registration-validation exception uses a deliberately invalid end bound on the genuine manager, observes retained factory publication and released physical section, then restores that test input for caller disposal. It is not a native exception test. Attempts01/02 preserve missing fixture-include compile failures; attempt03 preserves a mistaken registration-allocation trigger, after all three native/source comparisons and archive cleanup had already passed.

Before execution,509 files and123 actual linked objects were physically sealed, with each object equal to its current archive member. Sources, all dependent BSP headers, libraries, executable/map, probe/driver, native bytes and build evidence are included. All hashes remained unchanged, all files are read-only, and the after-manifest accounts for every execution output. The report records exact hashes and call rows. No installed archive, complete archive load, runtime startup or gameplay validation is claimed.
