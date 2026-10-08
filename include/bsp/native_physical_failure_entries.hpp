#pragma once

#include <cstdint>

namespace bsp {

// Descriptive names are hypotheses, not recovered symbols. These additional
// raw entries do not replace the existing typed VFS callback/identity routes.

// Original 00530620: exactly C3. No semantic inputs or result; plain RET pops
// only the return address. Non-stack registers and arithmetic flags are kept.
void __cdecl raw_ignore_native_vfs_mount_failure_00530620() noexcept;

// Original 00BD9E30..00BD9E3A: ECX is a genuine already-produced initialized
// actual manager borrowed for this call. Its readable +90 DWORD must contain
// the executable address of the genuine C3 callback (Original or qualified
// exact Source counterpart) in this process/code domain. The owner and code
// must remain alive; the callback returns without popping stack arguments.
// Numeric Original identities alone do not establish a callable Source target.
//
// MOV EAX,[ECX+90]; CALL EAX; RET4. EDX reaches the callback unchanged; the one
// stack DWORD is discarded, not passed as a callback argument. This entry reads
// neither global0109CEEC nor manager+18 and imposes no equality between EDX and
// the discarded word. BF5030's field18 loads belong to that separate caller.
//
// No semantic return value. With the genuine C3 target only, EAX retains the
// loaded target, ECX/EDX and other non-stack registers/flags stay unchanged.
// No current callable-domain owner is admitted here; this declaration does not
// authorize notifier execution, owner fabrication, or a connected read path.
void __fastcall raw_notify_native_vfs_request_failure_00bd9e30(
    void* actual_manager, std::uint32_t forwarded_edx,
    std::uint32_t discarded_stack_word) noexcept;

} // namespace bsp
