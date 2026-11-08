#include "bsp/archive_text_writer.hpp"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
struct RejectingSink final : bsp::ArchiveByteSink {
    unsigned calls{};
    void write(std::string_view) override {
        ++calls;
        throw std::runtime_error("nested archive rejection");
    }
};

struct ReentrantSink final : bsp::ArchiveByteSink {
    bsp::ArchiveTextWriter* outer{};
    bsp::ArchiveTextWriter* nested{};
    bsp::ArchiveTextWriter* failing{};
    std::string bytes;
    bool armed{};
    bool nested_rejected{};

    void write(std::string_view fragment) override {
        bytes.append(fragment);
        if (!armed) return;
        armed = false;
        nested->write_field("inner", bsp::SettingsValue::from_int(9));
        try {
            failing->write_field("bad", bsp::SettingsValue::from_bool(false));
        } catch (const std::runtime_error& error) {
            nested_rejected = std::string_view(error.what()) == "nested archive rejection";
        }
        // Change the SAME retained writer+4 during its indent callback.
        // BD8CDF must observe root depth for the key, and BD8D00 for the row
        // terminator. The unusual byte order is deliberate reentry evidence.
        outer->end_section();
    }
};
}

int main() {
    try {
        bsp::ArchiveStringSink nested_sink;
        bsp::ArchiveTextOutput nested_output{nullptr, false, &nested_sink};
        bsp::ArchiveTextWriter nested(nested_output);
        RejectingSink rejecting_sink;
        bsp::ArchiveTextOutput failing_output{nullptr, false, &rejecting_sink};
        bsp::ArchiveTextWriter failing(failing_output);
        ReentrantSink outer_sink;
        bsp::ArchiveTextOutput outer_output{nullptr, false, &outer_sink};
        bsp::ArchiveTextWriter outer(outer_output);
        outer_sink.outer = &outer;
        outer_sink.nested = &nested;
        outer_sink.failing = &failing;
        outer.begin_section("T");
        outer_sink.armed = true;
        outer.write_field("field", bsp::SettingsValue::from_int(7));
        if (outer_sink.bytes != "T = {\n\t}\nfield = 7\n"
            || nested_sink.str() != "inner = 9\n"
            || !outer_sink.nested_rejected || rejecting_sink.calls != 1
            || outer.depth() != 0 || nested.depth() != 0 || failing.depth() != 0)
            throw std::runtime_error("archive field reentry/depth/exception mismatch");
        std::puts("archive field: nested output and rejection restore parent; live depth reloads passed");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
