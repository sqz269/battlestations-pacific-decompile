# Raw native online startup SDK boundary

This packet binds the SDK/Win32 calls made by the actual 3F0h manager
constructor 00A40DF0. It does not reconstruct that parent body or the SDK
implementations. NativeOnlineStartupSdkRuntime borrows an already-loaded
XLiveLibrary and resolves the original named ordinal imports once. Its
secondary constructor accepts the same typed, already-resolved import table.
Neither constructor calls an SDK function or creates a listener.

| Parent site / target | Import | Native argument stack | Return used by A40DF0 |
| --- | --- | --- | --- |
| A40F31 / KERNEL32 IAT CE22A8 | GetUserDefaultLangID | none | low 16 bits written to init-info+10 |
| A40F46 / A4D5DE | XLiveInitializeEx, ordinal 5297 | 1Ch info pointer, DWORD 20029900h | ignored |
| A40F63 / A4D5D8 | XOnlineStartup, ordinal 5310 | none | ignored |
| A40F72 / A4D5D2 | XWSAStartup, ordinal 1 | DWORD 202h, pointer to caller's 400-byte WSADATA | ignored; output version read regardless |
| A40F84 / A4D5CC | XWSACleanup, ordinal 2 | none | ignored |
| A40F8E / A4D494 | XSocketNTOHS, ordinal 38 | DWORD 0C02h | full EAX pushed to next call |
| A40F94 / A4D5C6 | XNetSetSystemLinkPort, ordinal 84 | one DWORD from prior EAX | ignored |
| A40FD4 / A4D5B4 | XNotifyCreateListener, ordinal 5270 | DWORD areas 2Fh, DWORD flags 0 | raw handle stored at manager+1C |

Each A4Dxxx target is a complete six-byte JMP [IAT] thunk in the installed
binary. The adapter retains the correct SDK function names and stdcall argument
slots. It returns raw SDK result bits even where A40DF0 discards them. Its
x_wsa_startup directly passes the caller's actual NativeOnlineWsadata400
pointer: failure cannot throw, replace the buffer, zero its preimage, or hide
partial SDK writes. A40DF0 checks version_00 at A40F77 even on failure and
calls cleanup only when those bytes differ from 0202h. The parent owns that
check and the manager mutation order.

The caller constructs a zeroed NativeOnlineInitializeInfo1c, writes size
1Ch, and obtains device+8 through 00B1FEF0, which reads the current renderer
at +1A10. It independently reloads F8D394 at A40F23 and supplies the
same mutable renderer+1A28 pointer at info+C. The adapter never copies
render parameters. GetUserDefaultLangID fills the low word at info+10.
00B1FEF0 and 00A40DF0 are read-only dependencies owned by the parent.

The focused fixture uses typed recording imports, verifies failed/partial
WSADATA writes and full DWORD forwarding, and makes no real SDK/network call.
The exact call sites, IAT slots, original PE spans, build, fixture and runtime
manifest are recorded in reports/native_online_startup_sdk.json. Ghidra was
read only. This C++ interface is not a native binary-ABI replacement, and no
game startup was validated.
