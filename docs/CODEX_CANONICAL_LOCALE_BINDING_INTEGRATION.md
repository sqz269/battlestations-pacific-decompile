# Canonical locale owner binding integration

This batch supplies an explicit Source owner binding for the raw locale manager.
The new owner remains separate from the semantic locale host. Preparing it does
not initialize or load the manager, and no application startup path calls it.

The permanent string process appends the actual F8BC4C publication and a one-time
locale domain claim. The existing AA0 and three raw string pool cells keep their
identities. The singleton deletion table appends a borrowed typed locale service
at offset 220; all older fields keep their offsets and the table size becomes 224.
Core knows only the small interface, while the concrete owner belongs to bsp_game.

The finite CFF140/CFEA5C dispatcher passes the captured receiver/profile and its
actual caller-owned flags object by reference. An absent locale binding retains
the previous rejection behavior. Preparation retains the owner pointer before
binding so a failed preparation cannot bypass the host's pre-drain fence. Normal
and fallback shutdown consult that fence before the raw manager drain; closure
is checked after the real drain and manager publication clear.

Preparation requires the host's genuine raw-string deletion bindings to have
already been installed by the real VFS owner services. A bare host does not meet
that prerequisite. Preparation creates no substitute services or loaded locale.

The coupled map insertion revision is 3ed87041c0b1d35a2dacac3cd76452b8ebb74eee.
It uses the real node allocator and raw key-string context. Duplicate lookup
returns the existing wide-value header without allocation. A missing key increments
the native count before allocation and preserves the two current-head reads when
linking. Only an exception escaping the actual key constructor selects the real
slot return on the privately captured pool/slot. Other failures retain their
effects; no count or payload rollback is invented. Its additional canonical-pool
admission observation is explicitly qualified as Source behavior.

The shared dispatcher and string-process translation units pass an independent
MSVC Win32 object build with /W4 /WX /permissive- /fp:strict. The dispatcher object
is x86 (14C) and has no undefined concrete Game locale-owner symbols. The concrete
owner revision is b8b71e79e15d336fc76ed63dda2058b5a8f4ccfc. Its fixed strict Win32
object check also passes. Root reviewed the complete owner; the worker applied
the requested nonnull-parent admission check before initialized borrowing and drain.

The combined Release Win32 build at 0616ca399e7146c0a61cc8959af461383d722df2
links bsp_game and the Core-only math target successfully. Both existing CTest
checks pass (2/2, 11.50 seconds). No locale owner body or game run was invoked.
Independent owner and map reviews accept the exact frozen Source code, with no
scoped code blocker. Receipts are under
local/canonical_locale_owner_integration_20261030/ and the frame review tree's
local/raw_locale_owner_boundary_review_frame_20261030/owner_source_review/.
No Source readiness, Original ABI compatibility, loaded tables, game execution,
or raw Text/startup activation is established. The nine raw Text choices remain
false, and the frozen EMPTY runtime candidate retains its source-admission hold.

The next bounded loading dependency is a raw-context 004C53E0 wide-resize bridge.
The existing actual-header storage interface borrows the same cells but makes
release noexcept; its getter-failure boundary must stay explicit. Full AA0020
also needs genuine VFS override/string-vector transport and raw16 growth. The
STL-shaped A9FD50 body remains outside vendor reconstruction credit.
