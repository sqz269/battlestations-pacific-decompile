# Native string duplicate Source C++ unwind review

**PASS for exactly the retained Source case.** Root's one recorded execution of the frozen ordinary caller and actual Core `duplicate_native_string_00438E40` produced allocator `std::bad_alloc`, reached the typed catch, destroyed the observation marker, preserved the four checked volatile caller words, and reported successful cleanup APIs. This closes that particular Source allocation-failure transport observation. It does not establish general Source, Original, hidden-constructor, Game, private-frame/register ABI, fault or activation behavior.

This independent review used baseline Main `03ef58daf98d9dff844e8a4039be830868d429b7` and only the twelve artifacts named by Root's immutable manifest. Manifest SHA256 is `d0f3b598e4b96a29985e4c47e863c47b9574e9cc437014fe14ba23e08e3a6b4e`; initial report SHA256 is `6d72455b2604754ce9ff0464413ae015aeb82c20c0b30a4df6ca27b8bc488d26`. Every file digest was verified before inspection and exact copies retained. Root artifacts, Source, address leases, shared metadata, Ghidra and mutable SCC candidate files were untouched. No compilation, rerun, target execution or new test occurred in this review.

## Whole fixture and self-only failure setup

The complete fixture commits a genuine `16MiB` `VirtualAlloc` input, fills its first `16MiB-1` bytes with `a`, and writes the final NUL before applying pressure. It records its own PID and creation FILETIME. It creates an unnamed job with default noninheritable handle, assigns only the current process, and requires the membership query to report exactly one assigned/listed PID equal to itself before setting a limit. It opens or changes no other process/job and launches no child.

The limit is the observed current private usage plus `4MiB`, with overflow checked. The fixture then installs `no_retry` through the real CRT `_set_new_handler`; this hook only records its call/size and returns zero. It does not replace `malloc`, replace the canonical allocator, or throw a fabricated exception. The actual duplicate call appears once inside a C++ try containing the observation marker. Typed `std::bad_alloc` and catch-all are distinct. Ordinary return, another exception, wrong handler count/size, wrong marker counts or failed observations force failure.

After the catches, the fixture checks all four volatile caller words, submits zeroed job-limit information, restores the captured prior hook through the setter, checks three input endpoints, frees any unexpected successful duplicate through the real matching Source free, releases the input mapping and closes its job handle. Early setup failures use `not_run` and attempt input/handle cleanup. These unexecuted branches were reviewed statically; they are not additional successful run cases. The own-process error-mode setting and job assignment are not claimed to have been restored/detached before exit.

## Actual retained code and providers

The frozen build command targets Win32 with C++17, `/EHsc /MD /O2 /fp:strict /W4 /WX`, links the existing Core/Lua/zlib archives and embeds an `asInvoker` manifest. It is an ignored focused probe, with no production policy/guard change. The command was read, not executed by this reviewer.

Independent COFF parsing and preferred-VA PE relocation resolution match **all bytes and all 105 relocations in nine complete selected Source/EH sections (`1,836B`)**, including the whole main section, real marker destructor/no-retry hook, Source duplicate/allocator/free and physical EH records.

| Retained implementation | Physical result |
| --- | --- |
| Core duplicate | Complete `57B` at preferred VA `00401670`. Its two REL32 words are at offsets `22h/2Ch`, targeting real Source allocation helper `00401630` and the `memcpy` import thunk `004024BE`. All other49 object/PE bytes agree. |
| Source allocation helper | Complete `58B`; builds the real size request and calls canonical `singleton_lifetime_allocate` at `00401790`. |
| Canonical allocator | Complete `90B`; uses the `malloc` import, then `_callnewh` with the same requested size on null. A zero handler result selects the actual `std::bad_alloc` construction/`_CxxThrowException` path. No CRT/vendor function body was inspected or reconstructed. |
| Matching Source free | Complete `6B` forwarding to the real `free` import. The duplicate allocation failed in this run, so successful-result free and `memcpy` were not reached. |
| Caller and EH | Whole `1,420B` main section plus the marker/hook/EH sections match the frozen PE and their objects, including all references. The standalone marker constructor was inlined/discarded; its actual main counter store is retained. |

The PE statically imports `VCRUNTIME140.dll` and the UCRT API sets, including `malloc`, `_callnewh`, `_set_new_handler`, `_CxxThrowException` and `__CxxFrameHandler3`. This proves retained link routing, not the identities/versions of modules loaded during execution. Loaded CRT identities were **not captured**. Actual runtime ASLR addresses were also not retained; addresses here are preferred static VAs.

The frozen preflight records four production Source/Git pins and reports the earlier49-byte Native noncall comparison. This review verifies the retained Source objects, final PE and real targets. It does not freshly read mutable production Source/archives or the Original PE outside the permitted manifest. No new Original-body or ABI credit follows from this case.

## Physical C++ unwind

The PE's actual FuncInfo at `004038D4` contains magic `19930522`, maxState3, one try block and EH flags1. The try map is `tryLow0/tryHigh1/catchHigh2` with two handlers. State1 maps to state0 through unwind funclet `00402620`, which reaches the real marker destructor `00401000`. The typed handler points to the actual `std::bad_alloc` type descriptor; the allocator's throw metadata contains that same descriptor. The other handler is catch-all.

At `0040136F`, main arms state1 immediately before the actual Source duplicate call at `00401373`. Constructor/normal-return/destructor/catch sites are distinct. A typed exception with `normal=0`, marker counts1/1 and the real unwind action demonstrates this one C++ scope unwind; the marker was not manually incremented in a catch to simulate cleanup. Physical stores/checks for the four volatile words remain at EBP offsets `-58h/-54h/-50h/-4Ch`.

Those four words are four checked caller locations. They are **not** proof of the entire stack, nonvolatile-register restoration, arbitrary caller ABI, native private cleanup frames or Original FH3/SEH behavior.

## Recorded execution and restoration limits

The immutable log and report agree on PID `64572`, creation FILETIME `134378080556956158`, one run and tool exit0. They record:

| Observation | Recorded value |
| --- | --- |
| Input/allocation request | `16,777,216B` |
| Private usage / process limit | `17,813,504B /22,007,808B` (exact `4,194,304B` spare) |
| Typed catch / other catch / normal return | `1 /0 /0` |
| Real no-retry hook calls / requested bytes | `1 /16,777,216` |
| Marker constructed / destroyed | `1 /1` |
| Four caller words / three input endpoints | Both checks passed. No whole-buffer/stack/register claim. |
| Limit restoration / hook restoration | Limit API returned success; hook setter returned `no_retry` while installing the captured previous hook. |
| Input release / job handle close | Both APIs returned success. |

There is no post-restore limit or hook readback, no captured pre-exit job detachment, and no claim that closing the handle immediately destroyed the job object. `CloseHandle` success and actual process termination are recorded. The log's zero restore error is the fixture's success value after the limit API succeeded. No other process's policy was changed by the fixture. This is one controlled no-retry/private-limit case, not handler retry/reentry/throw, every OOM, every Source caller or a portable frozen-runtime attestation.

Only this document and `reports/cc16_native_string_duplicate_cpp_unwind_review.json` change. The result adds no production Source implementation, general Source/Original admission, vendor reconstruction, hidden constructor, full Game ownership/activation, ABI or gameplay credit. All broader gates remain.
