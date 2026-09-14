#pragma once

#include <cstdint>

namespace bsp {
// Complete original C16879..C16897 (__NLG_Notify), 31 bytes. Native Win32
// register entry: EAX=raw destination, EBP=inherited frame, [entry ESP+4]=code.
// The one original code word is callee-popped (RET 4); no binding argument is
// added. An ordinary C++ call cannot establish the EAX/EBP input contract.
//
// Requires the already-published GameNativeCanonicalDataOwner: actual RW
// descriptor E16830[10h], retained for process lifetime. This leaf writes
// code/+8, destination/+4, frame/+Ch in that order, leaving signature/+0
// untouched. It performs no initialization, validation, locking or callback.
// All general registers and flags survive an ordinary return; ESP advances
// by eight bytes from entry (return word plus code). The original temporary
// pushes/pops and their exact stack effects are retained. Fault/nonlocal exit
// can expose partial stores; no rollback or exception translation is supplied.
// See docs/NATIVE_CRT_NLG_NOTIFY_DJ.md. Exact source bytes do not establish
// native caller/unwind closure, debugger notification ownership or gameplay.
void __stdcall notify_native_crt_nlg_00c16879(std::uint32_t code);
} // namespace bsp
