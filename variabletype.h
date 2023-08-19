#ifndef VARIABLE_TYPE_H
#define VARIABLE_TYPE_H

#include <string>

class variable {
    int integer_value;
    std::string string_value;
    char character_value;
    double double_value;

    enum variable_type {INTEGER, STRING, CHARACTER, DOUBLE} type;

    public:

    variable(int i) : integer_value(i) {type = INTEGER;};
    variable(std::string s) : string_value(s) {type = STRING;};   
    variable(char c) : character_value(c) {type = CHARACTER;};
    variable(double d) : double_value(d) {type = DOUBLE;};

    operator int() const { return integer_value; };
    operator std::string() const { return string_value; };    
    operator char() const {return character_value; };
    operator double() const {return double_value; };

    variable &operator=(const int& i){integer_value = i; return *this;};
    variable &operator=(const std::string& s){string_value = s; return *this;};
    variable &operator=(const char& c){character_value = c; return *this;};
    variable &operator=(const double &d){double_value = d; return *this;};
};


#endif // VARIABLE_TYPE_H
