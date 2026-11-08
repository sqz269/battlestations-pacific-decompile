#include "bsp/game_scene_post_process.hpp"
#include "bsp/scene_file.hpp"
#include "bsp/scene_weather_cc13.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
struct Publications final : bsp::scene_weather_cc13::ScenePostProcessHost {
    unsigned count{};
    float middle_gray{};
    void set_hdr_params_00b0e9b0(const bsp::scene_weather_cc13::ScenePostProcessHdr& value) override {
        ++count;
        middle_gray = value.middle_gray;
    }
    void set_motion_blur_params_00b0ea20(
        const bsp::scene_weather_cc13::ScenePostProcessMotionBlur&) override { ++count; }
    void set_underwater_wave_params_00b0ea70(
        const bsp::scene_weather_cc13::ScenePostProcessUnderwaterWave&) override { ++count; }
};
int main(int argc, char** argv) {
    try {
        if(argc != 2) throw std::runtime_error("schema path required");
        std::ifstream input(argv[1], std::ios::binary);
        if(!input) throw std::runtime_error("schema could not be read");
        std::string text((std::istreambuf_iterator<char>(input)), {});
        bsp::SceneLexer lexer(text, ";{}=:()");
        if(lexer.next().text != "properties" || lexer.next().text != "SceneRootProps")
            throw std::runtime_error("schema declaration");
        std::vector<std::string> errors;
        auto schema = bsp::parse_scene_property_block_008f5a00(lexer, errors);
        if(!errors.empty()) throw std::runtime_error(errors.front());
        bsp::SceneLexer authored(R"({ "g_PostProcess" { "Colorize" {
            TextureName = S ""; } "HDR" { MiddleGray = F 2.0; } } })", ";{}=:()");
        auto header = bsp::parse_scene_property_block_008f5a00(authored, errors);
        if(!errors.empty()) throw std::runtime_error(errors.front());
        auto record = bsp::game::read_scene_post_process_record(schema, header);
        Publications publications;
        record.publish_parameters(publications);
        if(publications.count != 3 || publications.middle_gray != 2.0f)
            throw std::runtime_error("partial HDR override / parameter publications mismatch");
        auto names = record.remap_names();
        if(!names[0].empty() || names[1] != "ColourRemap.tga" ||
            names[2] != "volume_technicolor_002.tga" || names[3] != "volume_remap_UW.tga")
            throw std::runtime_error("authored empty / schema fallback mismatch");
        bsp::SceneLexer scalar("{ g_PostProcess = I 0; }", ";{}=:()");
        auto disabled_header = bsp::parse_scene_property_block_008f5a00(scalar, errors);
        if(!errors.empty()) throw std::runtime_error(errors.front());
        auto disabled = bsp::game::read_scene_post_process_record(schema, disabled_header);
        Publications disabled_publications;
        disabled.publish_parameters(disabled_publications);
        if(disabled_publications.count != 0)
            throw std::runtime_error("scalar g_PostProcess published schema parameters");
        for(const auto& name : disabled.remap_names())
            if(!name.empty()) throw std::runtime_error("scalar g_PostProcess inherited schema remaps");
        std::cout << "PASS: schema overlay and authored empty Colorize; scalar g_PostProcess publishes no parameters and keeps remaps empty\n";
        return 0;
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
