#include "walkthrough_gen/walkthrough_gen.h"

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <set>
#include <unordered_map>

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
                    std::cout << "Adding template " << name << std::endl;
                    processed.insert(std::string(name));
                } catch (const std::exception& e) {
                    std::cout << name << " - " << e.what() << std::endl;
                }
            }
        }
    }
}

void generate_walkthrough(time_t seed, const std::unordered_map<std::string, int>& rng_map,
                          const std::unordered_map<std::string, int>& rooms_map,
                          const std::unordered_map<std::string, int>& heal_map, std::ostream& out) {
    inja::Environment env;
    env.set_line_statement("$$"); // Change line statements so they don't conflict with markdown headers
    env.set_expression("{$", "$}");
    nlohmann::json data;
    data["seed"] = seed;
    for (const auto& [battle, rng]: rng_map) {
        data["rng"][battle] = rng;
        if (battle == "guardian") {
            data["guardian_battle"] = sim_guardian(rng);
        }
    }
    for (const auto& [battle, rooms]: rooms_map) {
        data["rooms"][battle] = rooms;
    }
    for (const auto& [battle, heals]: heal_map) {
        data["heals"][battle] = heals;
    }

    std::cout << data << std::endl;
    try {
        add_templates(env);
        inja::Template walkthrough = env.parse(templates::full_walkthrough);
        env.render_to(out, walkthrough, data);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
