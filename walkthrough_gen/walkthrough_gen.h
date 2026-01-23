#pragma once

#include <filesystem>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

#include "templates.h"

void generate_walkthrough() {
	inja::Environment env;
	env.set_line_statement("$$"); // Line statements ## (just an opener)
	nlohmann::json data;
	data["yakra"] = 0x03;
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