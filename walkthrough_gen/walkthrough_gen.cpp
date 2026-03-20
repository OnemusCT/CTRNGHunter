#include "walkthrough_gen/walkthrough_gen.h"

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <set>
#include <unordered_map>

#include "seed_parser.h"
#include "templates.h"
#include "../guardian_sim.h"

void add_templates(inja::Environment& env) {
    int tries = 0;
    std::set<std::string> processed;
    // We have a dependency tree for templates, but it's not that complex
    // so I just do a dumb version of trying each one over and over until
    // they all work
    while (tries < 10 && processed.size() != templates::all.size()) {
        ++tries;
        for (const auto& [name, template_str]: templates::all) {
            if (!processed.contains(std::string(name))) {
                try {
                    inja::Template t = env.parse(template_str);
                    env.include_template(std::string(name), t);
                    processed.insert(std::string(name));
                } catch (const std::exception& e) {
                    std::cout << name << " - " << e.what() << std::endl;
                }
            }
        }
    }
}

void generate_walkthrough(WalkthroughType type, time_t seed,
                          const std::unordered_map<std::string, RNGSim::EncounterStats>& stats_map, std::ostream& out) {
    inja::Environment env;
    env.set_line_statement("$$"); // Change line statements so they don't conflict with markdown headers
    env.set_expression("{$", "$}");
    nlohmann::json data;
    data["seed"] = seed;
    data["seedString"] = seed_to_string(seed);
    for (const auto& [battle, stats]: stats_map) {
        data["rng"][battle] = stats.battle_rng;
        data["rooms"][battle] = stats.extra_rooms;
        data["heals"][battle] = stats.extra_heals;
    }
    std::cout << data << std::endl;
    try {
        add_templates(env);
        inja::Template walkthrough;
        bool onemus = false;
        switch (type) {
            case WalkthroughType::ONEMUS:
                onemus = true;
            case WalkthroughType::FULL:
                data["onemus"] = onemus;
                walkthrough = env.parse(templates::full_walkthrough);
                env.render_to(out, walkthrough, data);
                break;
            case WalkthroughType::SIMPLE:
                walkthrough = env.parse(templates::simplified_walkthrough);
                env.render_to(out, walkthrough, data);
                break;
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
