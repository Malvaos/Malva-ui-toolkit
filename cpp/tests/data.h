// Reads the two generated data files in dist/ without a JSON library: resolved.json (every colour of both modes)
// and motion-golden.json (easing and spring samples computed independently in Python).
#pragma once
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#ifndef MALVA_DIST_DIR
#error "MALVA_DIST_DIR must point at the kit's dist/ directory"
#endif

namespace testdata {

// section ("light", "dark", "brand") -> role -> "#RRGGBBAA"
inline std::map<std::string, std::map<std::string, std::string>> resolved()
{
    std::map<std::string, std::map<std::string, std::string>> out;
    std::ifstream f(std::string(MALVA_DIST_DIR) + "/resolved.json");
    std::string line, section;
    const std::regex open_re(R"re(^\s{2}"([\w-]+)":\s*\{)re"), kv_re(R"re(^\s{4}"([\w-]+)":\s*"(#[0-9A-Fa-f]+)")re");
    std::smatch m;
    while (std::getline(f, line)) {
        if (std::regex_search(line, m, open_re))
            section = m[1];
        else if (std::regex_search(line, m, kv_re))
            out[section][m[1]] = m[2];
    }
    return out;
}

inline std::vector<double> numbers(const std::string& line)
{
    std::vector<double> v;
    const std::string rest = line.substr(line.find('[') + 1);
    std::stringstream ss(rest);
    std::string tok;
    while (std::getline(ss, tok, ','))
        v.push_back(std::stod(tok));
    return v;
}

// key -> samples, for every line of the form  "key": [ ... ]
inline std::map<std::string, std::vector<double>> golden()
{
    std::map<std::string, std::vector<double>> out;
    std::ifstream f(std::string(MALVA_DIST_DIR) + "/motion-golden.json");
    std::string line;
    const std::regex key_re(R"re(^\s*"([\w-]+)":\s*\[)re");
    std::smatch m;
    while (std::getline(f, line)) {
        if (std::regex_search(line, m, key_re))
            out[m[1]] = numbers(line);
    }
    return out;
}

}  // namespace testdata
