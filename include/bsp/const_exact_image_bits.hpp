#pragma once

namespace bsp {

// cc13-fix67 (fix7), OFF. One family switch for the ulp-level constant corrections that
// docs/CC13_CONST_EXACT.md listed (cc9 grant 2026-10-17): float gates and clamps a few ulp
// off the image's float, and doubles written as short decimals where the image holds a
// widened float (0.85 where the qword is 0.8500000238418579). Each affected constant selects
// `kConstExactImageBitsBound ? <image bits> : <old value>` at its declaration and names the
// image bits and address in its comment; docs/CC13_FIX67.md lists them. With the switch OFF
// the compiled behaviour is exactly the old one.
inline constexpr bool kConstExactImageBitsBound = false;

}  // namespace bsp
