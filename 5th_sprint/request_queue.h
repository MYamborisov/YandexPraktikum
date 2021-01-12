#pragma once

#include "search_server.h"

#include <cstdint>
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);

    // we make "wrappers" for all search methods to save results for our statistics
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        const auto result = search_server_.FindTopDocuments(raw_query, document_predicate);
        AddRequest(result.size());
        return result;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        std::uint64_t timestamp;
        int results;
    };
    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
    int no_results_requests_;
    std::uint64_t current_time_;
    constexpr static int sec_in_day_ = 1440;

    void AddRequest(int results_num);
};