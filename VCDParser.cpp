#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <thread>

#include "VCDParser.h"
        
using namespace std;

//! Instance a new VCD file container.
VCDParser::VCDParser(){

}
        
//! Destructor
VCDParser::~VCDParser(){

    // Delete signals and scopes.
    delete root_scope;

    

    for (std::vector<VCDScope*>::iterator j = scopes.begin(); j != scopes.end(); ++j) {
    
        for (std::vector<VCDSignal*>::iterator i = ((*j)->signals).begin(); i != ((*j)->signals).end(); ++i)
        {
            delete *i;
        }
        
        delete *j;
    }
    for (std::vector<VCDScope*>::iterator j = _scopes.begin(); j != _scopes.end(); ++j) {
    
        for (std::vector<VCDSignal*>::iterator i = ((*j)->signals).begin(); i != ((*j)->signals).end(); ++i)
        {
            delete *i;
        }

        delete *j;
    }
    for(auto it:signals_map){
        delete it.second;
    }
    for (std::vector<VCDSignal*>::iterator i = signals.begin(); i != signals.end(); ++i){
        delete *i;
    }

    // Delete signal values.
    
    for(auto hash_val = times.begin();
             hash_val != times.end();
             ++hash_val)
    {
        delete hash_val -> second;
    }
    for(auto hash_val = values.begin();
             hash_val != values.end();
             ++hash_val)
    {
        delete hash_val -> second;
    }

}


/*!
@brief Add a new scope object to the VCD file
*/
void VCDParser::add_scope(
    VCDScope * s
){
    scopes.push_back(s);
}


/*!
@brief Add a new signal object to the VCD file
*/
void VCDParser::add_signal(
    VCDSignal * s
){
    signals.push_back(s);
    if (signals_map.find(s->hash) == signals_map.end())
    {
        signals_map[s->hash] = s;
    }
    // Add a timestream entry
    /*if(val_map.find(s -> reference) == val_map.end()) {
        // Values will be populated later.
        val_map[s -> hash] = new VCDSignalValues();
    }*/
}


/*!
 */
VCDScope *VCDParser::get_scope(VCDScopeName name) {
  for (std::vector<VCDScope*>::iterator i = scopes.begin(); i != scopes.end(); ++i)
    {
    if ((*i)->name == name) {
      return *i;
    }
  }
  return NULL;
}


/*!
@brief Add a new signal value to the VCD file, tagged by time.
*/
void VCDParser::add_signal_value(
    unsigned int time,
    VCDBit bit,
    std::string   name
){
    times[name]->push_back(time);
    values[name]->push_back(bit);
}


/*!
*/
/*std::vector<VCDTime> VCDParser::get_timestamps(){
    return times;
}*/


/*!
*/
std::vector<VCDScope*> VCDParser::get_scopes(){
    return _scopes;
}


/*!
*/
std::vector<VCDSignal*> VCDParser::get_signals(){
    return signals;
}


/*!
*/
/*void VCDParser::add_timestamp(
    VCDTime time
){
    times.push_back(time);
}*/

/*!
*/
VCDValue * VCDParser::get_signal_value_at (
    const VCDSignalHash& hash,
    VCDTime       time,
    bool erase_prior
){/*
    auto find = val_map.find(hash); 
    if(find == val_map.end()) {
        return NULL;
    }
    
    VCDSignalValues * vals = find->second;

    if(vals -> size() == 0) {
        return NULL;
    }

    VCDSignalValues::iterator erase_until = vals->begin();

    VCDValue * tr = NULL;

    for(VCDSignalValues::iterator it = vals -> begin();
             it != vals -> end();
             ++ it) {

        if((*it) -> time <= time) {
            erase_until = it;
            tr = (*it) -> value;
        } else {
            break;
        }
    }

    if (erase_prior) {
        // avoid O(n^2) performance for large sequential scans
        for (VCDSignalValues::iterator i = vals->begin() ; i != erase_until; i++) {
            delete (*i) -> value;
        }
        vals->erase(vals->begin(), erase_until);
    }

    return tr;*/
    return 0;
}

/*TimedValues * VCDParser::get_signal_values (
    VCDSignalHash hash
){
    if(val_map.find(hash) == val_map.end()) {
        return NULL;
    }

    return val_map[hash];
}*/

