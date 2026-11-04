#pragma once

namespace bsp {
// Goal 4 chain 343. The gunnery host's ship death messages from 00827A90 (9Ah explode to
// parts, 98h wreck) and the hull segment detach 99h from 0092CED0 travel through 0076E520's
// local copy (writer, 00768530's constructor, reader); 00780120's receiver ladder asks the
// copy's IsType, and the unit's 00821E80 arm acts on the copy. OFF keeps the effects applied
// at the send as before.
inline constexpr bool kWireGoal4Chain343Bound = true;
}
