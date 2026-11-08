#pragma once

#include "bsp/native_session_message_base.hpp"
#include "bsp/session_participant_pools.hpp"

namespace bsp {

// Borrow both authoritative mission fields. 0075B430 stores only identity;
// this projection must not be used as a native participant field layout.
class NativeSessionMessageParticipantOwner final : public NativeSessionMessageOwnerView {
public:
    NativeSessionMessageParticipantOwner(const SessionParticipantPools& participants,
        const std::size_t& local_slot) noexcept
        : participants_(participants), local_slot_(local_slot) {}

    void* selected_owner_14() const noexcept override {
        return const_cast<void*>(participants_.active_record_identity(local_slot_));
    }

private:
    const SessionParticipantPools& participants_;
    const std::size_t& local_slot_;
};

} // namespace bsp
