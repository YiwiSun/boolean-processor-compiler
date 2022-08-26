#include <iostream>
#include <fstream> 
#include <cstring>
#include <chrono>
#include <algorithm>
#include <thread>
#include <limits>
#include <cstddef>
// #define NDEBUG
#include <cassert>
#include <queue>

// #include <boost/serialization/string.hpp>
// #include <boost/serialization/vector.hpp>
// #include <boost/serialization/map.hpp>
// #include <boost/serialization/deque.hpp>
// #include <boost/serialization/access.hpp>
// #include <boost/archive/text_iarchive.hpp>
// #include <boost/archive/text_oarchive.hpp>

// #include <boost/archive/impl/basic_text_oarchive.ipp>
// #include <boost/archive/impl/text_oarchive_impl.ipp>

#include "Preprocess.h"
#include "Parser.h"
#include "VCDTypes.h"

using namespace std;

// class MyOArchive : public boost::archive::text_oarchive_impl<MyOArchive> {
// public:
//     bool MyData;

//     friend class boost::archive::detail::common_oarchive<MyOArchive>;
//     friend class basic_text_oarchive<MyOArchive>;
//     friend class boost::archive::save_access;

//     MyOArchive(std::ostream &os, unsigned int flags = 0) : boost::archive::text_oarchive_impl<MyOArchive>(os, flags) {}
// };

// class inter {
// public:
//     inter() {}
//     ~inter() {}

//     /* data */
//     Parser p;
//     std::vector<std::vector<int>> levels;
//     std::map<std::string, int> initial_net_map; // net : inintial id

//     std::vector<unsigned int> data_in_num_start;
//     std::vector<unsigned int> data_out_num_start;
//     // std::vector<unsigned int> delay_start;
//     // std::vector<unsigned int> delay_width;
//     std::vector<unsigned int> functions_start;
//     // std::vector<unsigned int> functions_width;

//     std::vector<short> data_in_num;             // in ports num of each lut
//     std::vector<short> data_out_num;            // out ports num of each lut
//     std::vector<unsigned int> val_num_start;    // input val starting id of each lut
//     // std::vector<short> delay_val_num;
//     // std::vector<short> functions_func_num;
//     // std::vector<short> functions_val_num;
//     // std::vector<short> host_delay_edges;
//     // std::vector<short> host_in_bit;
//     // std::vector<short> host_out_bit;
//     // std::vector<float> host_rise_val;
//     // std::vector<float> host_fall_val;

//     // std::vector<short> host_functions;

//     template<class Archive>
//     void serialize(Archive & ar, const unsigned int version) {
//         ar & p;
//         ar & levels;
//         ar & initial_net_map;
//         ar & data_in_num_start;
//         ar & data_out_num_start;
//         // ar & delay_start;
//         // ar & delay_width;
//         // ar & functions_start;
//         // ar & functions_width;
//         ar & data_in_num;
//         ar & data_out_num;
//         ar & val_num_start;
//         // ar & delay_val_num;
//         // ar & functions_func_num;
//         // ar & functions_val_num;
//         // ar & host_delay_edges;
//         // ar & host_in_bit;
//         // ar & host_out_bit;
//         // ar & host_rise_val;
//         // ar & host_fall_val;
//         // ar & host_functions;
//     }
// };

/**
 * @brief generate intermediate parm via pre-processing
 **/

//! Instance a new Pre-processing Class
inter::inter() {

}

//! Destructor
inter::~inter() {

}

