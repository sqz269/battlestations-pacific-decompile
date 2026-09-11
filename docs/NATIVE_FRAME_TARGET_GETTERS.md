# Native frame target getters

The complete original leaves are `B1F6D0` (11 bytes), `B1F6E0` (4 bytes) and
`B1F710` (4 bytes). They read actual borrowed storage directly, without
retention, validation, initialization or whole-owner construction.

Color reads the DWORD at owner+8+4*slot with native DWORD address wrapping.
Its original interface is ECX owner and a stack slot with RET4; the new
two-argument fastcall uses EDX slot and plain RET. Depth retains the native
four-byte `MOV EAX,[ECX+18h]; RET` body. sRGB retains the exact four-byte
`MOV AL,[ECX+3Ch]; RET` body, preserving non-Boolean byte values and leaving
the other EAX bytes outside its AL return contract.

Source passes the strict Win32 build, both existing CTests and focused
original-code verification. The frame-target owner lifetime and full binding integration
are separate dependencies. No game or visual validation is claimed.

## Primary integration

The primary verified all nineteen original bytes against fresh live Ghidra and
the installed PE, eight native seeds, the actual main library and its exact
archive object. All three complete COFF and linked leaf bodies match their
expected five/four/four bytes. One ignored fixture compares twelve original
and main-library calls using aligned and unaligned actual storage, wrapped
slot arithmetic and non-Boolean sRGB bytes with seeded upper EAX. All returns
match, all 128 input bytes are unchanged, and complete original-code and
fixture .text postimages match. Names and appended evidence are saved, prior
comments preserved, full ledger records registered and exports forcibly
refreshed. No permanent tests or owner-lifetime/gameplay claims were added.
