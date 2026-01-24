#pragma once

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

#include "templates.h"

#include <string>
#include <unordered_map>

void generate_walkthrough(const std::unordered_map<std::string, int>& rng_map, const std::unordered_map<std::string, int>& rooms_map) {
	inja::Environment env;
	env.set_line_statement("$$"); // Line statements ## (just an opener)
	nlohmann::json data;
	for (const auto& [battle, rng] : rng_map) {
		data["rng"][battle] = rng;
	}
	for (const auto& [battle, rooms] : rooms_map) {
		data["rooms"][battle] = rooms;
	}
	try {
		inja::Template yakra = env.parse(templates::yakra);
		env.include_template("yakra", yakra);
		inja::Template dragontank = env.parse(templates::dragontank);
		env.include_template("dragontank", dragontank);
		inja::Template walkthrough = env.parse(templates::walkthrough);
		env.render_to(std::cout, walkthrough, data);
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}