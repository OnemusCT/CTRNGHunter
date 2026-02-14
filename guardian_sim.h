#pragma once
#include <cstdint>
#include <string>
#include <vector>

enum Character {
    CRONO, MARLE, LUCCA
};

std::string sim_guardian(uint8_t rngconst,
                         const std::vector<Character>& p1_turn_order = {
                             CRONO, MARLE, LUCCA,
                             CRONO, MARLE,
                             CRONO, LUCCA,
                             CRONO, MARLE
                         }, const std::vector<Character>& p2_turn_order = {
                             CRONO, MARLE, LUCCA,
                             CRONO, MARLE,
                             CRONO, LUCCA,
                             CRONO, MARLE
                         });
