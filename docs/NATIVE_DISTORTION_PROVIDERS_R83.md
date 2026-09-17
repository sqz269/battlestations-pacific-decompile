# Distortion capability and scene lifetime providers

Addresses: 00b20160, 00b724e0, 00b72430, 00b72580, 00924480, 00925430

## Result

R83 implements the complete 42-byte B20160 capability query and connects the
existing scene constructor/lifetime to the node runtime's raw string pool.
The scene's existing concrete weak-handle provider now accepts the application's
shared raw singleton manager, and that manager can dispatch D190B4 teardown.
The previous semantic string and singleton interfaces remain available.

The strict MSVC Win32 build and three existing CTests pass. One focused local
probe passes four original/source queries against real D3D9, six controlled
HRESULT cases, and six original/source scene constructor/lifetime cases.
No repository tests were added. Full B4F560 initialization and gameplay remain
unproved; this packet supplies dependencies for that continuing work.

## Capability query

B20160 spans [B20160,B2018A), takes ECX renderer and one stacked format, returns
the Boolean in EAX/AL and RET4. It loads current renderer+1990, the object's COM
table, and method+28, then calls CheckDeviceFormat with adapter0, HAL1,
X8R8G8B8 (22), usage80001h, texture kind3, and caller format.

The installed D3D9 header identifies 80001h as render target plus
QUERY_POSTPIXELSHADER_BLENDING; the probe statically checks that combination.
NEG/SBB/ADD accepts **only HRESULT zero**, including rejection of a positive
success value. It does not translate engine flags, cache a result or write the
renderer. Native arbitrary pointer faults and register/stack ABI compatibility
are not claimed by the new C++ interface.

The B4F560 caller loads the renderer at B4F602, passes format112 at B4F608,
calls at B4F60A and stores AL at distortion owner+260 at B4F60F.

## Scene name domain

Existing B724E0 [150 bytes] establishes the actual 24h outer scene: real weak
base, D62D48 profile, null root0C, zero string header10/14, null lighting1C,
zero20, name copy and live D7A24C scalar18. The raw branch now performs the
same self-copy guard, resize-preserve, source-length recheck, and current
destination-length/source-data/destination-data reload sequence. It copies
exactly that length; resize writes the terminator. Both domains use the same
owner and name header, without a substitute string or pool.

Constructor cleanup and B72430 [166 bytes] name destruction use that SAME
configured domain. Raw destruction runs full actual-header 41DD20, including
the getter and current return gate. B72580 [30 bytes] remains composed through
the existing flag1 zero-reference path; an independent flags0 source interface
was not added. Existing root and lighting fields still use host companions.
Changing the string domain does not establish binary-compatible scene pointers.

Both scene routines have two native unwind states. DFAAD4's map at DFAAC4
uses CC1B40/base and CC1B48/string; DFAB08's map at DFAAF8 uses CC1B60/base and
CC1B68/string. Original native FH3/SEH and raw getter failures during cleanup
were not exercised. The existing nonthrowing terminal interface terminates on
escaping C++ exceptions; broader exception equivalence remains open.

## Shared weak-owner lifetime

00924480 [189 bytes] uses the existing SoundLifetimeAccess projection, which
borrows either the old semantic manager or the actual01090AA0 publication cell.
CapturedSoundLifetimeSection captures the first manager's +10 critical section,
enters/increments it, and later leaves that same section. The second getter
resolves the current manager before reading the current0109CE90 publication
for registration. No separate manager, list, count or cleanup policy is created.

NativeSingletonDeletionBindings now appends weak_owner_domain at offset140;
all earlier offsets are unchanged and total Win32 size is144. D190B4's actual
slot0 is 925430 [55 bytes]. The new dispatch passes the popped mutex and flags
to the existing concrete deleter, which destroys its actual critical section,
clears the supplied0109CE90 publication and frees iff bit0. It does not require
the current publication to equal the popped object. Missing binding remains a
source contract error. The context must outlive the shared manager drain.

## Evidence and runtime scope

The collector reverified six complete bodies (632 bytes, including the 42 new
capability bytes), scene unwind maps/handlers, profile slots, constant and two
distortion caller spans: 874 selected live/PE bytes. All direct call rows are
checked mechanically; indirect calls are retained separately in the report.
Six existing/new Ghidra names receive appended evidence under the write lock,
preserving prior comments and force-refreshing the exports. No listing repair
was needed; the old B72580 post-free gap is already repaired in current Ghidra.

Real D3D9 accepts queried formats21/111/112/114 on this machine. A separate
recording COM fixture verifies exact arguments, untouched renderer storage and
results for S_OK, S_FALSE and E_FAIL in original and source bodies. That fixture
does not establish device behavior beyond the separately executed real queries.

Original/source scene cases use names of length0,12 and513. They compare all
24h bytes after normalizing name/weak-handle addresses, check name bytes and
terminator, retain an actual weak handle through scene deletion, check its null
target and remaining count, then release it. Source cases additionally retain
and release a scene reference before terminal deletion. All weak slots return;
the weak mutex and string pool register in and drain through the application's
same raw manager. Final D3D API references are zero.

Copied scene bodies use full-source weak-base/string-pool adapters; the weak
getter itself was source-executed, not native-differentially executed. The weak
pool uses explicit fixture storage on the shared E188B4 allocator list. The
probe does not establish process-static weak-pool startup/exit wiring. Root and
lighting fields remain null, and native EH/root branches trap if reached.

## Follow-up

Finish B4F560 initialization using these capability, raw string and weak-owner
providers with the existing camera/frame/post/material graph. Establish actual
weak-pool process lifetime and full scene root/lighting behavior as needed by
that path. Execute complete initialization/teardown, resolve the R82 early
failure concern, then bind B107F0 resource startup and validate gameplay.
