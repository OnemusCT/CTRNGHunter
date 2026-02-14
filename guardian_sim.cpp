#include "guardian_sim.h"

#include "rng_table.h"

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <climits>
#include <algorithm>
#include <format>
#include <print>
#include <tuple>
#include <bitset>


static constexpr int PREMATURE_HP_THRESHOLD = 60;
static constexpr int BIT_HP = 200;
static constexpr int POD_REGEN_TICKS = 12;

enum Target {
    LEFT, RIGHT
};

struct CharStats {
    std::string name;
    int base_damage;
    int crit_chance; // 0 = no crit roll, no RNG consumed
    Target preferred;
};

static const CharStats CHAR_STATS[] = {
    // Name, damage, crit, default target
    {"Crono", 60, 10, LEFT}, // CRONO
    {"Marle", 40, 20, RIGHT}, // MARLE
    {"Lucca", 35, 0, LEFT}, // LUCCA
};

std::string target_name(Target t) {
    return t == LEFT ? "left pod" : "right pod";
}

// To target the left enemy you have to press right and vice versa
std::string target_direction(Target t) {
    return t == LEFT ? "RIGHT" : "LEFT";
}

Target opposite(Target t) {
    return t == LEFT ? RIGHT : LEFT;
}

static uint8_t advance_rng(uint8_t rng) { return (rng + 1) % 0x100; }

bool would_prematurely_kill(int target_hp, int damage, int other_hp) {
    return (target_hp > 0) &&
           (target_hp <= damage) &&
           (other_hp > PREMATURE_HP_THRESHOLD);
}

Target choose_target(Target preferred, int pref_hp, int other_hp, int damage) {
    if (pref_hp <= 0 || would_prematurely_kill(pref_hp, damage, other_hp))
        return opposite(preferred);
    return preferred;
}

struct Action {
    Character who;
    Target target;
    int damage;
    bool crit;
};

struct SimResult {
    std::vector<Action> actions;
    int left_kill_turn = -1;
    int right_kill_turn = -1;
    int gap = INT_MAX;
    bool both_dead = false;
    int redirects = 0; // times Marle hit left or Lucca hit right
    int final_rng = -1;
};

SimResult simulate(const std::vector<Character>& turn_order,
                   uint8_t starting_rng,
                   std::bitset<8> crono_combo) {
    SimResult result;

    int left_hp = BIT_HP;
    int right_hp = BIT_HP;
    uint8_t rng = starting_rng % 0x100;
    int crono_attack_num = 0;

    for (int turn = 0; turn < std::ssize(turn_order); turn++) {
        if (left_hp <= 0 && right_hp <= 0) break;

        Character who = turn_order[turn];
        bool is_crit = false;

        const auto& stats = CHAR_STATS[who];

        if (stats.crit_chance > 0) {
            is_crit = crit_table(rng, stats.crit_chance);
            rng = advance_rng(rng);
        }
        int damage = stats.base_damage;
        if (is_crit) damage *= 2;

        Target target;
        if (who == CRONO) {
            target = crono_combo.test(crono_attack_num++) ? RIGHT : LEFT;
            if (target == LEFT && left_hp <= 0) target = RIGHT;
            if (target == RIGHT && right_hp <= 0) target = LEFT;
        } else {
            int pref_hp = (stats.preferred == LEFT) ? left_hp : right_hp;
            int other_hp = (stats.preferred == LEFT) ? right_hp : left_hp;
            target = choose_target(stats.preferred, pref_hp, other_hp, damage);
        }

        // Count off-default targeting for Marle (default right) and Lucca (default left)
        if (who == MARLE && target == LEFT) result.redirects++;
        if (who == LUCCA && target == RIGHT) result.redirects++;

        if (target == LEFT) {
            left_hp -= damage;
            if (left_hp <= 0 && result.left_kill_turn == -1)
                result.left_kill_turn = turn;
        } else {
            right_hp -= damage;
            if (right_hp <= 0 && result.right_kill_turn == -1)
                result.right_kill_turn = turn;
        }

        result.actions.push_back({who, target, damage, is_crit});
    }

    result.both_dead = (result.left_kill_turn != -1 && result.right_kill_turn != -1);
    result.gap = result.both_dead
                     ? std::abs(result.left_kill_turn - result.right_kill_turn)
                     : INT_MAX;
    result.final_rng = rng;

    return result;
}

