#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <vector>
#include <cstring>
#include <chrono>
// #define NDEBUG
#include <cassert>

#include "parser.h"

using namespace std;

/**
 * @brief Parse netlist files (.v) generated by yosys
 **/

//! Instance a new gate-level netlist parser.
Parser::Parser() {

}

//! Destructor
Parser::~Parser() {

}

void Parser::parse_v(std::string& v_path){
    auto start_v = std::chrono::steady_clock::now();
    cout << "1. Executing netlist file parsing." << endl;
    ifstream inf(v_path);
    if(!inf) {
        cout << "File" << v_path << "Open Error!" <<endl;
        cout << "Exit!" <<endl;
        exit(-1);
    }

    string full_line = "";
    bool process_mode = false;      // the start signal to parse module
    bool end_of_line_found = false;
    unsigned long long lut_num = 0; // the number of lut
    unsigned input_num = 0;
    unsigned output_num = 0;
    unsigned inter_num = 0;
    unsigned input_bits = 0;
    unsigned output_bits = 0;
    unsigned inter_bits = 0;

    while(!inf.eof()) {
        std::string line;
        getline(inf, line);
        line.erase(0, line.find_first_not_of(" ")); 
        line.erase(line.find_last_not_of(" ") + 1);
        if(line == " ") {
            continue;
        }

        vector<std::string> tmp;
        std::string delims = " \t\r\n";
        tmp = Split(line, delims);
        if(tmp.size() == 0) {
            continue;
        }
        if(tmp[0] == "") {
            cout << "ERROR: string " << line << endl;
            exit(-1);
        }
        if(tmp[0] == "(*") {
            continue;
        }

        transform(tmp[0].begin(), tmp[0].end(), tmp[0].begin(), ::tolower);
        if(tmp[0] == "module") {
            process_mode = true;
            if(tmp[1].find("(") != string::npos) {
                top_module_name = tmp[1].substr(0, tmp[1].find("("));
            }
            else {
                top_module_name = tmp[1];
            }        
            continue;
        }
        else if(tmp[0] == "endmodule") {
            process_mode = false;
            break;
        }

        // start parsing
        if(process_mode && !end_of_line_found) {
            // combine lines
            line.erase(0, line.find_first_not_of(" "));
            line.erase(line.find_last_not_of(" ") + 1);
            std::string new_line = line;
            full_line = full_line + " " + new_line;
            if(new_line.size() > 0 && new_line[new_line.size() - 1] == ';') {
                tmp = Split(full_line, " \t\r\n");
                full_line = "";
                end_of_line_found = false;
                if(tmp[0] == "input" || tmp[0] == "output" || tmp[0] == "inout" || tmp[0] == "wire" || tmp[0] == "reg") {
                    Pin cur_pin;

                    // multiple bits pin
                    if(tmp[1].find("[") != std::string::npos) {
                        assert(tmp.size() > 2);

                        // Pin cur_pin;
                        cur_pin.type = tmp[0];
                        std::string tmp_name;
                        if (tmp[2][tmp[2].size() - 1] == ';' || tmp[2][tmp[2].size() - 1] == ';')
                            tmp_name = tmp[2].substr(0, tmp[2].size() - 1);
                        else
                            tmp_name = tmp[2];    
                        cur_pin.name = tmp_name; 
                        if(pins.find(cur_pin.name) != pins.end()) {
                            assert(pins.find(cur_pin.name)->second.type == "input" || pins.find(cur_pin.name)->second.type == "output" || pins.find(cur_pin.name)->second.type == "inout");
                            continue;
                        }                
                        cur_pin.lindex = stoi(tmp[1].substr(tmp[1].find("[") + 1, tmp[1].find(":") - tmp[1].find("[") - 1));                     
                        cur_pin.rindex = stoi(tmp[1].substr(tmp[1].find(":") + 1, tmp[1].find("]") - tmp[1].find(":") - 1));                      
                        cur_pin.size = max(cur_pin.lindex, cur_pin.rindex) - min(cur_pin.lindex, cur_pin.rindex) + 1;                     
                        pins[tmp_name] = cur_pin;

                        int i = cur_pin.lindex;
                        for(int j = 0; j < cur_pin.size; j++) {
                            std::string _name = tmp_name + "[" + to_string(i) + "]";
                            pin_bits.push_back(_name);
                            if(cur_pin.lindex > cur_pin.rindex) {
                                i--;
                            }
                            else {
                                i++;
                            }
                        }
                    }
                    
                    // single bit pin
                    else {
                        // Pin cur_pin;
                        cur_pin.type = tmp[0];
                        std::string tmp_name;
                        if (tmp[1][tmp[1].size() - 1] == ';' || tmp[1][tmp[1].size() - 1] == ';')
                            tmp_name = tmp[1].substr(0, tmp[1].size() - 1);
                        else
                            tmp_name = tmp[1];
                        cur_pin.name = tmp_name;
                        if (pins.find(cur_pin.name) != pins.end()) {                        
                            assert(pins.find(cur_pin.name)->second.type == "input" || pins.find(cur_pin.name)->second.type == "output" || pins.find(cur_pin.name)->second.type == "inout");
                            continue;
                        }
                        cur_pin.lindex = -1;  
                        cur_pin.rindex = -1;                       
                        cur_pin.size = 1;                       
                        pins[tmp_name] = cur_pin;
                        pin_bits.push_back(tmp_name);
                    }

                    // cout << "pin name: ";
                    // cout.setf(ios::left);
                    // cout.width(10);
                    // cout << cur_pin.name;
                    // cout << "type: ";
                    // cout.setf(ios::left);
                    // cout.width(10);
                    // cout << cur_pin.type << "   ---";
                    // cout << "lindex: " << cur_pin.lindex << " ";
                    // cout << "rindex: " << cur_pin.rindex << " ";
                    // cout << "size: " << cur_pin.size << endl;
                }
                
                else if(tmp[0] == "assign") {
                    // pinbitValue parsing
                    if(tmp.size() <= 5) {
                        if(tmp.size() == 2) {
                            assert(tmp[1].find("=") != std::string::npos);
                            assert(tmp[1][tmp[1].size() - 1] == ';');

                            std::string left = tmp[1].substr(0, tmp[1].find("="));
                            std::string right = tmp[1].substr(tmp[1].find("=") + 1, tmp[1].size() - tmp[1].find("=") - 1);
                            if(right == "1\'b1" || right == "1\'b0") {
                                TimedValues* tvs = new TimedValues();
                                TimedValue t_val;
                                t_val.t = 0;
                                t_val.value = (right == "1\'b1" ? VCD_1 : VCD_0);
                                tvs->push_back(t_val);
                                pinbitValues[left] = tvs;
                            }
                            else {
                                assign_pairs[left] = right;
                            }
                        }
                        else if(tmp.size() == 3) {
                            std::string left, right;
                            assert((tmp[1].find("=") != std::string::npos) || (tmp[2].find("=") != std::string::npos));
                            assert(tmp[2][tmp[2].size() - 1] == ';'); 

                            if (tmp[1].find("=") != std::string::npos) {   
                                left = tmp[1].substr(0, tmp[1].find("="));
                                right = tmp[2].substr(0, tmp[2].size() - 1);
                            }
                            else if (tmp[2].find("=") != std::string::npos) {
                                left = tmp[1];
                                right = tmp[2].substr(tmp[2].find("=") + 1, tmp[2].size() - tmp[2].find("=") - 1);
                            }
                            if (right == "1\'b1" || right == "1\'b0") { 
                                TimedValues *tvs = new TimedValues();
                                TimedValue t_val;
                                t_val.t = 0;
                                t_val.value = (right == "1\'b1" ? VCD_1 : VCD_0);
                                tvs->push_back(t_val);
                                pinbitValues[left] = tvs;
                            }
                            else {
                                assign_pairs[left] = right;
                            }
                        }
                        else if(tmp.size() == 4 || tmp.size() == 5) {
                            assert(tmp[2] == "=");
                            assert(tmp[tmp.size() - 1][tmp[tmp.size() - 1].size() - 1] == ';');

                            std::string left = tmp[1];
                            std::string right = (tmp.size() == 4) ? tmp[3].substr(0, tmp[3].size() - 1) : tmp[3];
                            if (right == "1\'b1" || right == "1\'b0") {
                                TimedValues *tvs = new TimedValues();
                                TimedValue t_val;
                                t_val.t = 0;
                                t_val.value = (right == "1\'b1" ? VCD_1 : VCD_0);
                                tvs->push_back(t_val);
                                pinbitValues[left] = tvs;
                            }
                            else {
                                assign_pairs[left] = right;
                            }
                        }
                    }
                              
                    // lut parsing
                    else {
                        //continue;
                        assert(tmp[2] == "=");
                        assert(tmp[4] == ">>");
                        assert(tmp[tmp.size() - 1][tmp[tmp.size() - 1].size() - 1] == ';');

                        LutType cur_lut;
                        cur_lut.num = lut_num; 
                        lut_num += 1;
                        cur_lut.out_ports = tmp[1];
                        vector<string>::iterator i = find(pin_bits.begin(), pin_bits.end(), cur_lut.out_ports);
                        assert(i != pin_bits.end());

                        string tmp_line = "";
                        for(unsigned i = 1; i < tmp.size(); ++i) {
                            tmp_line += tmp[i];
                        }
                        string in_ports_vec = tmp_line.substr(tmp_line.find("{") + 1, tmp_line.find("}") - tmp_line.find("{") - 1);
                        
                        // not combined pins
                        if(in_ports_vec.find(":") == std::string::npos) {
                            vector<string> tmp_in_ports = Split(in_ports_vec, ",");
                            cur_lut.in_ports = tmp_in_ports;
                            for (int i = 0; i < cur_lut.in_ports.size(); i++) {
                                vector<string>::iterator iter = find(pin_bits.begin(), pin_bits.end(), cur_lut.in_ports[i]);
                                assert(iter != pin_bits.end());
                            }

                        }

                        // multiple bits pin
                        else { 
                            /*
                            // e.g. a[5], a[6], b[1:0]
                            if(mul_pin_in_ports.size() == 3) {
                                int mul_pin_index;
                                vector<string> mul_pin;
                                vector<string> tmp_in_ports;
                                for (int i = 0; i < mul_pin_in_ports.size(); ++i) {               
                                    if (mul_pin_in_ports[i].find(":") != std::string::npos) {
                                        mul_pin_index = i;
                                        mul_pin.push_back(mul_pin_in_ports[i].substr(0, mul_pin_in_ports[i].find(":")) + "]");
                                        mul_pin.push_back(mul_pin_in_ports[i].substr(0, mul_pin_in_ports[i].find("[") + 1) + mul_pin_in_ports[i].substr(mul_pin_in_ports[i].find(":") + 1));
                                    }
                                    else {
                                        continue;
                                    }
                                }
                                for (int i = 0; i < mul_pin_index; i++) {
                                    tmp_in_ports.push_back(mul_pin_in_ports[i]);
                                }
                                for (int i = 0; i < mul_pin.size(); i++) {
                                    tmp_in_ports.push_back(mul_pin[i]);
                                }
                                for (int i = mul_pin_index + 1; i < mul_pin_in_ports.size(); i++) {
                                    tmp_in_ports.push_back(mul_pin_in_ports[i]);
                                }
                                cur_lut.in_ports = tmp_in_ports;
                                string in_ports = "";
                                for(vector<string>::iterator iter = cur_lut.in_ports.begin(); iter != cur_lut.in_ports.end(); iter++) {
                                    in_ports += *iter + " ";
                                }
                                cout << "in ports: ";
                                cout.setf(ios::left);
                                cout.width(30);
                                cout << in_ports;
                            }
                            // e.g. b[4:3], a[2:1]
                            // e.g. a[1:0], b[3]
                            // e.g. a[4:2], c[5]
                            else if(mul_pin_in_ports.size() == 2) {
                                continue;
                            }
                            // e.g. b[6:3]
                            else if(mul_pin_in_ports.size() == 1) {
                                //continue;
                                vector<string> mul_pin;
                                string comb_mul_pin = mul_pin_in_ports[0];           
                                int left = stoi(comb_mul_pin.substr(comb_mul_pin.find("[") + 1, comb_mul_pin.find(":") - comb_mul_pin.find("[") - 1));
                                int right = stoi(comb_mul_pin.substr(comb_mul_pin.find(":") + 1, comb_mul_pin.find("]") - comb_mul_pin.find(":") - 1));
                                //cout << left << " " << right << endl;
                                int len = max(left, right) - min(left, right) + 1;
                                assert(len == 4);

                                if(left > right) {
                                    for (int i = 0; i < len; i++) {
                                        mul_pin.push_back(comb_mul_pin.substr(0, comb_mul_pin.find("[") + 1) + to_string(left - i) + "]");
                                    }
                                }
                                else {
                                    for (int i = 0; i < len; i++) { 
                                        mul_pin.push_back(comb_mul_pin.substr(0, comb_mul_pin.find("[") + 1) + to_string(left + i) + "]");
                                    }
                                }

                            }
                            */

                            vector<string> mul_pin_in_ports = Split(in_ports_vec, ",");
                            //vector<string> mul_pin;
                            //map<int, vector<string>> mul_pins;
                            vector<int> mul_pin_index;
                            vector<string> tmp_in_ports;
                            string in_ports = "";
                            int cur_idx = 0;
                            for(int i = 0; i < mul_pin_in_ports.size(); i++) {
                                if(mul_pin_in_ports[i].find(":") != string::npos) {
                                    mul_pin_index.push_back(i);
                                    //cout << i << " ";
                                }
                            }
                            for(int idx : mul_pin_index) {
                                vector<string> mul_pin;
                                int left = stoi(mul_pin_in_ports[idx].substr(mul_pin_in_ports[idx].find("[") + 1, mul_pin_in_ports[idx].find(":") - mul_pin_in_ports[idx].find("[") - 1));
                                int right = stoi(mul_pin_in_ports[idx].substr(mul_pin_in_ports[idx].find(":") + 1, mul_pin_in_ports[idx].find("]") - mul_pin_in_ports[idx].find(":") - 1));
                                //cout << left << " " << right << endl;
                                int len = max(left, right) - min(left, right) + 1;
                                //assert(len <= 4);
                                if (left > right) {
                                    //vector<string> mul_pin;
                                    for (int i = 0; i < len; i++) {
                                        mul_pin.push_back(mul_pin_in_ports[idx].substr(0, mul_pin_in_ports[idx].find("[") + 1) + to_string(left - i) + "]");
                                    }
                                    //mul_pins[idx] = mul_pin;
                                }
                                else {
                                    //vector<string> mul_pin;
                                    for (int i = 0; i < len; i++) {
                                        mul_pin.push_back(mul_pin_in_ports[idx].substr(0, mul_pin_in_ports[idx].find("[") + 1) + to_string(left + i) + "]");
                                    }
                                    //mul_pins[idx] = mul_pin;
                                }
                                //int cur_idx = 0;
                                for(int i = cur_idx; i < idx; i++) {
                                    tmp_in_ports.push_back(mul_pin_in_ports[i]);
                                }
                                //vector<string> cur_mul_pin = mul_pins[idx];
                                /*
                                for (int i = 0; i < cur_mul_pin.size(); i++) {                               
                                    cout << cur_mul_pin[i] << " ";                                   
                                }
                                cout << endl;
                                */
                                /*
                                for(int i = 0; i < cur_mul_pin.size(); i++) {
                                    tmp_in_ports.push_back(cur_mul_pin[i]);
                                }
                                */

                                for (int i = 0; i < mul_pin.size(); i++) {                                
                                    tmp_in_ports.push_back(mul_pin[i]);
                                }
                                cur_idx = idx + 1;
                                
                            }
                            if(cur_idx != mul_pin_in_ports.size()) {
                                for(int i = cur_idx; i < mul_pin_in_ports.size(); i++) {
                                    tmp_in_ports.push_back(mul_pin_in_ports[i]);
                                }                               
                            }
                            cur_lut.in_ports = tmp_in_ports;
                            for (int i = 0; i < cur_lut.in_ports.size(); i++) {    
                                vector<string>::iterator iter = find(pin_bits.begin(), pin_bits.end(), cur_lut.in_ports[i]);
                                assert(iter != pin_bits.end());
                            }

                            // for (vector<string>::iterator iter = cur_lut.in_ports.begin(); iter != cur_lut.in_ports.end(); iter++)
                            // {
                            //     in_ports += *iter + " ";
                            // }
                            // cout << "in ports: ";
                            // cout.setf(ios::left);
                            // cout.width(30);
                            // cout << in_ports;
                        }
                        
                        string tmp_lut_res = tmp[3].substr(tmp[3].find("h") + 1, tmp[3].size() - tmp[3].find("h") - 1);
                        cur_lut.lut_res = tmp_lut_res;        
                        luts[cur_lut.num] = cur_lut;

                        net_from_id[cur_lut.out_ports] = cur_lut.num;
                        for (int i = 0; i < cur_lut.in_ports.size(); i++) {      
                            net_for_id[cur_lut.in_ports[i]].push_back(cur_lut.num);
                        }

                        // string in_ports = "";
                        // for (int i = 0; i < cur_lut.in_ports.size(); i++) {  
                        //     in_ports += cur_lut.in_ports[i] + " ";
                        // }
                        // cout << "lut num: ";
                        // cout.setf(ios::left);
                        // cout.width(5);
                        // cout << cur_lut.num << "   ---";
                        // cout << "out ports: ";
                        // cout.setf(ios::left);
                        // cout.width(10);
                        // cout << cur_lut.out_ports << " ";
                        // cout << "in ports: ";   
                        // cout.setf(ios::left);
                        // cout.width(30);
                        // cout << in_ports;
                        // cout << "lut res: " << cur_lut.lut_res << endl;
                    }
                }
                // alu instance parsing
                else {               
                    continue;
                    // string ins_type = tmp[0];
                }
            }
        }
    }
    
    // for(map<string, int>::iterator i = net_from_id.begin(); i != net_from_id.end(); i++) {
    //     cout << i->first << "   " << i->second << endl; 
    // }
    // cout << "-------------------" << endl;
    // for(map<string, vector<int>>::iterator i = net_for_id.begin(); i != net_for_id.end(); i++) {
    //     cout << i->first << "   ";
    //     vector<int> id_vec = i->second;
    //     for(vector<int>::iterator j = id_vec.begin(); j != id_vec.end(); j++) {
    //         cout << *j << " ";
    //     }
    //     cout << endl;
    // }

    for(map<string, Pin>::iterator i = pins.begin(); i != pins.end(); i++) {
        if (i->second.type == "input") {
            input_num += 1;
            input_bits += i->second.size;
        }
        else if (i->second.type == "output") {
            output_num += 1;
            output_bits += i->second.size;
        }
        else if (i->second.type == "wire") {
            inter_num += 1;
            inter_bits += i->second.size;
        }
        //cout << i->second.type << " " << i->second.name << endl;
    }
    cout << endl;
    cout << "=== " << top_module_name << " ===" << endl;
    cout << endl;
    cout.setf(ios::left);
    cout.width(20);
    cout << "input signals: ";
    cout.unsetf(ios::left);
    cout.width(20);
    cout << input_num << endl;
    cout.setf(ios::left);
    cout.width(20);
    cout << "input bits: ";
    cout.unsetf(ios::left);
    cout.width(20);
    cout << input_bits << endl;
    cout.setf(ios::left);
    cout.width(20);
    cout << "output signals: ";
    cout.unsetf(ios::left);
    cout.width(20);
    cout << output_num << endl;
    cout.setf(ios::left);
    cout.width(20);
    cout << "output bits: ";
    cout.unsetf(ios::left);
    cout.width(20);
    cout << output_bits << endl;
    cout.setf(ios::left);
    cout.width(20);
    cout << "internal signals: ";
    cout.unsetf(ios::left);
    cout.width(20);
    cout << inter_num << endl;
    cout.setf(ios::left);
    cout.width(20);
    cout << "inter bits: ";
    cout.unsetf(ios::left);
    cout.width(20);
    cout << inter_bits << endl;
    cout.setf(ios::left);
    cout.width(20);
    cout << "$lut cells: ";
    cout.unsetf(ios::left);
    cout.width(20);
    cout << lut_num << endl;
    cout << endl;

    auto end_v = std::chrono::steady_clock::now();
    long duration_v = std::chrono::duration_cast<std::chrono::milliseconds>(end_v - start_v).count();
    cout << "Successfully finished netlist file parsing. (" << duration_v << "ms) " << endl;
    cout << endl;
    inf.close();
}

vector<std::string> Parser::Split(const std::string &str, const std::string &delim) {
    vector<std::string> res;
    if(str == ""){
        return res;
    }
    char *strs = new char[str.length() + 1];
    strcpy(strs, str.c_str());
    char *delims = new char[delim.length() + 1];
    strcpy(delims, delim.c_str());

    char *p = strtok(strs, delims);

    while(p) {
        std::string s = p;
        res.push_back(s);
        p = strtok(NULL, delims);
    }
    return res;
}