#pragma once

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using NodeData = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        Node() = default;
        Node(bool value);
        Node(std::nullptr_t /* value*/);
        Node(int value);
        Node(double value);
        Node(std::string value);
        Node(Dict map);
        Node(Array array);

        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsBool() const;
        bool IsMap() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;
        bool AsBool() const;
        double AsDouble() const;

        NodeData GetData() const;

        friend bool operator==(const Node& left, const Node& right);

        friend bool operator!=(const Node& left, const Node& right);

    private:
        NodeData data_;
    };

    class Document {
    public:
        Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    bool operator==(const Document& left, const Document& right);

    bool operator!=(const Document& left, const Document& right);

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json