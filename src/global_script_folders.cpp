#include "bsp/global_script_folders.hpp"
#include "bsp/native_pooled_string_substring.hpp"

#include <cstring>
#include <deque>
#include <utility>
#include <vector>

namespace bsp {
namespace {
struct StringOwner {
    NativeString value;
    NativeStringStorage& strings;
    bool owns = true;
    explicit StringOwner(NativeStringStorage& storage) : strings(storage) {}
    ~StringOwner() { if (owns) destroy_native_string_header_0041dd20(&value, strings); }
};
void copy_counted(NativeString& destination, const NativeString& source,
    NativeStringStorage& strings) {
    destination.resize_0041dd40(strings, source.length(), true);
    if (source.length()) std::memcpy(destination.data(), source.data(), destination.length());
}
std::string counted(const NativeString& value) {
    return value.length() ? std::string(value.data(), value.length()) : std::string{};
}
void literal(NativeString& value, const char* text, std::uint32_t length,
    NativeStringStorage& strings) {
    value.resize_0041dd40(strings, length, true);
    if (value.data()) std::memcpy(value.data(), text, value.length() + 1u);
}

// Existing00886280 returns a standard-string sequence. Materialize its native
// pooled payload ownership; standard deque nodes stand in for native list
// nodes/sentinel. Provider allocation/exception timing is not reproduced.
struct PathList {
    std::deque<NativeString> values;
    NativeStringStorage& strings;
    explicit PathList(NativeStringStorage& storage) : strings(storage) {}
    PathList(const PathList&) = delete;
    ~PathList() {
        for (auto& value : values) destroy_native_string_header_0041dd20(&value, strings);
    }
    void fill(const std::vector<std::string>& source) {
        for (const auto& text : source) {
            values.emplace_back();
            auto& value = values.back();
            value.resize_0041dd40(strings, static_cast<std::uint32_t>(text.size()), true);
            if (value.length()) std::memcpy(value.data(), text.data(), value.length());
        }
    }
    void pop_front(NativeString& output) {
        //00557A90's valid nonempty-list contract: copy before erase/release.
        copy_counted(output, values.front(), strings);
        destroy_native_string_header_0041dd20(&values.front(), strings);
        values.pop_front();
    }
};
struct CompiledNames {
    std::vector<NativeString> values;
    NativeStringStorage& strings;
    ~CompiledNames() {
        //00427110(0): count shrinks before each captured string is returned.
        while (!values.empty()) {
            auto value = std::move(values.back());
            values.pop_back();
            destroy_native_string_header_0041dd20(&value, strings);
        }
    }
    void append(const NativeString& source) {
        //004CDC20's deep-copy contract, not a borrowed pointer to the pop temp.
        StringOwner copy(strings);
        copy_counted(copy.value, source, strings);
        values.push_back(std::move(copy.value));
    }
    bool contains(std::uint32_t length, const char* data) const {
        for (const auto& value : values) {
            if (length == value.length() && (!length || _stricmp(data, value.data()) == 0))
                return true;
        }
        return false;
    }
};
void enumerate(PathList& output, const char* directory, const char* suffix,
    std::uint32_t suffix_length, std::uint32_t flags, GlobalScriptFolderContext& context) {
    StringOwner extension(context.strings);
    literal(extension.value, suffix, suffix_length, context.strings);
    StringOwner folder(context.strings);
    folder.value.assign_0041e870(context.strings, directory);
    output.fill(context.resources_0109ceec->enumerate_descriptors_00886280(
        counted(folder.value), counted(extension.value), flags));
    // directory then extension pooled temporaries are released before pop.
}
} // namespace

void load_global_script_folder_00886370(MissionLuaHostServices& captured_owner,
    const char* directory, std::uint32_t flags, GlobalScriptFolderContext& context) {
    CompiledNames compiled{{}, context.strings};
    PathList binaries(context.strings);
    enumerate(binaries, directory, ".luab", 5, flags, context);
    while (!binaries.values.empty()) {
        StringOwner path(context.strings);
        binaries.pop_front(path.value);
        compiled.append(path.value); // before execution, including failed opens/chunks
        (void)run_script_file(captured_owner, path.value.data() ? path.value.data() : "");
    }

    PathList sources(context.strings);
    enumerate(sources, directory, ".lua", 4, flags, context);
    while (!sources.values.empty()) {
        StringOwner path(context.strings);
        sources.pop_front(path.value);
        StringOwner candidate(context.strings);
        copy_counted(candidate.value, path.value, context.strings);
        {
            StringOwner prefix(context.strings);
            construct_native_string_substring_00469840(&candidate.value, &prefix.value,
                0, candidate.value.length() - 3u, context.strings);
            copy_counted(candidate.value, prefix.value, context.strings);
        }
        char* captured_data;
        std::uint32_t captured_length;
        {
            StringOwner suffix(context.strings);
            literal(suffix.value, "luab", 4, context.strings);
            // Native EDI/ESI retain the suffix data/length through resize.
            auto* const suffix_data = suffix.value.data();
            const auto suffix_length = suffix.value.length();
            const auto prefix_length = candidate.value.length();
            if (suffix_length) {
                candidate.value.resize_0041dd40(context.strings,
                    prefix_length + suffix_length, true);
                std::memcpy(candidate.value.data() + prefix_length, suffix_data, suffix_length);
            }
            captured_data = candidate.value.data();
            captured_length = candidate.value.length();
        }
        if (!compiled.contains(captured_length, captured_data))
            (void)run_script_file(captured_owner, path.value.data() ? path.value.data() : "");
        // Captured candidate data/length are the native EBX/EBP cleanup pair.
        if (captured_data) context.strings.release(captured_data, captured_length + 1u);
        candidate.owns = false; // captured cleanup already performed; leave dead header alone
    }
    // Source list, compiled list, then reverse-order remembered names cleanup.
}

void load_global_script_folders_00886900(MissionLuaHostServices& captured_owner,
    GlobalScriptFolderContext& context) {
    load_global_script_folder_00886370(captured_owner, "Scripts/global/", 1, context);
    load_global_script_folder_00886370(captured_owner, "Scripts/datatables/autoload/", 0, context);
}
} // namespace bsp