// Generate adjacency list
void inter::GenAdjList(map<int, LutType> &luts, map<string, vector<int>> &net_for_id, /*map<string, int> &net_from_id,*/ \
map<int, vector<int>> &adjList/*, vector<int> &edgesSize, vector<int> &edgesOffset*/) {
    // for (map<string, vector<int>>::iterator i = net_for_id.begin(); i != net_for_id.end(); i++)
    // {
    //     if (net_from_id.find(i->first) != net_from_id.end())
    //     {
    //         int id = net_from_id.find(i->first)->second;
    //         adjList[id] = i->second;
    //     }
    // }

    int n = luts.size();
    for(unsigned i = 0; i < n; i++) {
        LutType cur_lut = luts.find(i)->second;
        string cur_out = cur_lut.out_ports;
        if(net_for_id.find(cur_out) != net_for_id.end()) {
            adjList[i] = net_for_id[cur_out];
        }
        sort(adjList[i].begin(), adjList[i].end());
        adjList[i].erase(unique(adjList[i].begin(), adjList[i].end()), adjList[i].end());
        // edgesSize.push_back(adjList[i].size());
    }

    // for (map<int, vector<int>>::iterator i = adjList.begin(); i != adjList.end(); i++)
    // {
    //     cout << i->first << " --- ";
    //     for (int j = 0; j < i->second.size(); j++)
    //     {
    //         cout << i->second[j] << " ";
    //     }
    //     cout << endl;
    // }
}

void inter::SetInDegrees(map<int, vector<int>> &adjList, vector<int> &indegrees) {
    for(map<int, vector<int>>::iterator i = adjList.begin(); i != adjList.end(); i++) {
        for(int j = 0; j < i->second.size(); j++) {
            indegrees[i->second[j]] += 1; 
        }
    }
}

void inter::TopologyOrder(map<int, vector<int>> &adjList, vector<int> &indegrees, int &n, \
vector<vector<int>> &levels) {
    vector<int> Q;
    vector<int> tmp_Q;
    for(int i = 0; i < n; i++) {  
        if (indegrees[i] == 0) {
            Q.push_back(i);           
        }     
    }
    while(!Q.empty()) { 
        levels.push_back(Q);
        for (int i = 0; i < Q.size(); i++) {
            n--;
            int tmp = Q[i];
            for(int j = 0; j < adjList[tmp].size(); j++) {
                int v = adjList[tmp][j];
                indegrees[v] -= 1;
                if (indegrees[v] == 0) {
                    tmp_Q.push_back(v);
                }
            }      
        }
        Q = tmp_Q; 
        tmp_Q.clear();
    }
    assert(n <= 0);
}


// int main(int argc, char **argv) {
//     string v_path, inter_file;

