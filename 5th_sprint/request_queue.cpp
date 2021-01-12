#include "request_queue.h"

using namespace std;

    RequestQueue::RequestQueue(const SearchServer& search_server)
        : search_server_(search_server)
        , no_results_requests_(0)
        , current_time_(0) {
    }

    vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
        const auto result = search_server_.FindTopDocuments(raw_query, status);
        AddRequest(result.size());
        return result;
    }

    vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
        const auto result = search_server_.FindTopDocuments(raw_query);
        AddRequest(result.size());
        return result;
    }

    int RequestQueue::GetNoResultRequests() const {
        return no_results_requests_;
    }

    void RequestQueue::AddRequest(int results_num) {
        // new request - new second
        ++current_time_;
        // deleting all search results that are outdated
        while (!requests_.empty() && sec_in_day_ <= current_time_ - requests_.front().timestamp) {
            if (requests_.front().results == 0) {
                --no_results_requests_;
            }
            requests_.pop_front();
        }
        // saving the new search result
        requests_.push_back({current_time_, results_num});
        if (results_num == 0) {
            ++no_results_requests_;
        }
    }