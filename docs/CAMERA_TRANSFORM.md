# Camera world and view caches

The typed CameraTransform stores parent, validity flags, local/world/view matrices;
CameraState combines it with the existing projection state and view-projection
cache. This reconstructs the operations below with new C++ ownership and ABI.
Zero initialization is an interface default, not a native constructor claim.

- 00b6db70 always refreshes its own world. With a parent, it first refreshes that
parent only when its world-valid bit2 is absent, then composes local*parentWorld.
Without a parent it copies local to world through004134f0. It then sets bit2.
- 00b6fcb0 returns cached view when bit8 is set. Otherwise it ensures world,
inverts into a separate temporary, copies that to view and sets bit8.
- 00b70490 returns cached view-projection when projection flags bit10h is set.
Otherwise it ensures view and projection, multiplies view*projection into a
temporary, copies the result and sets bit10h.

The public copy helper retains the sixteen native sequential x87 load/store
pairs. Affine composition retains its native instruction order and forces the
last column to0,0,0,1; its destination must not overlap either source. World/view
refresh uses disjoint matrices. Original camera function ABIs and offsets are in
the preceding camera analysis documents; body hashes are in
`reports/camera_transform.json`.

Parent pointers must refer to live objects in an acyclic graph. Direct field edits
do not invalidate caches. The subsequent local-matrix setter port supports native
selective descendant invalidation and the direct camera override; see the local
edit follow-up below. Parent changes, hierarchy ownership and construction remain
separate work. No cycle handling or guessed ancestor camera invalidation is added.

The existing probe initializes parent localZ=-0.75 and child localZ=-0.25, both
with invalid caches. The combined accessor produces worldZ=-1, viewZ=1 and sets
the expected world/view/projection/combined validity bits. Its uploaded result
renders centerFF407FBF/outsideFF000000 with state restoration. The optional native
camera comparison now includes affine composition on the existing scaled/rotated
fixture; all16 words match. Win32 build, two existing CTests and the full D3D9
probe pass. No new test target was added. These checks do not prove dirty edit
behavior, arbitrary hierarchies, exceptional FP inputs or game execution.

## Local edit follow-up

The typed transform now includes first-child/next-sibling links, auxiliary flags
and an explicit optional notification function/context. This models the native
attached+A0 virtual+3C call boundary; it does not implement group ownership.
`set_transform_local_matrix_00b6db10` copies first, then only when flags&A is
nonzero clears own flags, notifies, and invalidates world-valid descendants.
`invalidate_camera_descendants_00b6da30` skips entire already-invalid subtrees
and leaves camera projection/combined flags untouched.

`set_camera_local_matrix_00b71430` clears camera flags withFFFFFE4B before the
base setter, then refreshes world direction and target through00b70660. The latter
retains x87 operand order and intermediate stores, including position-first X
addition versus direction-first Y/Z. These semantic names are provisional.
Position setters, reparenting and callbacks for arbitrary group types are
still unported. A direct ancestor edit must not be described as automatically
invalidating descendant camera combined caches.

The existing draw fixture now changes child localZ from-0.25 to-0.5 after caches
are valid. Its supplied notification observes own flags cleared, projection8
preserved, VP10 cleared and the descendant still valid; afterward that descendant
is invalidated. DirectionZ=1, targetZ=-0.25 and rebuilt viewZ=1.25 pass. The
regenerated view-projection renders the expected center/outside pixels and
restores state. Win32 build, both existing CTests, native matrix comparisons
and full D3D9 probe pass. No new test target was added. Evidence is in
`reports/camera_local_edit.json`; attached cGroup callback analysis is in
`CAMERA_ATTACHED_CALLBACK.md`. This is one explicit callback/hierarchy fixture,
not native callback ownership or gameplay validation.

## World edit follow-up

`derive_camera_local_from_world_00b6e7e0` copies world to local for a root; a
parented transform ensures the parent inverse cache, multiplies world by that
inverse and copies the result to local. `set_transform_world_matrix_00b6e870`
copies world, notifies the attachment, derives local, invalidates descendants,
invokes its required virtual40 callback, then assigns validity flags=2. The
callback argument explicitly represents dispatch; it is not an optional stub.

The camera override00b71460 clears camera flags withFFFFFE4B, calls that base
setter with the known00b6dbe0 callback, then refreshes direction/target. That
callback clears auxiliary bits10/20 and notifies the attachment again. The two
notifications see the old transform flags; the final flags=2 assignment happens
after both. The explicit callback/context interface does not implement native
cGroup ownership or resolve all other virtual overrides.

The existing parent-camera fixture now follows its local edit with worldZ=-1.5.
Its first notification sees prior localZ=-0.5 and a valid descendant; its second
sees derived localZ=-0.75, an invalidated descendant and cleared auxiliary bits.
Both see old world/view validity and projection8 preserved/VP10 invalid. Final
flags2, parent inverse validity, targetZ=-0.5 and rebuilt viewZ=1.5 pass, followed
by expected pixel readback and state restoration. Build, both existing CTests,
all existing native matrix comparisons and the full D3D9 probe pass. No new
test target was added. Byte evidence is in `reports/camera_world_edit.json`.
Position/view setters, reparenting, full group ownership and gameplay remain
unported; no arbitrary hierarchy or exceptional-FP claim follows.