void inter::preprocess(string& v_path) {
    // Parser _parser;

    // if(argc == 3) {
    //     v_path = argv[1];
    //     inter_file = argv[2];
    // }
    // else {
    //     cout << "[USAGE] ./Preprocess v_path [inter_file]" << endl;
    //     exit(-1);
    // }

    /** pre process **/
    _parser.parse_v(v_path);

    map<int, LutType> luts = _parser.get_luts();
    map<string, TimedValues*> pinbitValues = _parser.get_pinbitValues();
    map<string, string> assign_pairs = _parser. get_assign_pairs();
    map<string, vector<int>> net_for_id = _parser.net_for_id;
    map<string, int> net_from_id = _parser.net_from_id;

    // for gragh generation
    map<int, vector<int>> adjList;  // vertex : list of edges
    // vector<int> edgesSize;          // number of edges of each vertex
    int n = luts.size();
    vector<int> indegrees(n);
    // vector<vector<int>> levels;



    /***************************** topology ordering ***************************/
    cout << "2. Executing topology ordering." << endl;
    auto start_pre = std::chrono::steady_clock::now();
    // generate adjacency list
    // cout << "Generating topological structure." << " ";
    auto start_graph = chrono::steady_clock::now();
    GenAdjList(luts, net_for_id, adjList);
    auto end_graph = chrono::steady_clock::now();
    long duration_graph = chrono::duration_cast<chrono::milliseconds> (end_graph - start_graph).count();
    // cout << "(" << duration_graph << "ms" << ")" << endl;

    // topology order
    // cout << "Generating hierarchical network." << " ";
    auto start_top = chrono::steady_clock::now();
    SetInDegrees(adjList, indegrees);
    TopologyOrder(adjList, indegrees, n, levels);
    auto end_top = chrono::steady_clock::now();
    long duration_top = chrono::duration_cast<chrono::milliseconds> (end_top-start_top).count();
    // cout << "(" << duration_top << "ms" << ")" << endl;
    // cout << "levels size: " << levels.size() << endl;



    /*************************** process levels *******************************/
    vector<int> tmp_lut_level(luts.size());
    vector<int> tmp_lut_pos_at_level(luts.size());
    for (unsigned i = 0; i < levels.size(); i++) {
        vector<int> cur_level = levels[i];
        for (unsigned j = 0; j < cur_level.size(); j++) {
            int cur_id = cur_level[j];
            tmp_lut_level[cur_id] = i;
            tmp_lut_pos_at_level[cur_id] = j;
        }
    }



    /************************* for gragh generation ***********************/
    // int initial_id = 0;
    // edges = 0;
    // vector<vector<int>> tmp_adjncys(luts.size());

    // for (unsigned i = 0; i < luts.size(); ++i) {
    //     LutType cur_lut = luts[i];
    //     std::vector<string> _in_net = cur_lut.in_ports;
    //     for (unsigned j = 0; j < _in_net.size(); ++j) {
    //         string cur_in = _in_net[j];
    //         if (net_from_id.find(cur_in) != net_from_id.end()) {
    //             edges += 1;
    //             int _id = net_from_id[cur_in];
    //             // cout << "id: " << _id << endl;
    //             tmp_adjncys[luts[i].num].push_back(_id);
    //             tmp_adjncys[_id].push_back(luts[i].num);
    //             luts[i].in_net_from_id.push_back(_id);
    //             luts[i].in_net_from_info.push_back(" ");
    //             luts[i].in_net_from_level.push_back(tmp_lut_level[_id]);
    //             luts[i].in_net_from_pos_at_level.push_back(tmp_lut_pos_at_level[_id]);
    //             // processor.add_in_net_from(_id, _inst_name_vec[_id], inst_level[_id], inst_pos_at_level[_id], _inst_name_vec[i]);
    //             //(processor.instances)[_inst_name_vec[i]].in_net_from_id.push_back(out_net_from_id[cur_in]);
    //             //(((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_info.push_back(_inst_name_vec[out_net_from_id[cur_in]]);
    //             //(((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_level.push_back(inst_level[out_net_from_id[cur_in]]);
    //             //(((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_pos_at_level.push_back(inst_pos_at_level[out_net_from_id[cur_in]]);
    //         }
    //         else if (pinbitValues.find(cur_in) != pinbitValues.end()) {
    //             // regard it as a constant
    //             //(((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_id.push_back(-2);
    //             //(((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_info.push_back(cur_in);
    //             //(((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_level.push_back(-2);
    //             TimedValues *tvs = pinbitValues[cur_in];
    //             int cur_val = (*(tvs->begin())).value;
    //             //(((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_pos_at_level.push_back(cur_val);
    //             // processor.add_in_net_from(-2, cur_in, -2, cur_val, _inst_name_vec[i]);

    //             // at this case, 'in net from pos at level' means value
    //             luts[i].in_net_from_id.push_back(-2);
    //             luts[i].in_net_from_info.push_back(cur_in);
    //             luts[i].in_net_from_level.push_back(-2);
    //             luts[i].in_net_from_pos_at_level.push_back(cur_val);
    //         }
    //         else if (assign_pairs.find(cur_in) != assign_pairs.end()) {
    //             string temp = assign_pairs[cur_in];
    //             if (net_from_id.find(temp) != net_from_id.end()) {
    //                 edges += 1;
    //                 int _id = net_from_id[temp];
    //                 // cout << "id: " << _id << endl;
    //                 tmp_adjncys[luts[i].num].push_back(_id);
    //                 tmp_adjncys[_id].push_back(luts[i].num);
    //                 // processor.add_in_net_from(_id, _inst_name_vec[_id], inst_level[_id], inst_pos_at_level[_id], _inst_name_vec[i]);
    //                 luts[i].in_net_from_id.push_back(_id);
    //                 luts[i].in_net_from_info.push_back(" ");
    //                 luts[i].in_net_from_level.push_back(tmp_lut_level[_id]);
    //                 luts[i].in_net_from_pos_at_level.push_back(tmp_lut_pos_at_level[_id]);
    //             }
    //             else if (pinbitValues.find(temp) != pinbitValues.end()) {     
    //                 TimedValues *tvs = pinbitValues[temp];
    //                 int cur_val = (*(tvs->begin())).value;
    //                 //(((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_pos_at_level.push_back(cur_val);
    //                 // processor.add_in_net_from(-2, temp, -2, cur_val, _inst_name_vec[i]);

    //                 // at this case, 'in net from pos at level' means value
    //                 luts[i].in_net_from_id.push_back(-2);
    //                 luts[i].in_net_from_info.push_back(temp);
    //                 luts[i].in_net_from_level.push_back(-2);
    //                 luts[i].in_net_from_pos_at_level.push_back(cur_val);
    //             }
    //             else {
    //                 /*(((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_id.push_back(-1);
    //                 (((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_info.push_back(temp);
    //                 (((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_level.push_back(-1);*/
    //                 if (initial_net_map.find(temp) == initial_net_map.end()) {
    //                     initial_net_map[temp] = initial_id;
    //                     initial_id++;
    //                 }
    //                 //(((processor.instances).find(_inst_name_vec[i]))->second).in_net_from_pos_at_level.push_back(initial_net_map[temp]);

    //                 // processor.add_in_net_from(-1, temp, -1, initial_net_map[temp], _inst_name_vec[i]);

    //                 // at this case, 'in net from pos at level' means initial id
    //                 luts[i].in_net_from_id.push_back(-1);
    //                 luts[i].in_net_from_info.push_back(temp);
    //                 luts[i].in_net_from_level.push_back(-1);
    //                 luts[i].in_net_from_pos_at_level.push_back(initial_net_map[temp]);
    //             }
    //         }
    //         else {
    //             if (initial_net_map.find(cur_in) == initial_net_map.end()) {
    //                 initial_net_map[cur_in] = initial_id;
    //                 initial_id++;
    //             }
    //             // processor.add_in_net_from(-1, cur_in, -1, initial_net_map[cur_in], _inst_name_vec[i]);

    //             // at this case, 'in net from pos at level' means initial id
    //             luts[i].in_net_from_id.push_back(-1);
    //             luts[i].in_net_from_info.push_back(cur_in);
    //             luts[i].in_net_from_level.push_back(-1);
    //             luts[i].in_net_from_pos_at_level.push_back(initial_net_map[cur_in]);
    //         }
    //     }
    // }
    // debug
    // cout << "edges: " << edges << endl;
    // cout << "net from id: " << endl;
    // for (map<string, int>::iterator i = net_from_id.begin(); i != net_from_id.end(); i++) {
    //     cout << i->first << " " << i->second;
    //     cout << endl;
    // }
    // cout << "adjncys :" << endl;
    // for (int i = 0; i < tmp_adjncys.size(); i++) {
    //     cout << i << "    ";
    //     for (int j = 0; j < tmp_adjncys[i].size(); j++) {    
    //         cout << tmp_adjncys[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    edges = 0;
    vector<vector<int>> tmp_adjncys(luts.size());

    for (unsigned i = 0; i < luts.size(); ++i)
    {
        LutType cur_lut = luts[i];
        std::vector<string> _in_net = cur_lut.in_ports;
        for (unsigned j = 0; j < _in_net.size(); ++j)
        {
            string cur_in = _in_net[j];
            if (net_from_id.find(cur_in) != net_from_id.end())
            {
                edges += 1;
                int _id = net_from_id[cur_in];
                // cout << "id: " << _id << endl;
                tmp_adjncys[luts[i].num].push_back(_id);
                tmp_adjncys[_id].push_back(luts[i].num);
            }
            else if (assign_pairs.find(cur_in) != assign_pairs.end())
            {
                string temp = assign_pairs[cur_in];
                if (net_from_id.find(temp) != net_from_id.end())
                {
                    edges += 1;
                    int _id = net_from_id[temp];
                    // cout << "id: " << _id << endl;
                    tmp_adjncys[luts[i].num].push_back(_id);
                    tmp_adjncys[_id].push_back(luts[i].num);
                }
            }
        }
    }

    // in net of each level lut from which levels 
    // std::vector<std::vector<int>> in_net_from_levels(levels.size());
    // std::vector<std::vector<int>> in_net_from_levels_num(levels.size());
    // for (unsigned i = 0; i < levels.size(); ++i) {
    //     std::vector<int> cur_level = levels[i];
    //     for (unsigned j = 0; j < cur_level.size(); ++j) {
    //         int cur_id = cur_level[j];
    //         LutType cur_lut = luts[cur_id];
    //         std::vector<int> _in_net_from_level = cur_lut.in_net_from_level;
    //         for (unsigned k = 0; k < _in_net_from_level.size(); ++k) {
    //             int tmp = _in_net_from_level[k];
    //             if (find(in_net_from_levels[i].begin(), in_net_from_levels[i].end(), tmp) == in_net_from_levels[i].end()) {
    //                 in_net_from_levels[i].push_back(tmp);
    //                 in_net_from_levels_num[i].push_back(1);
    //             }
    //             else {
    //                 auto it = find(in_net_from_levels[i].begin(), in_net_from_levels[i].end(), tmp);
    //                 int pos = distance(in_net_from_levels[i].begin(), it);
    //                 in_net_from_levels_num[i][pos] += 1;
    //             }
    //         }
    //     }
    // }
    // debug
    // for (unsigned i = 0; i < levels.size(); ++i)
    // {
    //     cout << "level: " << i << endl;
    //     std::vector<int> cur = in_net_from_levels[i];
    //     std::vector<int> cur_num = in_net_from_levels_num[i];
    //     cout << "\tlevel:";
    //     for (unsigned j = 0; j < cur.size(); ++j) {
    //         cout << cur[j] << " ";
    //     }
    //     cout << endl;
    //     cout << "\nnum:";
    //     for (unsigned j = 0; j < cur_num.size(); ++j) {
    //         cout << cur_num[j] << " ";
    //     }
    //     cout << endl;
    // }


    // pre process for mapping to boolean processor
    // int _size = luts.size();
    // int total_size = levels.size();

    // std::vector<unsigned int> tmp_data_in_num_start(total_size);  // lut's starting id of each level
    // std::vector<unsigned int> tmp_data_out_num_start(total_size); // lut's starting id of each level
    // // std::vector<unsigned int> delay_start(total_size);
    // // std::vector<unsigned int> delay_width(total_size);
    // std::vector<unsigned int> tmp_functions_start(total_size);    // lut's starting id of each level
    // // std::vector<unsigned int> functions_width(total_size);

    // std::vector<short> tmp_data_in_num(_size);          // in ports num of each lut
    // std::vector<short> tmp_data_out_num(_size);         // out ports num of each lut
    // std::vector<unsigned int> tmp_val_num_start(_size); // input val starting id of each lut
    // // std::vector<short> delay_val_num(_size);
    // // std::vector<short> functions_func_num(_size);   // func num of each inst
    // // std::vector<short> functions_val_num(_size);    // func val num of each inst

    // int _sum = 0;
    // // int _delay_start = 0;
    // // int _functions_start = 0;

    // for (unsigned i = 0; i < levels.size(); ++i)
    // {
    //     std::vector<int> cur_level = levels[i];
    //     const int lut_num = cur_level.size();
    //     int val_num_width = 0;
    //     // int delay_width_ = 0;
    //     // int functions_width_ = 0;
    //     tmp_data_in_num_start[i] = _sum;
    //     tmp_data_out_num_start[i] = _sum;
    //     // functions_start[i] = _functions_start;
    //     tmp_functions_start[i] = _sum;
    //     // delay_start[i] = _delay_start;
    //     for (int j = 0; j < lut_num; ++j)
    //     {
    //         LutType cur_lut = luts[cur_level[j]];
    //         std::vector<string> cur_in = cur_lut.in_ports;
    //         tmp_data_in_num[j + _sum] = cur_in.size();
    //         std::string cur_out = cur_lut.out_ports;
    //         tmp_data_out_num[j + _sum] = cur_out.size();

    //         tmp_val_num_start[j + _sum] = val_num_width;
    //         unsigned int _val_num_width = cur_in.size();
    //         val_num_width += _val_num_width;

    //         // std::vector<Delay> delays = cur_inst.get_delay();
    //         // int _delay_width = delays.size();
    //         // delay_val_num[j + _sum] = _delay_width;
    //         // delay_width_ = max(_delay_width, delay_width_);

    //         // std::vector<std::vector<int>> functions = cur_inst.function_id_vec;
    //         // int func_height = functions.size();
    //         // int func_width = 0;
    //         // for (unsigned jjj = 0; jjj < functions.size(); ++jjj)
    //         // {
    //         //     int _func_width = functions[jjj].size();
    //         //     func_width = max(_func_width, func_width);
    //         // }
    //         // functions_width_ = max(func_width * func_height, functions_width_);
    //         // functions_func_num[j + _sum] = func_height;
    //         // functions_val_num[j + _sum] = func_width;

            
    //     }
    //     _sum += lut_num;
    //     // _delay_start += inst_num * delay_width_;
    //     // _functions_start += lut_num * functions_width_;
    //     // delay_width[i] = delay_width_;
    //     // functions_width[i] = functions_width_;
    // }
    // for (int i = 0; i < data_in_num_start.size(); ++i)
    //{
    //     cout << data_out_num_start[i] << endl;
    // }

    // std::vector<short> host_delay_edges(_delay_start);
    // std::vector<short> host_in_bit(_delay_start);
    // std::vector<short> host_out_bit(_delay_start);
    // std::vector<float> host_rise_val(_delay_start);
    // std::vector<float> host_fall_val(_delay_start);

    // std::vector<short> host_functions(_functions_start);

    // for (unsigned i = 0; i < levels.size(); ++i)
    // {
    //     std::vector<int> cur_level = levels[i];
    //     const int inst_num = cur_level.size();

    //     int delay_start_ = delay_start[i];
    //     int delay_width_ = delay_width[i];
    //     int functions_width_ = functions_width[i];
    //     int functions_start_ = functions_start[i];
    //     for (int j = 0; j < inst_num; ++j)
    //     {
    //         Instance cur_inst = instances[_inst_name_vec[cur_level[j]]];

    //         std::vector<Delay> delays = cur_inst.get_delay();
    //         std::vector<string> cur_in = cur_inst.in_net;
    //         std::vector<string> cur_out = cur_inst.out_net;
    //         for (unsigned it = 0; it < delays.size(); ++it)
    //         {
    //             host_delay_edges[delay_start_ + j * delay_width_ + it] = delays[it].edge;
    //             auto pos = find(cur_in.begin(), cur_in.end(), delays[it].in_bit);
    //             host_in_bit[delay_start_ + j * delay_width_ + it] = distance(cur_in.begin(), pos);
    //             pos = find(cur_out.begin(), cur_out.end(), delays[it].out_bit);
    //             host_out_bit[delay_start_ + j * delay_width_ + it] = distance(cur_out.begin(), pos);
    //             host_rise_val[delay_start_ + j * delay_width_ + it] = delays[it].rise_val;
    //             host_fall_val[delay_start_ + j * delay_width_ + it] = delays[it].fall_val;
    //         }
    //         if (delays.size() < delay_width_)
    //         {
    //             for (int ttt = delays.size(); ttt < delay_width_; ++ttt)
    //             {
    //                 host_delay_edges[delay_start_ + j * delay_width_ + ttt] = -1;
    //                 host_in_bit[delay_start_ + j * delay_width_ + ttt] = -1;
    //                 host_out_bit[delay_start_ + j * delay_width_ + ttt] = -1;
    //                 host_rise_val[delay_start_ + j * delay_width_ + ttt] = -1.0;
    //                 host_fall_val[delay_start_ + j * delay_width_ + ttt] = -1.0;
    //             }
    //         }

    //         std::vector<std::vector<int>> functions = cur_inst.function_id_vec;
    //         int func_height = functions.size();
    //         int func_width = functions_width_ / func_height;
    //         for (int iii = 0; iii < func_height; ++iii)
    //         {
    //             for (int kkk = 0; kkk < func_width; ++kkk)
    //             {
    //                 if (kkk >= functions[iii].size())
    //                     host_functions[functions_start_ + j * functions_width_ + iii * func_width + kkk] = -1;
    //                 else
    //                 {
    //                     host_functions[functions_start_ + j * functions_width_ + iii * func_width + kkk] = functions[iii][kkk];
    //                 }
    //             }
    //         }
    //         if (func_width * func_height < functions_width_)
    //         {
    //             for (int ttt = func_width * func_height; ttt < functions_width_; ++ttt)
    //             {
    //                 host_functions[functions_start_ + j * functions_width_ + ttt] = -1;
    //             }
    //         }
    //     }
    // }

    // debug
    // for(unsigned i = 0; i < levels.size(); i++)
    // {
    //     cout << "level: " << i << "(size:" << levels[i].size() << ")" << endl;
    //     cout << "\t";
    //     for (unsigned j = 0; j < (levels[i]).size(); ++j)
    //     {
    //         cout << levels[i][j] << "(" << luts[levels[i][j]] << "), ";
    //     }
    //     cout << endl;
    // }

    /** output intermediate file **/
    // inter _inter;
    // _inter.p = _parser;
    // _inter.levels = levels;    
    // _inter.initial_net_map = initial_net_map;                     
    // _inter.data_in_num_start = data_in_num_start;   
    // _inter.data_out_num_start = data_out_num_start; 
    // // _inter.delay_start = delay_start;               
    // // _inter.delay_width = delay_width; 
    // _inter.functions_start = functions_start; 
    // _inter.data_in_num = data_in_num;               
    // _inter.data_out_num = data_out_num;             
    // _inter.val_num_start = val_num_start;           
    // _inter.delay_val_num = delay_val_num;           
    // _inter.functions_func_num = functions_func_num; 
    // _inter.functions_val_num = functions_val_num;   
    // _inter.host_delay_edges = host_delay_edges;  // (0:pos, 1:neg; 2:both)    
    // _inter.host_in_bit = host_in_bit;       
    // _inter.host_out_bit = host_out_bit;     
    // _inter.host_rise_val = host_rise_val;   
    // _inter.host_fall_val = host_fall_val;   
    // _inter.host_functions = host_functions; 
    // template <class Instance>
    // ofstream ofs(inter_file.c_str());
    // boost::archive::text_oarchive oa(ofs);
    // oa & _inter;
    // cout << "store database into " << inter_file << endl;
    // ofs.close();

    auto end_pre = std::chrono::steady_clock::now();
    long duration_pre = std::chrono::duration_cast<std::chrono::milliseconds>(end_pre - start_pre).count();
    cout << "Successfully finished topology ordering. (" << duration_pre << "ms)" << endl;
    cout << endl;
    
    // data_in_num_start = tmp_data_in_num_start;
    // data_out_num_start = tmp_data_out_num_start;
    // functions_start = tmp_functions_start;
    // data_in_num = tmp_data_in_num;
    // data_out_num = tmp_data_out_num;
    // val_num_start = tmp_val_num_start;
    adjncys = tmp_adjncys;
    lut_level = tmp_lut_level;
    lut_pos_at_level = tmp_lut_pos_at_level;

}