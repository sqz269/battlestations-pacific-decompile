# Native scene token text wrappers, CC12

`src/native_scene_token_text.cpp` supplies the complete ordinary schedules of
`008D9930` and `008D9980` over the accepted raw tokenizer providers. The new
interfaces take `NativeSceneTokenValueContext&`; they borrow an already-live
`0x838` tokenizer and return its own text storage. The TU is deliberately
unregistered for the integrator's review.

## Original contract and complete byte coverage

Live typed metadata verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, language `x86:LE:32:default`, image base `00400000`,
and modification number `10`. Both functions are returning, non-thunk bodies.
Current live prototypes are `undefined ...(void)` and cannot establish the ABI.
Assembly and the complete live byte captures establish the following instead.

| Entry | Original receiver and arguments | Result and cleanup | Complete coverage |
| --- | --- | --- | --- |
| `008D9930` | ECX owner; expected text at entry stack+4 | AL boolean; both exits RET4 | `008D9930..008D9976`, 71 bytes |
| `008D9980` | ECX owner; no stack argument | EAX token pointer; RET0 or tail peek | `008D9980..008D999C` and `008D99A0..008D99E8`, 102 body bytes |

For `008D9980`, the retained byte envelope is 105 bytes, ending exclusively at
`008D99E9`. `008D999B` is `EB 03`, an unconditional jump to `008D99A0`.
The excluded three bytes at `008D999D..008D999F` are `8D 49 00`,
`LEA ECX,[ECX]`. Disk decoding agrees with the live bytes. No reachable branch
in this wrapper enters them. These are skipped alignment padding, so no listing
repair is needed. The reachable `LEA ESP,[ESP]` at `008D99BC` is also a no-op.

The complete retained byte envelopes match the installed executable
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`
(12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`).

| Entry | Envelope SHA-256 |
| --- | --- |
| `008D9930` | `a736de888b2957f038b900b33fb8c640cf168023953a828f6cdebbf97dd4db7c` |
| `008D9980` | `0f1d747bb0f1f05c93a53fc7d336cde1e9e8cabb38f003e38cd5b93412a9d5f2` |
| immediate comparison `00438E10` | `94e8a5ab6688cce154e9a580bc5e0e08cfcc8be7af08e28c7d7be7ce29f4bea7` |

## Schedules and accepted direct providers

`expect_native_scene_token_008d9930` peeks through
`peek_native_scene_token_008d8a70`, then compares the returned text against the
borrowed expected text through `compare_insensitive_00438e10`.
The Original comparison takes ECX/EDX and returns an integer ordering: equal
pointers produce zero, null left produces -1, null right produces 1, and two
nonnull distinct pointers use `00BF7FBF` (`_stricmp`). The accepted Source is
`src/entity_identity.cpp`; its comparison uses the linked current CRT.
The wrapper tests only equality with zero.

On a match, the Original's inline forward copy reads `owner+5`, writes
`owner+405`, includes the terminating NUL, and then clears only `owner+805`.
The accepted `consume_native_scene_token_008d8960` in
`src/native_scene_tokenizer.cpp` has exactly this schedule. The wrapper then
returns true. A mismatch calls `recover_native_scene_whitespace_008d8f70` and
returns false, with no extra peek or consume.

`read_native_scene_token_008d9980` first peeks and tests `owner+80A`.
If that byte is zero, it reads every current-text byte through the NUL before
testing whether the scan was empty. Nonempty text succeeds. Empty text succeeds
only when the quoted byte at `owner+4` is nonzero. Success performs the same
consume schedule and returns the exact pointer `owner+5`. Failure calls
recovery, then returns the result of another raw peek. The Source retains the
Original's 32-bit scan-cursor arithmetic; it adds no `strlen`/CRT dependency.

The accepted recovery Source in `src/native_scene_token_values.cpp` skips only
following whitespace using the actual scanner cache, physical EOF, cursor, and
line state, stops on its configured immutable stop string (Original `00D15F34`
is empty), and copies physical EOF `+809` to token EOF `+80A`.
It retains the token cache `+805` and performs no consume, comment skip, or
logging. Thus mismatch must not be described as preserving the entire scanner
position. The failure re-peek in `008D9980` is retained even when the cache makes
it immediately return existing text.

These are direct calls into accepted raw Source, without replacement callbacks,
operation phases, or model-lexer ownership. Borrowed expected text may point at
tokenizer-owned text: its contents are compared after the initial peek. Returned
text remains borrowed and may change on later reads. As in the Original, valid
token contents must fit the `0x400` buffers and be NUL-terminated. Actual storage
may already be live or come from the accepted stream constructor; missing
file-backed constructor `008D9CF0` is a separate task.

## Static validation and remaining boundaries

The strict MSVC Win32 compile used
`/O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict /std:c++17` and passed with
no compiler diagnostics. Every emitted code section and all relocations were
inspected. The two wrapper COMDATs are `0x42` and `0x5E` bytes; helper COMDATs
are `0x0E` and `0x0B` bytes. All eight REL32 relocations name only raw peek,
consume, recovery, and the accepted comparison. The read wrapper retains the
full NUL scan, the subsequent quoted-byte test, consume on success, and
recovery followed by peek on failure. The expect wrapper retains peek,
comparison, and its separate consume/recovery branches.

The machine-readable report records the repository build's terminal result.
The build passed both existing CTest suites; it emitted LNK4006 for duplicate
`spawn_request_id_matches` definitions in existing Lua-spawn object files.
The new TU is covered by its strict standalone compile; the repository build
does not compile it until integration registers it. No tests or probes were
added, and no target process was started.

Local evidence is under `local/WholeSceneTokenTextSource02/`: complete live byte
captures and retained binaries, typed project/body metadata, strict object and
full dump/disassembly, build log, and `input_pins.DATA.json` with complete copies
and hashes of the direct Source inputs. These establish complete ordinary
Source and static build evidence. They do not establish an Original binary ABI,
private CRT/locale equivalence, class provenance, FS/EH behavior, runtime/game
equivalence, or application integration. Source uses new C++ calling conventions
and explicit contexts; the Original only specifies AL for the boolean result.
No Ghidra, ledger, shared header, CMake, or application wiring was changed.
