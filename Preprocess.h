#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "Parser.h"

#ifndef Preprocess_H
#define Preprocess_H

using namespace std;

/**
 * @brief generate intermediate parm via pre-processing
 **/
class inter {
public:
    //! Instance a new Pre-processing Class
    inter();

    //! Destructor
    ~inter();

    void GenAdjList(map<int, LutType> &luts, map<string, vector<int>> &net_for_id, /*map<string, int> &net_from_id,*/
                    map<int, vector<int>> &adjList /*, vector<int> &edgesSize, vector<int> &edgesOffset*/);
    void SetInDegrees(map<int, vector<int>> &adjList, vector<int> &indegrees);
    void TopologyOrder(map<int, vector<int>> &adjList, vector<int> &indegrees, int &n,
                       vector<vector<int>> &levels);

    void preprocess(string &v_path);

    /* data */
    Parser _parser;
    std::vector<std::vector<int>> levels;
    // std::map<std::string, int> initial_net_map; // net : inintial id

    // std::vector<unsigned int> data_in_num_start;
    // std::vector<unsigned int> data_out_num_start;
    // std::vector<unsigned int> delay_start;
    // std::vector<unsigned int> delay_width;
    // std::vector<unsigned int> functions_start;
    // std::vector<unsigned int> functions_width;

    // std::vector<short> data_in_num;          // in ports num of each lut
    // std::vector<short> data_out_num;         // out ports num of each lut
    // std::vector<unsigned int> val_num_start; // input val starting id of each lut
    int edges;
    std::vector<std::vector<int>> adjncys;
    vector<int> lut_level;
    vector<int> lut_pos_at_level;
    // std::vector<short> delay_val_num;
    // std::vector<short> functions_func_num;
    // std::vector<short> functions_val_num;
    // std::vector<short> host_delay_edges;
    // std::vector<short> host_in_bit;
    // std::vector<short> host_out_bit;
    // std::vector<float> host_rise_val;
    // std::vector<float> host_fall_val;

    // std::vector<short> host_functions;

};

#endif