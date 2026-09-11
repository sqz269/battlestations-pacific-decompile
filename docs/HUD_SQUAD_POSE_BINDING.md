# HUD squad pose and live-list binding

Addresses: 006435D0, 00643781, 006437B4, 006437C2, 006437C7,
006437D8, 006437DD, 0064380D, 00643853, 00643873, 00414DB0.

The squad fragment uses the actual controlled owner at E188D8. This is distinct
from the displayed self-marker identity returned by 00927880 and stored in the
screen's field1C. Previously the reconstructed update refreshed and cached that
self-marker position once before iterating squad members. Native code refreshes
each node's payload first, then reloads E188D8 and refreshes that owner. It reads
both world matrices only after those refreshes, once for each member.

HudMarkersUpdateHost now supplies pure lookups of existing PoseRefreshView
objects. The update checks their actual C8 bytes and invokes the recovered
00414DB0 routine directly; it owns no validity flag or position cache. References
to both matrices survive the refreshes, including an exact same-pose alias.
The displayed self marker still uses its original separate identity.

The fragment captures the head at [[E188A8+19CC]+16C] once. Each iteration reads
node+8 as the member, performs refresh/measurement and marker callbacks, then
reads that same node's current next pointer at+4. The former count/index snapshot
is replaced by explicit live head/payload/next access. A callback may change the
next link; the current node must remain valid through the final link read.
The host must return existing owner and list identities without copied state or
fallback values. Malformed lists, disappearing required owners and async mutation
do not acquire a new successful behavior through this binding.

Each controlled-minus-member coordinate uses the native x87 subtraction and
float store. The radius path retains the original integer-to-float conversion,
extended intermediate products/sum, separate float stores for distance-squared
and radius-squared, and ordered strict comparison. In particular, it adds no
clamp for a negative radius and no NaN acceptance rule. This replaces ordinary
C++ arithmetic that allowed extra intermediate rounding.

The strict MSVC Win32 build and both existing CTests pass. One ignored fixture
uses actual canonical poses with different displayed/control identities, changes
the controlled owner and next list link during a marker callback, and verifies
the next iteration uses both changes. A separate section executes the original
70-byte radius instruction fragment with only a SETA/RET observation suffix;
18 comparisons cover six inputs under three x87 precision controls, checking
both the boolean result and exception-status bits. The fragment matches live
Ghidra and the configured disk image. reports/hud_squad_pose_binding.json records
the evidence and commands.

This closes the squad fragment's pose and list bindings within the existing HUD
update projection. GUI operations, unrelated sweeps, other HUD/minimap paths,
native owner ABI, unmasked FPU traps and gameplay remain separate validation.
