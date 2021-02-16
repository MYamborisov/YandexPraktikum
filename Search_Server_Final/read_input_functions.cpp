#include "read_input_functions.h"

#include <iostream>

using namespace std;

string ReadLine() {
    string input_string;
    getline(cin, input_string);
    return input_string;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}