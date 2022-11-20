#ifndef INPUTFIELD_HPP
#define INPUTFIELD_HPP
#include <vector>
#include <string>


enum input_field_type {COMBO_LIST, STRING, INTEGER, INTEGER_RANGE};
extern std::vector<std::string> empty_str_list;


class input_field {

protected:
    input_field_type type;
    std::string tooltip;
public:

    void set_tooltip(const std::string &t){tooltip = t;};
    std::string get_tooltip() {return tooltip;};
    virtual std::vector<std::string> &get_combo(){return empty_str_list;};
    virtual std::string get_combo_selected() const{return "Undefined";};
    virtual std::string get_combo_at(int index){return "Undefined";};
    virtual int get_combo_index() const {return -1;};
    virtual int get_int() const{return -1;};
    virtual std::string get_string() const{return "Undefined";};
    input_field_type get_type() const{return type;};

    virtual void set_int(int n){};
    virtual void set_string(std::string s){};
    virtual void set_combo(std::string &combo){};
    virtual void set_combo(int index){};
};



class integer_field : public input_field
{
    int int_val;
public:
    integer_field(int val){
        int_val = val;
        type = INTEGER;
    };
    void set_int(int n){int_val = n;};
    int get_int() const{return int_val;};

};

class string_field : public input_field
{
    std::string string_val;
public:
    void set_string(std::string s){string_val = s;};
    std::string get_string() const{return string_val;};
    string_field(const std::string &string)
    {
        string_val = string;
        type = STRING;
    };
};

class integer_range_field : public input_field
{
    int int_val;
    int max, min;
public:
    integer_range_field(int val, int _min=INT_MIN, int _max=INT_MAX){
        int_val = val;
        min = _min;
        max = _max;
        type = INTEGER_RANGE;
    };
    void set_int(int n){if (n >= min && n <= max) int_val = n;};
    int get_int() const{return int_val;};

};

class combo_list_field : public input_field
{
private:
    std::vector<std::string> options;
    int current_item_index;

public:
    combo_list_field(const std::vector<std::string> &o, unsigned int index){
        options = o;
        type = COMBO_LIST;
        current_item_index = index < options.size() ? index : 0;
    };
    int get_combo_index() const { return current_item_index; };

    std::vector<std::string> &get_combo(){return options;};
    std::string get_combo_selected() const{return options.at(current_item_index);};
    std::string get_combo_at(int index){if (index < options.size()) return options.at(index);else return "Undefined";};

    void set_combo(std::string &combo){
        for (int i = 0; i < options.size() ; i++)
        {
            if (options[i] == combo) current_item_index = i;
        }
    };
    void set_combo(int index){if (index < options.size()) current_item_index = index;};


};

#endif // INPUTFIELD_HPP




