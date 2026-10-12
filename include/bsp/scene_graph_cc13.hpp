#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Partition segments 88/89 (00B6D3C0..00B80ED0, 00B80FD0..00B86E10) reconstructions by
// cc13-files1. Evidence, original ABIs and uncertainty: docs/CC13_SCENE_GRAPH_NODES.md.
// New C++ interfaces over host data; the native routines use custom register ABIs
// (XMM0 float returns, EAX/ECX/EDX/ESI/EDI pointer inputs) that are not reproduced.

namespace bsp::cc13_scene {

// 28h-byte animation key.
struct AnimKey {
    std::int32_t type{};   // +00: interpolation of the segment that ENDS at this key
    float value{};         // +04
    float time{};          // +08
    float tension{};       // +0C  (TCB, type 0)
    float continuity{};    // +10
    float bias{};          // +14
    float in_a{};          // +18: type 1/2 in tangent; type 5 in-handle dt
    float in_b{};          // +1C: type 1/2 out tangent; type 5 in-handle dv
    float out_a{};         // +20: type 5 out-handle dt
    float out_b{};         // +24: type 5 out-handle dv
};

// Out-of-range behaviour at track +14h (before the first key) and +18h (after the last).
enum class OutOfRange : std::int32_t { zero = 0, hold = 1, cycle = 2, ping_pong = 3, relative_cycle = 4, linear = 5 };

// Track fields used by the sampler: +14h, +18h, keys +1Ch, count +20h.
struct AnimTrack {
    OutOfRange before{};
    OutOfRange after{};
    std::vector<AnimKey> keys;
};

// 00B75450: wrap `time` into the key range. Returns start + time - floor(time/len)*len
// (len = end - start; start when len == 0). When `cycles` is given it receives
// -trunc((wrapped - time)/len -/+ 0.5).
float wrap_time_00b75450(float time, float start, float end, std::int32_t* cycles);

// 00B75520: cubic Hermite basis for s; outputs are rounded to float like the native stores.
struct HermiteBasis { float p0, p1, t0, t1; };
HermiteBasis hermite_basis_00b75520(float s);

// 00B75820: tangent leaving `key` toward `next` (`prev` may be null).
float outgoing_tangent_00b75820(const AnimKey& key, const AnimKey* prev, const AnimKey& next);
// 00B75A20: tangent arriving at `key` from `before`; `after` may be null.
float incoming_tangent_00b75a20(const AnimKey& key, const AnimKey* after, const AnimKey& before);

// 00B75C10: index i with keys[i].time <= time < keys[i+1].time (interpolation probe plus
// bisection); 0 for fewer than three keys.
std::int32_t find_key_00b75c10(const AnimKey* keys, std::int32_t count, float time);

// 00B75580: cubic Bezier in power form for control values p0..p3 at s.
double bezier_00b75580(float p0, float p1, float p2, float p3, float s);
// 00B75610: bisect s in [lo, hi] until |target - bezier(s)| <= 1e-4. The native loop has no
// iteration limit; this version stops after `max_iterations` (documented deviation).
float bezier_solve_00b75610(float& lo, float& hi, float p0, float p1, float p2, float p3,
    float target, int max_iterations = 200);
// 00B75720: type-5 segment from `key` to `next` at absolute time.
float bezier_segment_00b75720(const AnimKey& key, const AnimKey& next, float time);

// 00B76CE0: sample a track at time.
float sample_track_00b76ce0(const AnimTrack& track, float time);

// 00B77220: nine optional channels (tx,ty,tz, rx,ry,rz, sx,sy,sz); a missing track leaves
// the output component unchanged.
// The track array (+10h) has at least 11 entries: node animation reads channel 10 and the camera
// channel 9 through 00B77430.
struct ChannelGroup { std::vector<const AnimTrack*> channels; };
void sample_channel_group_00b77220(const ChannelGroup& group, float position[3], float rotation[3],
    float scale[3], float time);

// 00B77360 / 00B77430 (ordinary animator vtable 00D62E60 slots +18h/+1Ch). The animator
// supplies translation +08h..+10h, angles +14h..+1Ch and groups +20h/+24h.
struct NodeAnimatorView {
    float translation[3]{};
    float angles[3]{};
    std::vector<const ChannelGroup*> groups;
};
// Fills position/rotation/scale for the matrix the native slot composes through
// 00B64CE0 (scale defaults to 1). Returns false for a missing group.
bool evaluate_animator_pose_00b77360(const NodeAnimatorView& animator, std::uint32_t group,
    float time, float position[3], float rotation[3], float scale[3]);
// Returns false for a missing group or channel; otherwise samples one channel.
bool evaluate_animator_channel_00b77430(const NodeAnimatorView& animator, std::uint32_t group,
    std::uint32_t channel, float time, float& out);


// ---------------------------------------------------------------------------
// Scene-graph node queries. A host node view: name (+54h) and children in sibling order
// (+34h first child, +3Ch next sibling).
struct SceneNodeView {
    std::string name;
    std::vector<SceneNodeView*> children;
};
// 00B6E750: direct children first (names compared case-insensitively), then, when
// recursive, each child's subtree in order.
SceneNodeView* find_child_by_name_00b6e750(SceneNodeView& node, const std::string& name, bool recursive);
// 00B6F9A0: post-order collection of descendants whose name contains `filter` (all when empty).
void collect_descendants_matching_00b6f9a0(SceneNodeView& node, const std::string& filter,
    std::vector<SceneNodeView*>& out);
// 00B6E150 visibility rule: node +ACh = clamp(1 - channel 10 sample, 0, 1); NaN stays NaN.
float node_visibility_from_channel_00b6e150(float sampled) noexcept;
// 00B70DE0: camera +1C4h = 2*atan(1/channel 9 sample) (00BF8490 is CRT atan; FPATAN fallback at 00BF84FF).
float camera_fov_from_channel_00b70de0(float sampled) noexcept;

// ---------------------------------------------------------------------------
// 00B811C0 resource dump table (the CSV save 00BF0A30 is a stub in this build).
struct ResourceDumpEntry { std::string name; std::int32_t size{}; std::int32_t refcount{}; };
// Returns the rows: header {Name, Size, RefCounter, "Size / RefCounter"}, then entries by
// size descending (qsort, so equal sizes have no defined order); size/refcount is integer
// division, which the native code performs even for a zero refcount (here: column left empty).
std::vector<std::vector<std::string>> resource_dump_rows_00b811c0(std::vector<ResourceDumpEntry> entries);
// The file name the native routine passes to the stub.
std::string resource_dump_file_name_00b811c0(const std::string& suffix);
}  // namespace bsp::cc13_scene
