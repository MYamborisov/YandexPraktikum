#include "remove_duplicates.h"

#include <iostream>
#include <map>
#include <set>
#include <string>

using namespace std;

void RemoveDuplicates(SearchServer& search_server) {
    map<set<string>, int> words_to_id;
    vector<int> to_delete;
    for (const int document_id : search_server) {
        set<string> words;
        for (auto [word, freq] : search_server.GetWordFrequencies(document_id)) {
            words.insert(word);
        }
        if (words_to_id.count(words)) {
            to_delete.push_back(document_id);
            cout << "Found duplicate document id " << document_id << endl;
        }
        else {
            words_to_id[words] = document_id;
        }
    }
    for (auto id : to_delete) {
        search_server.RemoveDocument(id);
    }
}