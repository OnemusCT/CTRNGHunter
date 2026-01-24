# Generate a templates.h file that contains constexpr
file(GLOB TEMPLATE_FILES_LIST "${TEMPLATE_DIR}/*.md")

set(HEADER_CONTENT "#pragma once\n#include <string_view>\n#include <unordered_map>\n\nnamespace templates {\n")
set(TEMPLATE_NAMES "")

foreach(TEMPLATE_FILE ${TEMPLATE_FILES_LIST})
    get_filename_component(TEMPLATE_NAME ${TEMPLATE_FILE} NAME_WE)
    file(READ "${TEMPLATE_FILE}" TEMPLATE_CONTENT)
    
    string(APPEND HEADER_CONTENT "    constexpr const char* ${TEMPLATE_NAME} = R\"(${TEMPLATE_CONTENT})\";\n\n")
    
    list(APPEND TEMPLATE_NAMES ${TEMPLATE_NAME})
endforeach()

string(APPEND HEADER_CONTENT "    // Map for iterating all templates\n")
string(APPEND HEADER_CONTENT "    inline const std::unordered_map<std::string_view, std::string_view> all = {\n")

foreach(TEMPLATE_NAME ${TEMPLATE_NAMES})
    string(APPEND HEADER_CONTENT "        {\"${TEMPLATE_NAME}\", ${TEMPLATE_NAME}},\n")
endforeach()
string(APPEND HEADER_CONTENT "    };\n")
string(APPEND HEADER_CONTENT "}\n")

file(WRITE "${OUTPUT_FILE}" "${HEADER_CONTENT}")