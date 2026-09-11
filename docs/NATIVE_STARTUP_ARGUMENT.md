# Native application initialization argument

Addresses: 008F81F0, 008F841E, 008F8429, 0073D410, 0073D4C2,
0073D933, 0073D945, 0073D94A, 0073CE20.

The configured executable's WinMain passes integer zero and the fixed string
`cachedload` to application initialization. After its 148h stack allocation,
0073D4C2 loads the second argument at ESP+150h into EBP. The initializer retains
that pointer, passes it to the native string duplicate at 0073D933, publishes
the duplicate at E1AE78, and calls the recovered switch parser at 0073D94A.
The entrypoint, stack and byte evidence is in NATIVE_ENTRY_BOOTSTRAP.md and
reports/native_entry_bootstrap.json. These are observations of the configured
image; other executables or DLL runtime patches are not established by them.

The reconstructed WinMain already passes the correct constant. GameStartupHost
previously discarded this argument and parsed GetCommandLineA instead. It now
forwards the same borrowed argument into its initialization phases and parses
it at the existing phase-3 location. The cache-dependent VFS factory tail thus
receives the native cached-load flag. Phase-2 mounts still precede parsing.
The parser's phase label in the host log is corrected to Phase 3.

The rebuilt executable's --frames, --log, --game-root and other host options
continue to use their separate GameExecutableOptions parser. This change does
not claim a complete original application initializer or native string/global
allocation ABI. The public host rejects a null mode string as an invalid typed
binding; the configured WinMain always supplies the nonnull constant.

Validation is recorded in reports/native_startup_argument.json. No original
installation file, saved game or live process is modified by this change.
