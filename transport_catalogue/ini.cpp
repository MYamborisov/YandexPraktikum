#include "ini.h"

using namespace std;

namespace ini {
    std::size_t Document::GetSectionCount() const {
        return sections_.size();
    }

    const Section& Document::GetSection(const std::string &name) const {
        if (sections_.count(name)) {
            return sections_.at(name);
        } else {
            static Section res;
            return res;
        }
    }

    Section& Document::AddSection(std::string name) {
        return sections_[name];
    }

    Document Load(istream& input) {
        Document result;
        string line;
        Section test;
        char c;
        while(getline(input ,line)) {
            if (line.find_first_not_of(" ") != string::npos && line[line.find_first_not_of(" ")] == '[') {
                size_t first = line.find_first_not_of(" ");
                size_t last = line.find_last_not_of(" ");
                string sec_name = line.substr(first + 1, last - first - 1);
                auto& sec = result.AddSection(sec_name);
                while (input >> c) {
                    if (c == '[') {
                        input.putback(c);
                        break;
                    }
                    input.putback(c);
                    getline(input ,line);
                    if (line.find('=') != string::npos) {
                        size_t pos = line.find('=');
                        string key = line.substr(0, pos);
                        string value = line.substr(pos + 1, line.size() - pos - 1);
                        size_t key_f = key.find_first_not_of(" "), key_l = key.find_last_not_of(' ');
                        size_t val_f = value.find_first_not_of(" "), val_l = value.find_last_not_of(' ');
                        key = key.substr(key_f, key_l - key_f + 1);
                        value = value.substr(val_f, val_l - val_f + 1);
                        sec[key] = value;
                    }
                }
            }
        }
        return result;
    }
}