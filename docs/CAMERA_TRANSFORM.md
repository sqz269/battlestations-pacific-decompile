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

Parent pointers must refer to live objects in an acyclic graph. Editing local or
parent fields does not automatically invalidate children or camera caches in this
interface. Native setters, dirty callbacks, hierarchy ownership and construction
remain separate work. No cycle handling or guessed dirty propagation is added.

The existing probe initializes parent localZ=-0.75 and child localZ=-0.25, both
with invalid caches. The combined accessor produces worldZ=-1, viewZ=1 and sets
the expected world/view/projection/combined validity bits. Its uploaded result
renders centerFF407FBF/outsideFF000000 with state restoration. The optional native
camera comparison now includes affine composition on the existing scaled/rotated
fixture; all16 words match. Win32 build, two existing CTests and the full D3D9
probe pass. No new test target was added. These checks do not prove dirty edit
behavior, arbitrary hierarchies, exceptional FP inputs or game execution.
