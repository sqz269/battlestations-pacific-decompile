# Package scan startup and priority policy

`0073cb10` performs a fresh VFS enumeration of directory `.` with extension
`mpkg` and flag 0, consumes the resulting names in order, skips names already
represented by a mounted provider, and requests a mount for each remaining
name. Startup calls it twice consecutively, before registering the asset
search directories in `00738360`. Neither invocation sorts package names.

The bounded C++ helper `package_mount_priority_0073cb10_fragment` implements
only the filename-derived priority supplied by this scan. It does not discover
packages, mount files, implement a package format, or choose provider priority.

## Invocation contexts and original ABI

Application initialization `0073d410` runs initial manager/factory/mount setup
only when global `0109ceec` is null. After those initial mounts:

```text
0073d87b MOV ESI,[ESP+2Ch]
0073d87f MOV ECX,ESI
0073d881 CALL0073cb10
0073d886 MOV ECX,ESI
0073d888 CALL0073cb10
0073d88d OR EBX,-1
0073d890 MOV ECX,[ESP+2Ch]
0073d894 CALL00738360
```

Both scans receive the same application pointer in ECX; the scan never reads
the incoming ECX before replacing it with local object addresses. It takes no
stack arguments and ends in plain RET at `0073ce15`. It uses global manager
`0109ceec` for query and mount operations. The two calls are not distinct
archive-versus-loose modes. Each reconstructs a query result after any mounts
created by the preceding call. It is an inference that this can discover
packages exposed by newly mounted providers; the reason for exactly two calls
is not encoded in the scan's observed body.

Saved Ghidra disassembly originally stopped at the `_free` call `0073cdfb`,
due to its incorrect no-return propagation. Raw bytes show the SEH/register
restoration and RET through `0073ce15`. The full body is 774 bytes, not merely
the range ending at the free call. No Ghidra metadata was changed in this task.

## Enumeration and duplicate handling

At `0073cbcf`, manager `00bdd990` receives native strings `.` and `mpkg`,
flag 0, and an output list. It builds callback `00bdbe20`, primary table
`00d6846c`, then calls mount traversal `00bdd0a0`. The callback's `+8` routine
`00bdbeb0` is the three bytes `32 c0 c3` (`XOR AL,AL; RET`), so enumeration
does not stop after one provider reports results.

Callback `+4=00be1130` uses provider pointer at mount record `+8` and dispatches
provider virtual `+14` with suffix, extension, flag and a temporary vector.
Assembly `00be1160..00be1183` establishes those arguments despite malformed
decompiler locals. It visits the returned vector's entries in ascending index
order and forwards each copied string to `00be0fc0`.

`00be0fc0` searches the output list using `00bdbfd0`. Equal length and
case-insensitive `_stricmp` equality reject a duplicate filename and emit
`FIND:rejected duplicate filename %s`. Otherwise it appends before the list
sentinel. Consequently the first spelling encountered survives, with order
determined by mount traversal followed by each provider's own enumeration
order. It is not a sorted filename set. Physical/archive enumeration may
have different orders; those provider implementations remain separate.

Scan helper `00557a90` copies and removes the **front** entry. After computing
its priority, scan calls `00bdb120` at `0073cd19`. This walks mounted tree
records, comparing the provider's native system-name string `+8/+C` with the
enumerated name by length and `_stricmp`. An existing provider is returned
and the scan skips mounting that name. This check is separate from query
deduplication and does not compare basenames or the mount's virtual prefix.

For a new name, call `0073cd6e` sends manager `00be1890` these five arguments:
`(enumerated name, ".", computed priority, 0, -1)`. It does not inspect a
package header before this call. Factory selection and provider construction
belong to the manager path; the scan itself cannot establish a package format.

## Priority expression and typed port

`0073cc75` initializes EDI to decimal 1000. The code performs a case-insensitive
substring search for `patch`, but modifies the priority only if the first
match is at offset zero (`0073ccb4 TEST ESI,ESI; JNZ`). It then takes the whole
suffix after five characters, calls native `atol`, and adds decimal 1000 with
32-bit `ADD EDI,3E8h` at `0073cceb`.

This is the **entire enumerated name**, not an extracted basename: a name such
as `folder/patch2.mpkg` retains priority 1000. `patch2.mpkg` yields 1002;
`PATCH-2.mpkg` yields 998; a leading `patch` with a nonnumeric suffix yields
1000. These are deductions from the expression, not new executed test cases.
There is no special numeric parser requiring the rest of the filename to be
digits and no extension removal before conversion.

The native library chain is:

