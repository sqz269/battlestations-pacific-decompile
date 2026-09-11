# Private Microsoft XLive runtime verification

Addresses: game IAT00CE25DC / thunk00C2F1D2, XLive ordinal5030.

The Microsoft GFWL 2.0.0673.0 DLL successfully loads in a standalone Win32
process when its matching legacy credential dependency is explicitly loaded
first by absolute path. An actual ordinal5030 `WM_NULL` call returns FALSE.
This is loader/message-call verification with the SDK uninitialized; it does
not establish account, network, overlay, renderer or gameplay behavior.

The installed `C:/Windows/SysWOW64/msidcrl40.dll` is a 15,872-byte Windows
10.0.26100.1 module lacking ordinal43, `ExportAuthState`, which XLive imports.
A debugger recorded Windows selecting that system file even when a matching
private file accompanied the caller-selected XLive DLL. That attempt produced
STATUS_ORDINAL_NOT_FOUND (C0000138), exposed as Win32 error182. Explicitly
preloading the private file selected the intended module and resolved loading.
The precise reason Windows preferred the system copy was not established.

The required files came from the game's bundled
`redist/gfwlivesetup.exe`, SHA256
`82b3088d0af4bad22bf8058659fb80654040fbafdccbd0fc8df996153f597217`.
The wrapper was not recognized as an archive. Offline PE/byte inspection found
a valid CAB at file offset0x8C00, length178584234. Extracting that exact range
and opening it with 7-Zip exposed `pkg/xLiveRedist.msi`; its `XLive.cab` contains
the runtime files. The installer and MSI were never executed.

The extracted files remain under ignored `local/gfwl-private-runtime/`:

| Private filename | Version | SHA256 |
| --- | --- | --- |
| xlive.dll | 2.0.0673.0 | 79da26ab6b2dc25936c3354087de0ba41da1a8b62924972e9100c29b95d34385 |
| msidcrl40.dll | 5.000.737.6 | 623cb6ca98e566357abbd0e76b15713921e1d7e1144c0c4f589a0407c7eff1ee |
| ppcrlconfig.dll | 5.000.7498.00 | 649557d6349ea0658808952c1bbf9a111ec2283c29345c7f904919fd599e5d61 |
| xlivefnt.dll | 2.0.0673.0 | a07e20c09a0c3eac2ed3e6288d67060e82b70595053153866b1cfb4f9958f9a4 |

All four extracted files passed Windows Authenticode verification with Microsoft
signers. XLive is hash-identical to the existing Microsoft system copy. The
credential DLL exposes ordinal43. Only `msidcrl40.dll` was explicitly preloaded;
no account or authentication API was called. No game/Windows file was changed.

`XLiveLibrary` now accepts an optional ordered list of absolute dependency paths.
It holds those modules until after XLive unloads, and releases them on failed
construction. It does not change the process DLL search path or silently select
an installed credential module. The caller owns the choice of runtime and paths.

Evidence: `local/gfwl-redist-inspection.json`,
`local/gfwl-private-runtime-identities.json`,
`local/xlive-redist-load-debug.txt`, `local/xlive-private-preload.out` and the
composition report. These are local provenance/probe artifacts, not committed
Microsoft redistributables. The previously tested game-local replacement still
failed during loading; its separate failure cause remains unresolved.