SimResult find_best(const std::vector<Character>& turn_order, uint8_t starting_rng) {
    auto crono_turns = std::ranges::count(turn_order, CRONO);
    int total_permutations = 1 << crono_turns;
    SimResult best;
    int best_permutation = -1;

    auto score = [](const SimResult& r) {
        return std::make_tuple(r.gap,
                               std::max(r.left_kill_turn, r.right_kill_turn),
                               r.redirects);
    };

    // Iterate through each possible permutation of Crono targets, testing LLLL, LLLR, LLRL, etc and finding the optimal
    // option
    for (int perm = 0; perm < total_permutations; perm++) {
        SimResult res = simulate(turn_order, starting_rng, std::bitset<8>(perm));
        if (!res.both_dead) continue;

        if (best_permutation == -1 || score(res) < score(best)) {
            best = res;
            best_permutation = perm;
        }
    }
    return best;
}

std::string format_phase(const SimResult& entry, std::string_view phase, bool suppress_instructions = false) {
    std::string out;
    int left_hp = BIT_HP, right_hp = BIT_HP;
    if (entry.redirects == 0) {
        if (!suppress_instructions) {
            out += "Lucca always Flamethrower left bit (RIGHT)\n";
            out += "Marle always attack right bit (LEFT)\n";
            out += "Lucca Flamethrower whenever up\n\n";
        }
        out += std::format("{}\n", phase);
        for (int i = 0; i < std::ssize(entry.actions); i++) {
            const auto& a = entry.actions[i];
            if (a.target == LEFT) left_hp -= a.damage;
            else right_hp -= a.damage;

            if (CHAR_STATS[a.who].name == "Crono") {
                out += std::format("    {:<5} attack {:<5} ({}){}\n",
                                   CHAR_STATS[a.who].name,
                                   target_direction(a.target),
                                   target_name(a.target),
                                   a.crit ? " CRIT" : "     ");
            } else if (CHAR_STATS[a.who].name == "Marle") {
                out += std::format("    {:<5} attack{}\n",
                                   CHAR_STATS[a.who].name,
                                   a.crit ? " CRIT" : "     ");
            }

            if (left_hp <= 0 && right_hp <= 0) break;
        }
    } else {
        for (int i = 0; i < std::ssize(entry.actions); i++) {
            const auto& a = entry.actions[i];
            if (a.target == LEFT) left_hp -= a.damage;
            else right_hp -= a.damage;

            out += std::format("    {:<5} attack {:<5} ({}){}\n",
                               CHAR_STATS[a.who].name,
                               target_direction(a.target),
                               target_name(a.target),
                               a.crit ? " CRIT" : "     ");

            if (left_hp <= 0 && right_hp <= 0) break;
        }
    }
    return out;
}

std::string sim_guardian(uint8_t rng, const std::vector<Character>& p1_turn_order, const std::vector<Character>& p2_turn_order) {
    SimResult phase1 = find_best(p1_turn_order, rng);
    std::string result = format_phase(phase1, "Phase 1");
    rng = phase1.final_rng;
    rng += POD_REGEN_TICKS;
    SimResult phase2 = find_best(p2_turn_order, rng);
    result += "    Auto-battle for 12 attacks\n\n";
    // ReSharper disable once CppDFAConstantConditions
    result += format_phase(phase2, "Phase 2", phase1.redirects == 0 && phase2.redirects == 0);
    result += "    Auto-battle to end";

    std::println("\nTotal attacks: {}", phase1.actions.size() + phase2.actions.size());
    return result;
}
