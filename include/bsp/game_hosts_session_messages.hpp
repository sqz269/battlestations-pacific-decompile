#pragma once
// Host-owned image memory for session messages (cc15-sm1, the bring-up pattern).
//
// The image builds a session message on the caller's stack or heap and hands it
// to 0077C2A0. The local route 0076E520 never queues that object: it writes the
// message into a 400h-byte stack stream (vtable[4h] at 0076E575), then rebuilds
// a second message from those bits with the factory 00768530 (0076E5A4), and it
// is that copy the queue holds until the drain frees it. So a queued message
// carries only what its write serialises: positions quantised, the object
// pointer dropped in favour of the 13-bit handle, the command reduced to its
// ordinal byte.
//
// This class owns the image-shaped context those bodies read (the session's
// game object, the bit codec's constants) and runs the round trip through the
// cc13 reconstructions. Each message is a host value: the sent one lives for the
// call, as the image's stack message does; the queued one is returned to the
// caller, who keeps it until its delivery and then calls the destructor, which
// is where the image's drain frees it.
#include "bsp/entity_orders.hpp"
#include "bsp/native_session_message_tags_90_to93.hpp"
#include <memory>

namespace bsp {

class HostSessionMessages {
public:
    HostSessionMessages();
    ~HostSessionMessages();
    HostSessionMessages(const HostSessionMessages&) = delete;
    HostSessionMessages& operator=(const HostSessionMessages&) = delete;

    // MT_GAMEUNIT_SETCMD: 0071C830 at 0071ED6C over a stack message, then
    // 0076E520's local copy (0071C930 write, 00768530 type byte and rewind,
    // case 92 at 00769D10: 00764F90, then 0071CA30 read). `command_ordinal` is
    // the command object's +4h ordinal, or negative for a null command (0071C830
    // then writes 0FFh). Returns the queued copy.
    NativeSessionMessage92 queue_set_command_92(int command_ordinal,
                                                const SceneCommandTarget& target,
                                                std::uint8_t flag) const;
    // 0071CB40, the destructor the drain's scalar delete 0071E3B0 runs. The
    // memory is the caller's, so only the destructor runs.
    static void free_set_command_92(NativeSessionMessage92& message) noexcept;

    // MT_COMMAND (58h): the message 007798D0 built, as the typed record that
    // mirrors its 3Ch layout, through 0076E520's local copy (00764D40 write,
    // 00768530 case 88 at 00769CB0: 00764C80, then 00764E60 read). Returns the
    // posted record with the fields the copy carries replaced by the copy's:
    // the command ordinal, the flags bit, the target. The copy is destroyed
    // (00764CF0) here because the record is all the delivery reads.
    EntityOrderMessage queue_command_88(const EntityOrderMessage& posted) const;

private:
    struct Image;
    std::unique_ptr<Image> image_;
};

} // namespace bsp