```text
00bf8417 JMP00bf83f1
00bf83f1 PUSH10; PUSH0; PUSH[ESP+0Ch]; CALL00c03468; ADD ESP,0Ch; RET
00c03468 Visual Studio2005 _strtol -> strtoxl(...,radix,flags0)
```

The port uses Win32 `std::strtol` with base 10 and null end pointer. This keeps
optional leading whitespace/sign, a decimal prefix, ignored trailing text,
and signed-long saturation on conversion overflow. The subsequent addition
is explicitly uint32 wrapping, with final bits copied to int32 to avoid C++
signed-overflow undefined behavior. The host uses its current CRT locale and
error state; this does not reproduce the original CRT locale object or errno
storage. `static_assert(sizeof(long)==4)` prevents accidentally claiming LP64
`strtol` behavior is equivalent.

The typed interface rejects non-ASCII, embedded-NUL and oversized strings
before changing output. It deliberately does not require `.mpkg`, because
that filter belongs to the earlier provider query. It collapses the native
substring search to a leading comparison: later matches never alter priority.
The priority helper's host bool reports domain acceptance, not mount success.

## Provider factory registration call order

The physical-directory factory is registered inside `00beda60`, called at
`0073d637`. Then startup performs:

| Registration | Singleton getter / constructor | Primary table | Factory method +4 |
|---|---|---|---|
| `0073d675` | `004fc150` / `00be5320` | `00d688b4` | `00be8120` |
| `0073d688` | `00736a90` / inline construction | `00cfea14` | `00bb9d90` |

Both call manager registration `00be0660` with the getter's return. Table
bytes are respectively `90 57 be 00 20 81 be 00` and
`90 70 73 00 90 9d bb 00`. Getter `004fc150` publishes singleton `0109db68`;
`00736a90` publishes `010904f4`. This table identifies concrete targets without
assigning a file format to a factory based on its call position. Effective
factory iteration order still depends on `00be0660` insertion semantics.

The initial mounts and five-argument `00be1890` ABI are documented in
`VFS_SEARCH_REGISTRATION.md`. This scan supplies priorities but does not prove
whether larger or smaller values win; that belongs to `00be1740` tree
insertion and traversal. The coordinated mount audit owns that conclusion.

## Installed snapshot and evidence

A read-only immediate-root enumeration of
`I:/SteamLibrary/steamapps/common/Battlestations Pacific` found zero loose
files ending `.mpkg`, `.zip`, or `.pak`. This installation contains many loose
asset directories and unrelated local modification artifacts. Zero matching
root files is not proof of zero provider-visible packages: `filestore`,
mounted archives, or other registered providers may return names not visible
as immediate physical files. No package headers were invented or decoded.
The exact root and empty list are recorded in ignored
`exports/bsp/package_startup/installed_root_packages.json`.

Every live batch verified `bsp`, `/battlestationspacific.exe`, x86 LE base
`00400000`. Saved-image bytes matched installed PE bytes for these spans:

| Start | Bytes | SHA-256 |
|---|---:|---|
| `0073cb10` complete scan | 774 | `d6f12a9f69158bb91207a30f83b1c56f532ecd830274fc17c7684d3bad51f2fa` |
| `0073cc75` priority evidence window | 153 | `0eab8a5103c892314e2aaeba0f9760074eb4a4883efdf435ee6228cda93f5273` |
| `0073d66d` factory registration calls | 32 | `ce71823b1e5863c95224cd76ff2b3ae3d41eeb78814b770e58754766e3794d31` |
| `0073d87b` two scans and defaults call | 30 | `e197032a1690a98ffc715b7d213f7ad092bd852743693905089ebac851c20f1b` |
| `00bf83f1` complete atol adapter | 17 | `27cb0873efea1943347af52c633464a9d343363040d8e2526438d239d8920368` |
| `00bf8417` complete thunk | 5 | `80a233c3d4c7f870b40bf35f81f3efc6333bca5668c83d9c1ea69fea41b4760f` |
| `00bdbeb0` complete continue query | 3 | `01cb47d078b4841b8408ec4fe278efa83115c6f6e101972987507d2b2b57dcf0` |
| `00be1130` provider-dispatch/vector-loop window | 168 | `9431e7bb1499fb9f480e1222b76050a1f60015f9cb44b09e5a2311e6edaef098` |

The coordinated `VFS_MOUNT_REGISTRATION.md` and `PROVIDER_FACTORY_STARTUP.md`
now establish mount and initial factory ordering. Remaining dependencies are
provider enumeration/filtering, actual archive contents/lifetimes and complete
startup integration. No builds, tests, shared
metadata edits or Ghidra mutations were performed by this bounded task.
