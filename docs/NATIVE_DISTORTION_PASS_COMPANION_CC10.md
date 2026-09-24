# Distortion pass canonical lifetime bridge

This extends the existing host-only `NativeRenderPassReference` to the actual
distortion profile `D61F1C`. It is not a newly recovered native function.
The first two current profile entries are `BD30E0` and `B4F540`; all eight live
Ghidra bytes match the installed PE. The original deleting entry takes ECX owner
and stack flags, returns the original owner identity, and executes RET4.

Binding retains the genuine allocation's atomic count at +4 and the SAME
canonical registry used by the effect's children. It does not retain, change
any native bytes, create a replacement counter, or register nested holders and
surfaces. A distortion owner requires an explicit `NativeDistortionLifetimeContext`
whose `effects` member is the exact same object as the companion context's.
All existing raw camera, frame, scene and post-effect lifetime contracts apply.

At zero, the bridge checks current virtual0 for BD30E0, rereads the current
profile/deleting slot, and calls the existing full B4F540 path with flags1.
It performs no second decrement. The raw deleter releases children and records,
executes the base teardown and frees the owner; only then does host metadata
retire and unbind, without accessing freed storage. A terminal failure is fatal
under the existing noexcept registry interface. The companion/context must stay
address-stable through terminal callbacks and external quiescence.

B4F0C0 leaves +34 camera, +38 frame and +3C scene untouched. In particular,
B4F560 may return false at capability checks before writing them. Constructor
completion, registration, and a false result do not establish valid cleanup
storage. The bridge neither initializes nor certifies those fields. Integration
of that native failure path requires an independently established valid preimage.
For initialization, the lifetime context must borrow the SAME persistent
`NativeDistortionInitializationBlock::scene_owner_3c()` publication by reference;
a snapshot is insufficient. The publication can become stale after deletion.

Strict MSVC Win32 Release and three existing CTests passed. One ignored actual
Direct3D9 lifecycle probe also passed, retaining the earlier bright-pass case.
The distortion case explicitly seeds valid null camera/frame/scene preimages
before executing the raw constructor, then supplies a genuine texture holder,
separate frame, record allocation and canonical post-effect held by two actual
owner fields with two genuine credits. It verifies all 26Ch bytes and the count
are unchanged on binding, missing context is rejected, nonfinal release preserves
children, final release retires both companions, resource tracking is restored,
the registry is empty and final device/API references reach zero.

That fixture preimage is not an allocator or production-initializer claim. The
probe excludes populated camera/scene teardown, original/source wrapper parity,
allocation failures, native ABI/FH3/SEH, full renderer initialization and game
validation. No new tracked tests or application binding are introduced. Evidence
and hashes are in `reports/native_distortion_pass_companion_cc10.json`.
