# AnimationChannels and Bone raw parser singletons (R24)

This packet reconstructs the two native 8-byte parser singleton families used
directly by application initialization. It adds explicit-service C++ for the
complete getter, primary scalar deletion, secondary deletion thunk and type-name
body for AnimationChannels and Bone. The parse-slot-8 bodies at `00B8A910` and
`00B8A990` are outside this packet.

## Recovered layout

| Parser | Getter | Publication | Initial profile | Primary profile | Secondary profile | Name target | Literal |
|---|---:|---:|---:|---:|---:|---:|---:|
| AnimationChannels | `00736DD0` | `01090298` | `CFEA08` | `CFEA38` | `CFEA34` | `00B8B050` | `00D633AC` |
| Bone | `00736EA0` | `0109029C` | `CFEA0C` | `CFEA48` | `CFEA44` | `00B8B080` | `00D633C0` |

Live bytes at `00CFEA34..00CFEA57` decode as these nine DWORD targets:
`00735D90`, `00737150`, `00B8B050`, `00B8A910`, `00735DC0`,
`00737190`, `00B8B080`, `00B8A990`, `00737680`. This packet binds the
first three entries of each four-entry parser table. The parse entries remain
unbound.

## Getter contract

Both getter bodies are complete 206-byte functions. A fast publication read
returns immediately. The slow path gets the actual singleton lifetime manager,
captures its `+10h` critical section, enters it and increments the native depth
at `+18h`, then rechecks publication. It allocates exactly eight bytes, writes
the transient initial secondary profile followed by the primary and final
secondary profiles, publishes, captures `parser+4`, performs a second manager
lookup and registers the captured secondary.

The native listings install handler records `00C860C8` and `00C860E8`, arm state
zero only after the lock/depth acquisition, and store the captured guard profile
`00CE37FC`. The source catch path destroys only that captured guard. A failure
during registration deliberately retains the published allocation.

## Names and deletion

The 33-byte name bodies ignore incoming parser `ECX` and construct the actual
mapped literal into the caller's raw owned-string output through `0041E870`.
`NativeResourceExtraParserNameCalls` handles only `00B8B050` and `00B8B080`;
it forwards every other captured target to the supplied existing finite map.

Each complete 58-byte primary scalar deletion clears its actual publication,
stamps `00CE3818` at the secondary and `00CFD7D0` at the primary, and frees the
captured allocation only when flags bit zero is set. The complete 8-byte
secondary entry subtracts four and tail-dispatches the primary body. The source
helper accepts only captured profiles `00CFEA34` and `00CFEA44` and uses the
same borrowed publication contexts used for construction.

## Evidence and boundaries

The two complete getter byte ranges, the live table range, all eight function
listings and their call targets were checked against the configured Ghidra
program `/battlestationspacific.exe`. A strict MSVC Win32 `/W4 /WX /fp:strict`
build passed with both tests at base `af9ce1480`; all eight export seeds matched
disk. The new code was also compiled by the integrating application build.

This is source-level behavior reconstruction. It does not establish original
callable ABI, FH3/SEH byte identity, hardware-fault behavior, parse-slot parity,
executable admission or gameplay validation. Detailed rows, original metadata
and validation results are in
`reports/native_resource_extra_parser_singletons_r24.json`.
