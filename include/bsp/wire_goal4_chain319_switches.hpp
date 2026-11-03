#pragma once

namespace bsp {
// Goal 4 chain 319: the scene property parser's E arm. A closed
// "key = E <table> : <symbol> ;" whose table and symbol the live library's
// native CEnum registry knows joins the native bag merge through 008F3A20
// (type 4: +28h the CEnum 0048E960 returns, +0Ch the 0048E840 value), and the
// consumers of a merged E property read that +0Ch instead of re-resolving it.
// OFF keeps E statements on the host tree and every consumer on
// PropertyLibrary::resolve_symbol.
inline constexpr bool kWireGoal4Chain319Bound = true;
}
