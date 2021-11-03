#include "json_builder.h"

namespace json {

    // BaseContext
    Builder::BaseContext::BaseContext(Builder &builder) : builder_(builder) {}

    Builder::DictItemContext Builder::BaseContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::ArrayItemContext Builder::BaseContext::StartArray() {
        return builder_.StartArray();
    }

    Builder &Builder::BaseContext::EndArray() {
        return builder_.EndArray();
    }

    Builder &Builder::BaseContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::KeyItemContext Builder::BaseContext::Key(std::string key) {
        return builder_.Key(move(key));
    }

    Builder &Builder::BaseContext::Value(Node value) {
        return builder_.Value(move(value));
    }

    // KeyItemContext

    Builder::KeyItemContext::KeyItemContext(Builder &builder) : BaseContext(builder) {}

    Builder::KeyValueItemContext Builder::KeyItemContext::Value(Node value) {
        return BaseContext::Value(move(value));
    }

    // KeyValueItemContext

    Builder::KeyValueItemContext::KeyValueItemContext(Builder &builder) : BaseContext(builder) {}


    // DictItemContext

    Builder::DictItemContext::DictItemContext(Builder &builder) : BaseContext(builder) {}


    // ArrayItemContext

    Builder::ArrayItemContext::ArrayItemContext(Builder &builder) : BaseContext(builder) {}

    Builder::ArrayValueItemContext Builder::ArrayItemContext::Value(Node value) {
        return BaseContext::Value(move(value));
    }

    // ArrayValueItemContext

    Builder::ArrayValueItemContext::ArrayValueItemContext(Builder &builder) : BaseContext(builder) {}

    Builder::ArrayValueItemContext Builder::ArrayValueItemContext::Value(Node value) {
        return BaseContext::Value(move(value));
    }

    // Builder

    using namespace std::literals;

    void ThrowExceptionIfIncorrectBuildOrder(const std::vector<Node*>& nodes_stack_) {
        if (nodes_stack_.empty() || (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray())) {
            throw std::logic_error("Try to create Entity in empty object or not in Array and Node"s);
        }
    }

    Builder::Builder() {
        nodes_stack_.push_back(&root_);
    }

    Builder::DictItemContext Builder::StartDict() {
        ThrowExceptionIfIncorrectBuildOrder(nodes_stack_);
        if (nodes_stack_.back()->IsArray()) {
            const_cast<Array &>(nodes_stack_.back()->AsArray()).push_back(Dict());
            Node* node = &const_cast<Array &>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        } else {
            *nodes_stack_.back() = Dict();
        }
        return *this;
    }

    Builder::ArrayItemContext Builder::StartArray() {
        ThrowExceptionIfIncorrectBuildOrder(nodes_stack_);
        if (nodes_stack_.back()->IsArray()) {
            const_cast<Array &>(nodes_stack_.back()->AsArray()).push_back(Array());
            Node* node = &const_cast<Array &>(nodes_stack_.back()->AsArray()).back();
            nodes_stack_.push_back(node);
        } else {
            *nodes_stack_.back() = Array();
        }
        return *this;
    }

    Builder &Builder::EndDict() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Try to end Dict in empty object or not in Dict"s);
        }
        nodes_stack_.erase(nodes_stack_.end() - 1);
        return *this;
    }

    Builder &Builder::EndArray() {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Try to end Array in empty object or not in Array"s);
        }
        nodes_stack_.erase(nodes_stack_.end() - 1);
        return *this;
    }

    Builder::KeyItemContext Builder::Key(std::string key) {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Try to insert Key in ready object or not in Dict"s);
        }
        nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsDict())[key]);
        return *this;
    }

    Builder &Builder::Value(Node value) {
        ThrowExceptionIfIncorrectBuildOrder(nodes_stack_);
        if (nodes_stack_.back()->IsArray()) {
            const_cast<Array &>(nodes_stack_.back()->AsArray()).push_back(value);
        } else {
            *nodes_stack_.back() = value;
            nodes_stack_.erase(nodes_stack_.end() - 1);
        }
        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Try to build before object is ready"s);
        }
        return root_;
    }
}