bool VCDParser::parse(std::string& vcdFilePath){//, std::map<std::string, int>& initial_net_map){
    //unsigned size = initial_net_map.size();
    //times.resize(size);
    //values.resize(size);
    ifstream inf(vcdFilePath);
    if (!inf)
    {
        cout << "File " << vcdFilePath << " Open Error!" << endl;
        cout << "Exit!" << endl;
        return false;
    }
    unsigned int current_time = 0;
    unsigned int last_time = 0;
    //times.push_back(0);
    while(!inf.eof())
    {
        string line;
        getline(inf, line);
        line.erase(0, line.find_first_not_of(" "));
        //parse date 
        if (line.find("$date") != std::string::npos)
        {
            std::vector<string> line_vec;
            if (line.find("$end") != std::string::npos)
            {
                line_vec = SplitBySpace(line);
            }
            else
            {
                string new_line = line;
                while(getline(inf, line))
                {
                    line.erase(0, line.find_first_not_of(" "));
                    new_line += " ";
                    new_line += line;
                    if (line.find("$end") != std::string::npos)
                    {
                        break;
                    }
                }
                line_vec = SplitBySpace(new_line);
            }
            string date_ = "";
            for (unsigned i = 1; i < line_vec.size()-1; ++i)
            {
                date_ += line_vec[i];
                date_ += " ";
            }
            date = date_;
            //cout << "Date: " << date << endl;
        }
        //parse version
        else if (line.find("$version") != std::string::npos)
        {
            std::vector<string> line_vec;
            if (line.find("$end") != std::string::npos)
            {
                line_vec = SplitBySpace(line);
            }
            else
            {
                string new_line = line;
                while(getline(inf, line))
                {
                    line.erase(0, line.find_first_not_of(" "));
                    new_line += " ";
                    new_line += line;
                    if (line.find("$end") != std::string::npos)
                    {
                        break;
                    }
                }
                line_vec = SplitBySpace(new_line);
            }
            string version_ = "";
            for (unsigned i = 1; i < line_vec.size()-1; ++i)
            {
                version_ += line_vec[i];
                version_ += " ";
            }
            version = version_;
            //cout << "Version: " << version << endl;
        }
        //parse timescale
        else if (line.find("$timescale") != std::string::npos)
        {
            std::vector<string> line_vec;
            if (line.find("$end") != std::string::npos)
            {
                line_vec = SplitBySpace(line);
            }
            else
            {
                string new_line = line;
                while(getline(inf, line))
                {
                    line.erase(0, line.find_first_not_of(" "));
                    new_line += " ";
                    new_line += line;
                    if (line.find("$end") != std::string::npos)
                    {
                        break;
                    }
                }
                line_vec = SplitBySpace(new_line);
            }
            string scale = line_vec[1];
            string num = "";
            string unit = "";
            for (string::iterator i = scale.begin(); i != scale.end(); ++i)
            {
                if (*i >= '0' && *i <= '9')    //number
                {
                    num += *i;
                }
                else
                {
                    unit += *i;
                }
            }
            time_resolution = stoi(num);
            if (unit == "s")
            {
                time_units = TIME_S;
            }
            else if(unit == "ms")
            {
                time_units = TIME_MS;
            }
            else if(unit == "ns")
            {
                time_units = TIME_NS;
            }
            else if(unit == "us")
            {
                time_units = TIME_US;
            }
            else if(unit == "ps")
            {
                time_units = TIME_PS;
            }
            //cout << "Timescale: " << time_resolution << "(unit:)" << time_units << endl;
            
        }
        //parse scope
        else if (line.find("$scope") != std::string::npos)
        {
            std::vector<string> line_vec;
            if (line.find("$end") != std::string::npos)
            {
                line_vec = SplitBySpace(line);
            }
            else
            {
                string new_line = line;
                while(getline(inf, line))
                {
                    line.erase(0, line.find_first_not_of(" "));
                    new_line += " ";
                    new_line += line;
                    if (line.find("$end") != std::string::npos)
                    {
                        break;
                    }
                }
                line_vec = SplitBySpace(new_line);
            }
            assert(line_vec.size() == 4);
            VCDScope * new_scope = new VCDScope();
            new_scope -> type = line_vec[1];
            new_scope -> name = line_vec[2];
            if (scopes.size() > 0)
            {
                new_scope -> parent = scopes[scopes.size() - 1];
                scopes[scopes.size() - 1] -> children.push_back(new_scope);
            }
            else
            {
                root_scope = new_scope;
                new_scope -> parent = NULL;
            }
            add_scope(new_scope);
            //cout << "Current scope: " << new_scope -> name << endl;
        }
        //parse signal definition
        else if (line.find("$var") != std::string::npos)
        {
            std::vector<string> line_vec;
            if (line.find("$end") != std::string::npos)
            {
                line_vec = SplitBySpace(line);
            }
            else
            {
                string new_line = line;
                while(getline(inf, line))
                {
                    line.erase(0, line.find_first_not_of(" "));
                    new_line += " ";
                    new_line += line;
                    if (line.find("$end") != std::string::npos)
                    {
                        break;
                    }
                }
                line_vec = SplitBySpace(new_line);
            }
            assert(line_vec.size() == 6 || line_vec.size() == 7);
            VCDSignal * new_signal = new VCDSignal();
            new_signal -> type = line_vec[1];
            new_signal -> size = stoi(line_vec[2]);
            new_signal -> hash = line_vec[3];
            new_signal -> reference = line_vec[4];
            // set_name_hash_pair(line_vec[4], line_vec[3]);
            // set_hash_name_pair(line_vec[3], line_vec[4]);
            if (line_vec.size() == 7)
            {
                string tmp = line_vec[5];
                std::size_t pos1, pos2, pos3;
                pos1 = tmp.find("[");
                pos2 = tmp.find(":");
                pos3 = tmp.find("]");
                assert(pos1 != std::string::npos && pos3 != std::string::npos);
                if (pos2 != std::string::npos)
                {
                    set_name_hash_pair(line_vec[4], line_vec[3]);
                    set_hash_name_pair(line_vec[3], line_vec[4]);
                    new_signal -> lindex = stoi(tmp.substr(pos1+1,pos2-pos1-1));
                    new_signal -> rindex = stoi(tmp.substr(pos2+1,pos3-pos2-1));
                    int start = new_signal -> lindex;
                    for (unsigned i = 0; i < new_signal->size; ++i)
                    {
                        string name = new_signal->reference + "[" + to_string(start) + "]";
                        if (times.find(name) == times.end())
                        {
                            times[name] = new vector<unsigned int>();
                            values[name] = new vector<short>();
                        }
                        /*if (initial_net_map.find(name) == initial_net_map.end())
                        {
                            initial_net_map[name] = size++;
                            times.resize(size);
                            values.resize(size);
                        }*/
                        if(new_signal->lindex < new_signal->rindex)
                            start++;
                        else
                            start--;
                    }
                }
                else
                {
                    set_name_hash_pair(line_vec[4] + line_vec[5], line_vec[3]);
                    set_hash_name_pair(line_vec[3], line_vec[4] + line_vec[5]);
                    new_signal -> lindex = stoi(tmp.substr(pos1+1,pos3-pos1-1));
                    new_signal -> rindex = -1;
                    string name = new_signal->reference + "[" + to_string(new_signal -> lindex) + "]";
                    if (times.find(name) == times.end())
                    {
                        times[name] = new vector<unsigned int>();
                        values[name] = new vector<short>();
                    }
                    /*if (initial_net_map.find(name) == initial_net_map.end())
                    {
                        initial_net_map[name] = size++;
                        times.resize(size);
                        values.resize(size);
                    }*/
                }
            }
            else
            {
                set_name_hash_pair(line_vec[4], line_vec[3]);
                set_hash_name_pair(line_vec[3], line_vec[4]);
                if(new_signal -> size == 1)
                {
                    new_signal -> lindex = -1;
                    new_signal -> rindex = -1;
                    string name = new_signal->reference;
                    if (times.find(name) == times.end())
                    {
                        times[name] = new vector<unsigned int>();
                        values[name] = new vector<short>();
                    }
                    /*if (initial_net_map.find(name) == initial_net_map.end())
                    {
                        initial_net_map[name] = size++;
                        times.resize(size);
                        values.resize(size);
                    }*/
                }
                else
                {
                    new_signal -> lindex = new_signal->size - 1;
                    new_signal -> rindex = 0;
                    for (unsigned i = 0; i < new_signal->size; ++i)
                    {
                        string name = new_signal->reference + "[" + to_string(i) + "]";
                        if (times.find(name) == times.end())
                        {
                            times[name] = new vector<unsigned int>();
                            values[name] = new vector<short>();
                        }
                        /*if (initial_net_map.find(name) == initial_net_map.end())
                        {
                            initial_net_map[name] = size++;
                            times.resize(size);
                            values.resize(size);
                        }*/
                    }
                }
                
            }

            VCDScope * cur_scope = scopes[scopes.size() - 1];
            new_signal->scope = cur_scope;
            cur_scope->signals.push_back(new_signal);
            add_signal(new_signal);
            //cout << "Signal: " << new_signal -> hash << "(" << new_signal -> reference << ")" << ", edge:" << new_signal -> lindex << ":" << new_signal -> rindex <<endl;
        }
        //parse upscope
        else if (line.find("$upscope") != std::string::npos)
        {
            _scopes.push_back(scopes.back());
            scopes.pop_back();
        }
        //parse dumpvars --- initial vals
        else if (line.find("$dumpvars") != std::string::npos)
        {
            //cout << "Initial: " << endl;
            std::vector<string> line_vec;
            while(getline(inf, line))
            {
                if (line.find("$end") != std::string::npos)
                {
                    break;
                }
                line_vec = SplitBySpace(line);
                /*TimedValue toadd;
                toadd.t = current_time;*/
                string hash;
                //bit value
                if (line_vec.size() == 1)
                {
                    string tmp = line_vec[0];
                    VCDBit bit;
                    switch(tmp[0]){
                        case('0'):
                            bit = VCD_0;
                            break;
                        case('1'):
                            bit = VCD_1;
                            break;
                        case('z'):
                        case('Z'):
                            bit = VCD_X;
                            break;
                        case('x'):
                        case('X'):
                        default:
                            bit = VCD_X;
                            break;
                    }
                    //toadd.value = bit;
                    hash = tmp.substr(1,tmp.size()-1);
                    string name = hash_name_pair[hash];
                    /*if (initial_net_map.find(name) != initial_net_map.end())
                    {
                        times[initial_net_map[name]].push_back(current_time);
                        values[initial_net_map[name]].push_back(bit);
                    }*/
                    add_signal_value(current_time, bit, name);
                }
                else
                {
                    hash = line_vec[1];
                    string name = hash_name_pair[hash];
                    VCDSignal* tmp_signal = get_signal_by_hash(hash);
                    if(!tmp_signal){
                        std::cout << "EEROR: can't find signal -- hash is " << hash << endl;
                        exit(-1);
                    }

                    string tmp = line_vec[0];
                    if (tmp[0] == 'b' || tmp[0] == 'B')    // bit vector
                    {
                        //VCDBitVector * vec = new VCDBitVector();
                        string val = tmp.substr(1, tmp.size()-1);
                        unsigned real_size = tmp_signal->size;
                        //expand bit --- unsigned
                        assert(real_size >= val.size());
                        if (real_size > val.size())
                        {
                            val = (val[0] == 'x'?std::string(real_size - val.size(), 'x'):std::string(real_size - val.size(), '0')) + val;
                        }
                        int start = tmp_signal->lindex;
                        for (unsigned i = 0; i < val.size(); ++i)
                        {
                            string real_name = name + "[" + to_string(start) + "]";
                            VCDBit bit;
                            switch(val[i]){
                                case('0'):
                                    //toadd.value = VCD_0;
                                    bit = VCD_0;
                                    break;
                                case('1'):
                                    //toadd.value = VCD_1;
                                    bit = VCD_1;
                                    break;
                                case('z'):
                                case('Z'):
                                    //toadd.value = VCD_X;
                                    bit = VCD_X;
                                    break;
                                case('x'):
                                case('X'):
                                default:
                                    //toadd.value = VCD_X;
                                    bit = VCD_X;
                                    break;
                            }
                            /*if (initial_net_map.find(real_name) != initial_net_map.end())
                            {
                                times[initial_net_map[real_name]].push_back(current_time);
                                values[initial_net_map[real_name]].push_back(bit);
                            }*/
                            add_signal_value(current_time, bit, real_name);
                            if(tmp_signal->lindex < tmp_signal->rindex)
                                start++;
                            else
                                start--;
                        }
                        
                    }
                    /*else if(tmp[0] == 'r' || tmp[0] == 'R') // real value
                    {
                        //not supported
                        VCDReal real_val;
                        string rval = tmp.substr(1, tmp.size()-1);
                        const char* rrval = rval.c_str();
                        float rval_f;
                        sscanf(rrval, "%g", &rval_f);
                        real_val = rval_f;
                        toadd->value = new VCDValue(real_val);
                    }*/
                    else
                    {
                        cout << "ERROR: not supported value type!" << endl;
                        return false;
                    }
                }
                //cout << "\thash:" << hash << endl;
            }
        }
        //parse VCDtime
        else if (line[0] == '#')
        {
            string tmp = line.substr(1, line.size()-1);
            stringstream num_str;
            num_str << tmp;
            num_str >> current_time;
            if (current_time > last_time)
            {
                //add_timestamp(current_time);
                last_time = current_time;
                end_time = current_time;
            }
            //cout << "Current time: " << current_time << endl;
        }
        //parse value change
        else if (line[0] != '$')
        {
            std::vector<string> line_vec;
            line_vec = SplitBySpace(line);
            //TimedValue toadd;
            //toadd.t = current_time;
            string hash;
            if (line_vec.size() < 3 && line_vec.size() > 0)
            {
                if (line_vec.size() == 1)
                {
                    string tmp = line_vec[0];
                    VCDBit bit;
                    switch(tmp[0]){
                        case('0'):
                            bit = VCD_0;
                            break;
                        case('1'):
                            bit = VCD_1;
                            break;
                        case('z'):
                        case('Z'):
                            bit = VCD_X;
                            break;
                        case('x'):
                        case('X'):
                        default:
                            bit = VCD_X;
                            break;
                    }
                    //toadd.value = bit;
                    hash = tmp.substr(1,tmp.size()-1);
                    string name = hash_name_pair[hash];
                    /*if (initial_net_map.find(name) != initial_net_map.end())
                    {
                        times[initial_net_map[name]].push_back(current_time);
                        values[initial_net_map[name]].push_back(bit);
                    }*/
                    if (times.find(name)!=times.end())
                    {
                        add_signal_value(current_time, bit, name);
                    }
                }
                else
                {
                    hash = line_vec[1];
                    string name = hash_name_pair[hash];
                    VCDSignal* tmp_signal = get_signal_by_hash(hash);
                    if(!tmp_signal){
                        std::cout << "EEROR: can't find signal -- hash is " << hash << endl;
                        exit(-1);
                    }
                    string tmp = line_vec[0];
                    if (tmp[0] == 'b' || tmp[0] == 'B')    // bit vector
                    {
                        //VCDBitVector * vec = new VCDBitVector();
                        string val = tmp.substr(1, tmp.size()-1);
                        unsigned real_size = tmp_signal->size;
                        //expand bit --- unsigned
                        assert(real_size >= val.size());
                        if (real_size > val.size())
                        {
                            val = (val[0] == 'x'?std::string(real_size - val.size(), 'x'):std::string(real_size - val.size(), '0')) + val;
                        }
                        int start = tmp_signal->lindex;
                        for (unsigned i = 0; i < val.size(); ++i)
                        {
                            string real_name = name + "[" + to_string(start) + "]";
                            VCDBit bit;
                            switch(val[i]){
                                case('0'):
                                    //toadd.value = VCD_0;
                                    bit = VCD_0;
                                    break;
                                case('1'):
                                    //toadd.value = VCD_1;
                                    bit = VCD_1;
                                    break;
                                case('z'):
                                case('Z'):
                                    //toadd.value = VCD_X;
                                    bit = VCD_X;
                                    break;
                                case('x'):
                                case('X'):
                                default:
                                    //toadd.value = VCD_X;
                                    bit = VCD_X;
                                    break;
                            }
                            if (values.find(real_name)!=values.end())
                            {
                                short tmp_val = values[real_name]->back();
                                VCDBit temp_val = VCDBit(tmp_val);
                                if(temp_val != bit)
                                    add_signal_value(current_time, bit, real_name);
                            }
                            /*if (initial_net_map.find(real_name) != initial_net_map.end())
                            {
                                if (times[initial_net_map[real_name]].back() != bit)
                                {
                                    times[initial_net_map[real_name]].push_back(current_time);
                                    values[initial_net_map[real_name]].push_back(bit);
                                }
                            }*/
                            if(tmp_signal->lindex < tmp_signal->rindex)
                                start++;
                            else
                                start--;
                        }
                    }
                    /*else if(tmp[0] == 'r' || tmp[0] == 'R') // real value
                    {
                        VCDReal real_val;
                        string rval = tmp.substr(1, tmp.size()-1);
                        const char* rrval = rval.c_str();
                        float rval_f;
                        sscanf(rrval, "%g", &rval_f);
                        real_val = rval_f;
                        toadd->value = new VCDValue(real_val);
                    }*/
                    else
                    {
                        cout << "ERROR: not supported value type!" << endl;
                        return false;
                    }
                }
                
                //cout << "\thash: " << hash <<endl;
            }
        }
        else
        {
            cout << "INFO: skip line: " << line << endl;
        }
    }
    cout << "VCD parse done!" << endl;
    return true;
}

std::vector<std::string> VCDParser::SplitBySpace(std::string& str)
{
    std::string buf;
    std::stringstream ss(str);

    std::vector<std::string> tokens;

    while (ss >> buf)
        tokens.push_back(buf);

    return tokens;
}

