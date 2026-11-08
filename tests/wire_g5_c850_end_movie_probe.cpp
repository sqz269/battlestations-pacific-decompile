#include "bsp/mission_result.hpp"

#include <cstdio>
#include <cstring>
#include <optional>
#include <string>

int main() {
    std::optional<bsp::MissionEndMovieRequest> request;
    const char expected[] = "movies/a\0b";
    std::string input(expected, sizeof(expected) - 1);
    bsp::set_end_of_mission_movie_004cd390(request, input, true, 73.5f);
    if (!request || request->name.length() != input.size()
        || request->name.data() == input.data()
        || std::memcmp(request->name.data(), expected, sizeof(expected)) != 0
        || request->field_00 != 0 || request->field_04 != 0
        || request->parameter_08 != 73.5f || request->parameter_0c != 1.0f
        || request->field_10 != 0 || request->field_11 != 0
        || request->field_1c != 0 || request->field_20 != 0) return 1;

    // The request owns the constructor's exact-length copy, including an
    // embedded NUL. Retiring the source cannot change the polled record.
    input.assign("changed");
    auto decision = bsp::mission_completion_poll_004d7ea0(0, request);
    if (!decision.runs || !decision.enqueues_debrief || !decision.suspends_drain
        || std::memcmp(request->name.data(), expected, sizeof(expected)) != 0) return 2;

    // Replacement destroys the old pooled string and constructs the same
    // empty-name native representation; the new debrief byte drives the poll.
    bsp::set_end_of_mission_movie_004cd390(request, "", false, -2.0f);
    decision = bsp::mission_completion_poll_004d7ea0(0, request);
    if (request->name.length() != 0 || request->name.data() != nullptr
        || request->parameter_08 != -2.0f || request->parameter_0c != 1.0f
        || !decision.runs || decision.enqueues_debrief || decision.suspends_drain) return 3;
    request.reset();
    if (bsp::mission_completion_poll_004d7ea0(0, request).runs) return 4;
    std::puts("c850 end-movie ownership/replacement/poll probe passed");
    return 0;
}
