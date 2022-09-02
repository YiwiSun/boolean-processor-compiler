#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <algorithm>
#include <limits>
#include <cstddef>
#include <bitset>
#include <bits/stdc++.h>
#include <cctype>
#include <metis.h>

// #include <boost/serialization/string.hpp>
// #include <boost/serialization/vector.hpp>
// #include <boost/serialization/map.hpp>
// #include <boost/serialization/deque.hpp>
// #include <boost/serialization/access.hpp>
// #include <boost/archive/text_iarchive.hpp>
// #include <boost/archive/text_oarchive.hpp>

#include "compile.h"
#include "Preprocess.h"
#include "VCDParser.h"
#include "Parser.h"
#include "VCDTypes.h"
// #include "Function.h"

// #define N_THREADS_PER_BLOCK 512
// #define N_TIMES_PER_THREAD 16
// #define MIN_THREAD_NUM 4
// #define MAX_BLOCKS 65535

// #define MAX_CLUSTERS 48
// #define N_PROCESSORS_PER_CLUSTER 64
// #define N_INS_PER_PROCESSOR 512

// #define SIM_START 10000
// #define SIM_START 0
// #define SIM_END 100000001
// #define SIM_END 20000001
// #define SIM_END 2539945010
// #define SIM_END 2972036001
// #define MAX_NUM 100000001

using namespace std;



// class inter
// {
// public:
//     inter() {}
//     ~inter() {}

//     // Instance *find_inst(int id)
//     // {
//     //     return p.find_inst(id);
//     // }

//     /* data */
//     Parser p;
//     std::vector<std::vector<int>> levels;
//     std::map<std::string, int> initial_net_map;

//     std::vector<unsigned int> data_in_num_start;
//     std::vector<unsigned int> data_out_num_start;
//     // std::vector<unsigned int> delay_start;
//     // std::vector<unsigned int> delay_width;
//     std::vector<unsigned int> functions_start;
//     // std::vector<unsigned int> functions_width;

//     std::vector<short> data_in_num;
//     std::vector<short> data_out_num;
//     std::vector<unsigned int> val_num_start;
//     // std::vector<short> delay_val_num;
//     // std::vector<short> functions_func_num;
//     // std::vector<short> functions_val_num;
//     // std::vector<short> host_delay_edges;
//     // std::vector<short> host_in_bit;
//     // std::vector<short> host_out_bit;
//     // std::vector<float> host_rise_val;
//     // std::vector<float> host_fall_val;

//     // std::vector<short> host_functions;

//     template <class Archive>
//     void serialize(Archive &ar, const unsigned int version) //
//     {                                                       // ar.register_type(static_cast<ModuleType *>(NULL));
//         ar & p;
//         ar & levels;
//         ar & initial_net_map;
//         ar & data_in_num_start;
//         ar & data_out_num_start;
//         // ar & delay_start;
//         // ar & delay_width;
//         ar & functions_start;
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




int main(int argc, char const *argv[])
{
    std::string v_path, vcdFilePath, instr_out, out_dir; /*database_path, saif_out, dumpon_time_str, dumpoff_time_str*/
    // int dumpon_time, dumpoff_time;
    if (argc == 5)
    {
        // database_path = argv[1];
        v_path = argv[1];
        vcdFilePath = argv[2]; 
        instr_out = argv[3];
        out_dir = argv[4];
        // dumpon_time_str = argv[3];
        // dumpoff_time_str = argv[4];
        // saif_out = argv[5];
    }
    else
    {
        // cout << "[USAGE] ./compile [intermediate_file] vcd_path dumpon_time(ps) dumpoff_time(ps)" << endl;
        cout << "[USAGE] ./compile v_path vcd_path [instr_path]" << endl;
        exit(-1);
    }

    auto start_total = std::chrono::steady_clock::now();

    inter _inter;
    _inter.preprocess(v_path);
    // ifstream ifs(database_path.c_str());
    // boost::archive::text_iarchive ia(ifs);
    // ia & _inter;
    // ifs.close();

    Parser _parser;
    std::vector<std::vector<int> > levels;
    // std::map<std::string, int> initial_net_map;

    // std::vector<unsigned int> data_in_num_start;
    // std::vector<unsigned int> data_out_num_start;
    // std::vector<unsigned int> delay_start;
    // std::vector<unsigned int> delay_width;
    // std::vector<unsigned int> functions_start;
    // std::vector<unsigned int> functions_width;

    // std::vector<short> data_in_num;
    // std::vector<short> data_out_num;
    // std::vector<unsigned int> val_num_start;
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

    _parser = _inter._parser;
    levels =  _inter.levels;
    // initial_net_map = _inter.initial_net_map;

    // data_in_num_start = _inter.data_in_num_start;
    // data_out_num_start = _inter.data_out_num_start;
    // delay_start = _inter.delay_start;
    // delay_width = _inter.delay_width;
    // functions_start = _inter.functions_start;
    // functions_width = _inter.functions_width;
    // data_in_num = _inter.data_in_num;
    // data_out_num = _inter.data_out_num;
    // val_num_start = _inter.val_num_start;
    edges = _inter.edges;
    adjncys = _inter.adjncys;
    lut_level = _inter.lut_level;
    lut_pos_at_level = _inter.lut_pos_at_level;
    // delay_val_num = _inter.delay_val_num;
    // functions_func_num = _inter.functions_func_num;
    // functions_val_num = _inter.functions_val_num;
    // host_delay_edges = _inter.host_delay_edges;
    // host_in_bit = _inter.host_in_bit;
    // host_out_bit = _inter.host_out_bit;
    // host_rise_val = _inter.host_rise_val;
    // host_fall_val = _inter.host_fall_val;
    // host_functions = _inter.host_functions;

    // debug
    // cout << "new:" << endl;
    // cout << _inter.levels.size() << endl;
    // cout << _inter._parser.luts.size() << endl;
    // cout << "levels: " << endl;
    // for (int i = 0; i < levels.size(); i++) {
    //     cout << i << "    ";
    //     for (int j = 0; j < levels[i].size(); j++) {
    //         cout << levels[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    std::map<int, LutType> luts = _inter._parser.get_luts();
    map<string, vector<int>> net_for_id = _inter._parser.net_for_id;
    map<string, int> net_from_id = _inter._parser.net_from_id;
    std::map<std::string, std::string> assign_pairs = _inter._parser.get_assign_pairs();
    std::map<std::string, TimedValues *> pinbitValues = _inter._parser.get_pinbitValues();
    map<string, string> pin_bits = _inter._parser.get_pin_bits();

    // partition param
    idx_t nVertices = _parser.luts.size();
    idx_t nEdges = edges;
    vector<vector<idx_t>> nAdjncys = adjncys;

    // pre-processing param
    std::map<std::string, int> initial_net_map;     // net : initial id



    /************************ graph partition ******************************/
    cout << "3. Executing graph partition." << endl;
    auto start_par = std::chrono::steady_clock::now();

    // the adjacency structure of the graph
    vector<idx_t> xadj(0);
    vector<idx_t> adjncy(0); 
    // vector<idx_t> adjwgt(0); // the weights of the edges

    for (int i = 0; i < nAdjncys.size(); i++)
    {
        xadj.push_back(adjncy.size()); // csr: row offsets (https://www.cnblogs.com/xbinworld/p/4273506.html)
        for (int j = 0; j < nAdjncys[i].size(); j++) {
            adjncy.push_back(nAdjncys[i][j]); 
        } 
        // adjwgt.push_back(w);	 // values
    }
    xadj.push_back(adjncy.size());
    assert(xadj.size() == (nVertices + 1));
    assert(adjncy.size() == (nEdges * 2));

    // debug
    // cout << vexnum << " " << edgenum << endl;
    // cout << "xadj size: " << xadj.size() << endl;
    // cout << "xadj: " << endl;
    // for (int i = 0; i < xadj.size(); i++)
    // {
    //     cout << xadj[i] << " ";
    // }
    // cout << endl;
    // cout << "adjncy: " << endl;
    // for (int i = 0; i < adjncy.size(); i++)
    // {
    //     cout << adjncy[i] << " ";
    // }
    // cout << endl;

    vector<idx_t> part = part_func(xadj, adjncy, /*adjwgt, */ METIS_PartGraphRecursive);
    // vector<idx_t> part = func(xadj, adjncy, adjwgt, METIS_PartGraphKway);

    // ofstream outpartition(par_out_path);
    // for (int i = 0; i < part.size(); i++)
    // {
    //     outpartition << i << " " << part[i] << endl;
    // }
    // outpartition.close();

    auto end_par = std::chrono::steady_clock::now();
    long duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par).count();
    cout << "Successfully finished graph partition. (" << duration_par << "ms)" << endl;
    cout << endl;



   /******************************* vcd parsing *******************************/
    cout << "4. Executing vcd file parsing." << endl;
    auto start_vcd = std::chrono::steady_clock::now();
    VCDParser _vcdparser;               // = new VCDParser();
    _vcdparser.parse(vcdFilePath);      //, initial_net_map))
    auto end_vcd = std::chrono::steady_clock::now();
    long duration_vcd = std::chrono::duration_cast<std::chrono::milliseconds>(end_vcd - start_vcd).count();
    cout << "Successfully finished vcd file parsing. (" << duration_vcd << "ms)" << endl;
    cout << endl;
   


    /**************************** pre processing ******************************/
    cout << "5. Executing pre-processing." << endl;
    auto start_pre = std::chrono::steady_clock::now();

    int parts = (_parser.luts.size() + N_PROCESSORS_PER_CLUSTER - 1) / N_PROCESSORS_PER_CLUSTER;    // part nums
    map<int, vector<int>> luts_in_part;     // partition : luts
    if (parts == 1 && part[0] == 1) 
    {   
        for (int i = 0; i < part.size(); i++)
        {
            // luts_in_part[0].push_back(i);
            part[i] -= 1;
        }
    }
    for (int i = 0; i < parts; i++)
    {
        for (int j = 0; j < part.size(); j++)
        {
            if (part[j] == i)
            {
                luts_in_part[i].push_back(j);
            }
        }
    }
    // debug
    // for (map<int, vector<int>>::iterator i = luts_in_part.begin(); i != luts_in_part.end(); i++) {
    //     cout << i->first << "    ";
    //     for (vector<int>::iterator j = i->second.begin(); j != i->second.end(); j++) {
    //         cout << *j << " ";
    //     }
    //     cout << endl;
    // }

    map<int, vector<vector<int>>> parts_levels;     // partition : levels
    for (int i = 0; i < parts; i++) {
        vector<int> cur_luts = luts_in_part[i];
        vector<vector<int>> cur_part_levels(levels.size());
        for (int j = 0; j < cur_luts.size(); j++) {
            int cur_level = lut_level[cur_luts[j]];
            cur_part_levels[cur_level].push_back(cur_luts[j]);
        }
        for (int l = 0; l < cur_part_levels.size(); ) {
            if (cur_part_levels[l].empty()) {
                cur_part_levels.erase(cur_part_levels.begin() + l);
            }
            else
            {
                l++;
            }
        } 
        parts_levels[i] = cur_part_levels;
    }
    // debug
    // for (int i = 0; i < parts; i++) {
    //     vector<vector<int>> cur_part_levels = parts_levels[i];
    //     cout << "parts " << i << ":" << endl;
    //     for (int l = 0; l < cur_part_levels.size(); l++) {
    //         cout << "level " << l << ":    ";
    //         for (int j = 0; j < cur_part_levels[l].size(); j++) {
    //             cout << cur_part_levels[l][j] << " ";
    //         }
    //         cout << endl;
    //     }
    // }

    map<int, int> lut_level_in_part;
    map<int, int> lut_pos_at_level_in_part;
    for (int p = 0; p < parts; p++) {
        vector<vector<int>> cur_part_levels = parts_levels[p];
        for (unsigned i = 0; i < cur_part_levels.size(); i++)
        {
            vector<int> cur_level = cur_part_levels[i];
            for (unsigned j = 0; j < cur_level.size(); j++)
            {
                int cur_id = cur_level[j];
                lut_level_in_part[cur_id] = i;
                lut_pos_at_level_in_part[cur_id] = j;
            }
        }
    }

    // generate 'net value from' info
    int initial_id = 0;
    for (int p = 0; p < parts; p++) {
        vector<int> cur_luts = luts_in_part[p];
        // vector<vector<int>> cur_part_levels = parts_levels[p];      
        for (int i = 0; i < cur_luts.size(); i++) {
            int lut_num = cur_luts[i];
            LutType cur_lut = luts[lut_num];
            std::vector<string> _in_net = cur_lut.in_ports;
            for (unsigned j = 0; j < _in_net.size(); ++j)
            {
                string cur_in = _in_net[j];
                if (net_from_id.find(cur_in) != net_from_id.end())
                {
                    int _id = net_from_id[cur_in];
                    luts[lut_num].in_net_from_id.push_back(_id);
                    luts[lut_num].in_net_from_info.push_back(" ");
                    if (part[_id] == p) {
                        luts[lut_num].in_net_from_part.push_back(-3);
                        luts[lut_num].in_net_from_level.push_back(lut_level_in_part[_id]);
                        luts[lut_num].in_net_from_pos_at_level.push_back(lut_pos_at_level_in_part[_id]);
                    }
                    else {
                        luts[lut_num].in_net_from_part.push_back(part[_id]);
                        luts[lut_num].in_net_from_level.push_back(lut_level_in_part[_id]);
                        luts[lut_num].in_net_from_pos_at_level.push_back(lut_pos_at_level_in_part[_id]);
                    }
                }
                else if (pinbitValues.find(cur_in) != pinbitValues.end())
                {
                    TimedValues *tvs = pinbitValues[cur_in];
                    int cur_val = (*(tvs->begin())).value;
                    // at this case, 'in net from pos at level' means value
                    luts[lut_num].in_net_from_id.push_back(-2);
                    luts[lut_num].in_net_from_info.push_back(cur_in);
                    luts[lut_num].in_net_from_part.push_back(-2);
                    luts[lut_num].in_net_from_level.push_back(-2);
                    luts[lut_num].in_net_from_pos_at_level.push_back(cur_val);
                }
                else if (assign_pairs.find(cur_in) != assign_pairs.end())
                {
                    string temp = assign_pairs[cur_in];
                    if (net_from_id.find(temp) != net_from_id.end())
                    {
                        int _id = net_from_id[temp];
                        luts[lut_num].in_net_from_id.push_back(_id);
                        luts[lut_num].in_net_from_info.push_back(" ");
                        if (part[_id] == p)
                        {
                            luts[lut_num].in_net_from_part.push_back(-3);
                            luts[lut_num].in_net_from_level.push_back(lut_level_in_part[_id]);
                            luts[lut_num].in_net_from_pos_at_level.push_back(lut_pos_at_level_in_part[_id]);
                        }
                        else
                        {
                            luts[lut_num].in_net_from_part.push_back(part[_id]);
                            luts[lut_num].in_net_from_level.push_back(lut_level_in_part[_id]);
                            luts[lut_num].in_net_from_pos_at_level.push_back(lut_pos_at_level_in_part[_id]);
                        }
                    }
                    else if (pinbitValues.find(temp) != pinbitValues.end())
                    {
                        TimedValues *tvs = pinbitValues[temp];
                        int cur_val = (*(tvs->begin())).value;
                        // at this case, 'in net from pos at level' means value
                        luts[lut_num].in_net_from_id.push_back(-2);
                        luts[lut_num].in_net_from_info.push_back(temp);
                        luts[lut_num].in_net_from_part.push_back(-2);
                        luts[lut_num].in_net_from_level.push_back(-2);
                        luts[lut_num].in_net_from_pos_at_level.push_back(cur_val);
                    }
                    else
                    {
                        if (initial_net_map.find(temp) == initial_net_map.end())
                        {
                            initial_net_map[temp] = initial_id;
                            initial_id++;
                        }
                        // at this case, 'in net from pos at level' means initial id
                        luts[lut_num].in_net_from_id.push_back(-1);
                        luts[lut_num].in_net_from_info.push_back(temp);
                        luts[lut_num].in_net_from_part.push_back(-1);
                        luts[lut_num].in_net_from_level.push_back(-1);
                        luts[lut_num].in_net_from_pos_at_level.push_back(initial_net_map[temp]);
                    }
                }
                else
                {
                    if (initial_net_map.find(cur_in) == initial_net_map.end())
                    {
                        initial_net_map[cur_in] = initial_id;
                        initial_id++;
                    }
                    // at this case, 'in net from pos at level' means initial id
                    luts[lut_num].in_net_from_id.push_back(-1);
                    luts[lut_num].in_net_from_info.push_back(cur_in);
                    luts[lut_num].in_net_from_part.push_back(-1);
                    luts[lut_num].in_net_from_level.push_back(-1);
                    luts[lut_num].in_net_from_pos_at_level.push_back(initial_net_map[cur_in]);
                }
            }
        }
    }

    VCDTimeUnit time_unit = _vcdparser.time_units;
    unsigned time_res = _vcdparser.time_resolution;
    VCDScope *root = _vcdparser.root_scope;
    string root_name = root->name;
    std::unordered_map<std::string, std::vector<unsigned int> *> vcd_times = _vcdparser.times;
    std::unordered_map<std::string, std::vector<short> *> vcd_values = _vcdparser.values;

    auto end_pre = std::chrono::steady_clock::now();
    long duration_pre = std::chrono::duration_cast<std::chrono::milliseconds>(end_pre - start_pre).count();
    cout << "Successfully finished pre-processing. (" << duration_pre << "ms)" << endl;
    cout << endl;

    // debug
    // for (int i = 0; i < luts.size(); i++) {
    //     cout << i << ": " << endl;
    //     cout << "in_net_from_id: ";
    //     for (int j = 0; j < luts[i].in_net_from_id.size(); j++) {
    //         cout << luts[i].in_net_from_id[j] << " ";
    //     }
    //     cout << endl;
    //     cout << "in_net_from_info: ";
    //     for (int j = 0; j < luts[i].in_net_from_info.size(); j++)
    //     {
    //         cout << luts[i].in_net_from_info[j] << " ";
    //     }
    //     cout << endl;
    //     cout << "in_net_from_part: ";
    //     for (int j = 0; j < luts[i].in_net_from_part.size(); j++)
    //     {
    //         cout << luts[i].in_net_from_part[j] << " ";
    //     }
    //     cout << endl;
    //     cout << "in_net_from_level: ";
    //     for (int j = 0; j < luts[i].in_net_from_level.size(); j++)
    //     {
    //         cout << luts[i].in_net_from_level[j] << " ";
    //     }
    //     cout << endl;
    //     cout << "in_net_from_pos_at_level: ";
    //     for (int j = 0; j < luts[i].in_net_from_pos_at_level.size(); j++)
    //     {
    //         cout << luts[i].in_net_from_pos_at_level[j] << " ";
    //     }
    //     cout << endl;
    // }
     


    /**************************** instruction generation *******************************/
    // int cluster_num = 0;
    // int processor_num = 0;
    // for (int p = 0; p < parts; p++) {
    //     vector<vector<int>> cur_part_levels = parts_levels[p];
    //     vector<int> cur_luts = luts_in_part[p];
    //     for (int l = 0; l < cur_part_levels.size(); l++) {
    //         vector<int> cur_level = cur_part_levels[l];
    //         for (int i = 0; i < cur_level.size(); i++) {
    //             int lut_num = cur_level[i];
    //             LutType cur_lut = luts[lut_num];
    //             cur_lut.node_addr = {cluster_num, processor_num};
    //             string lut_instr;
    //             vector<string> lut_instrs;
    //             Processor cur_processor;
    //             int in_num = 0;
    //             int cal = 0;
    //             string lut_input_select;
    //             for (int in = 0; in < cur_lut.in_ports.size(); in++) {
    //                 int in_net_from_id = cur_lut.in_net_from_id[in]; // in net from out net from luts; -1: in net from initial module; -2: in net from assign pin bit
    //                 string in_net_from_info = cur_lut.in_net_from_info[in];
    //                 int in_net_from_part = cur_lut.in_net_from_part[in]; // -1: in net from initial module; -2: in net from assign pin bit; -3: current part; other: part num
    //                 int in_net_from_level = cur_lut.in_net_from_level[in];
    //                 int in_net_from_pos_at_level = cur_lut.in_net_from_pos_at_level[in];
    //                 // in net from initial module
    //                 if (in_net_from_id == -1) {
    //                     // instr
    //                     // cur_processor.instr_mem.push_back();
    //                     // cur_processor.inter_mem.push_back();
    //                     in_num += 1;
    //                     lut_input_select.append("0");
    //                 }
    //                 // in net from assign pin bit
    //                 else if (in_net_from_id == -2) {
    //                     // instr
    //                     // cur_processor.instr_mem.push_back();
    //                     // cur_processor.inter_mem.push_back();
    //                     in_num += 1;
    //                     lut_input_select.append("0");
    //                 }
    //                 // in net from out net from luts
    //                 else {
    //                     if (in_net_from_part == -3) {
    //                         if (in_num == (cur_lut.in_ports.size() - 1)) {
    //                             Instr_1 instr_1;
    //                             instr_1.LUT_Value = cur_lut.lut_res;
    //                             instr_1.Node_Addr = luts[in_net_from_id].node_addr;
    //                         }
    //                         else {
    //                             Instr_4 instr_4;
    //                             instr_4.Node_Addr = luts[in_net_from_id].node_addr;
    //                         }
    //                     }
    //                     // in net from lut of other part
    //                     else {
    //                     }
    //                 }
    //             }
    //             processors[N_PROCESSORS_PER_CLUSTER * cluster_num + processor_num] = cur_processor;
    //             if (processor_num == N_PROCESSORS_PER_CLUSTER || processor_num == cur_luts.size())
    //             {
    //                 cluster_num += 1;
    //                 processor_num = 0;
    //             }
    //             else {
    //                 processor_num += 1;
    //             }
    //         }
    //     }
    // }
    cout << "6. Executing instruction generation." << endl;
    auto start_ins = std::chrono::steady_clock::now();

    int cluster_num = 0;
    int processor_num = 0;
    unsigned int current_time = 0;
    for (int p = 0; p < parts; p++)
    {
        vector<vector<int>> cur_part_levels = parts_levels[p];
        vector<int> cur_luts = luts_in_part[p];
        for (int l = 0; l < cur_part_levels.size(); l++)
        {
            vector<int> cur_level = cur_part_levels[l];
            for (int i = 0; i < cur_level.size(); i++)
            {
                int lut_num = cur_level[i];      
                luts[lut_num].node_addr = {cluster_num, processor_num};
                LutType cur_lut = luts[lut_num];
                // string lut_instr;
                vector<string> lut_instrs;
                Processor cur_processor;
                // int in_num = 0;
                // int cal = 0;
                int initial_num = count(cur_lut.in_net_from_id.begin(), cur_lut.in_net_from_id.end(), -1) + count(cur_lut.in_net_from_id.begin(), cur_lut.in_net_from_id.end(), -2); // num of inputs from initial module or assign pin bit
                // all input from initial module or assign pin bit
                if (initial_num == cur_lut.in_ports.size()) 
                {
                    string Data_Mem_Select;
                    Instr_1 instr_1;
                    instr_1.LUT_Value = cur_lut.lut_res;
                    instr_1.Node_Addr = cur_lut.node_addr;
                    for (int in = 0; in < cur_lut.in_ports.size(); in++) 
                    {
                        if (cur_lut.in_net_from_id[in] == -1)   // -1: in net from initial module
                        {     
                            // Data_Mem_Select
                            if (vcd_values.find(cur_lut.in_net_from_info[in]) != vcd_values.end())
                            {
                                vector<unsigned int> *tvs = vcd_times[cur_lut.in_net_from_info[in]];
                                vector<short> *tvs_val = vcd_values[cur_lut.in_net_from_info[in]];
                                vector<unsigned int>::iterator it = find(tvs->begin(), tvs->end(), current_time);
                                auto id = distance(tvs->begin(), it);
                                if (*(tvs_val->begin() + id) == 0)
                                    Data_Mem_Select.append("0");
                                else 
                                    Data_Mem_Select.append("1");
                            }
                            else
                            {
                                cout << "ERROR: No initial info of signal " << cur_lut.in_net_from_info[in] << " in vcd file!" << endl;
                            }
                        }
                        else if (cur_lut.in_net_from_id[in] == -2)  // -2: in net from assign pin bit
                        {    
                            if (cur_lut.in_net_from_pos_at_level[in] == 0) 
                                Data_Mem_Select.append("0");
                            else 
                                Data_Mem_Select.append("1");
                        }
                    }
                    instr_1.Data_Mem_Select = Data_Mem_Select;
                    instr_1.Operand_Addr = {MEM_DEPTH - 1, MEM_DEPTH - 1, MEM_DEPTH - 1, MEM_DEPTH - 1};
                    string lut_instr = cat_instr_1(instr_1);
                    lut_instrs.push_back(lut_instr);
                    cur_processor.instr_mem = lut_instrs;
                    luts[lut_num].res_pos_at_mem = 0;
                }
                // just one input from out net of other luts
                else if (initial_num == cur_lut.in_ports.size() - 1) 
                {
                    string Data_Mem_Select;
                    Instr_1 instr_1;
                    Instr_4 instr_4;
                    instr_1.LUT_Value = cur_lut.lut_res;
                    instr_4.Node_Addr = cur_lut.node_addr;
                    int instr_4_num;
                    for (int in = 0; in < cur_lut.in_ports.size(); in++)
                    {
                        if (cur_lut.in_net_from_id[in] == -1)   // -1: in net from initial module
                        {
                            // Data_Mem_Select
                            if (vcd_values.find(cur_lut.in_net_from_info[in]) != vcd_values.end())
                            {
                                vector<unsigned int> *tvs = vcd_times[cur_lut.in_net_from_info[in]];
                                vector<short> *tvs_val = vcd_values[cur_lut.in_net_from_info[in]];
                                vector<unsigned int>::iterator it = find(tvs->begin(), tvs->end(), current_time);
                                auto id = distance(tvs->begin(), it);
                                if (*(tvs_val->begin() + id) == 0)
                                    Data_Mem_Select.append("0");
                                else
                                    Data_Mem_Select.append("1");
                            }
                            else
                            {
                                cout << "ERROR: No initial info of signal " << cur_lut.in_net_from_info[in] << " in vcd file!" << endl;
                            }
                            instr_1.Operand_Addr.push_back(MEM_DEPTH - 1);
                        }
                        else if (cur_lut.in_net_from_id[in] == -2)  // -2: in net from assign pin bit
                        {
                            if (cur_lut.in_net_from_pos_at_level[in] == 0)
                                Data_Mem_Select.append("0");
                            else
                                Data_Mem_Select.append("1");
                            instr_1.Operand_Addr.push_back(MEM_DEPTH - 1);
                        }
                        else    // in net from out net from luts
                        {        
                            if (cur_lut.in_net_from_part[in] == -3)     // current part
                            {
                                LutType cur_in_from_lut = luts[cur_lut.in_net_from_id[in]];
                                instr_1.Node_Addr = cur_in_from_lut.node_addr;
                                int processors_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                                Processor cur_in_from_pro = processors[processors_id];
                                if (cur_in_from_pro.instr_mem.size() == 1)
                                {
                                    instr_4_num = 1; 
                                    instr_1.Operand_Addr.push_back(1); 
                                    Instr_2 instr_2;
                                    instr_2.Node_Addr = cur_in_from_lut.node_addr;
                                    instr_2.Data_Mem_Select = "0";
                                    instr_2.Operand_Addr = cur_in_from_lut.res_pos_at_mem;
                                    string lut_instr_2 = cat_instr_2(instr_2);
                                    processors[processors_id].instr_mem.push_back(lut_instr_2);
                                    luts[lut_num].res_pos_at_mem = cur_in_from_pro.instr_mem.size();
                                }
                                else if (cur_in_from_pro.instr_mem.size() > 1)
                                {
                                    instr_4_num = cur_in_from_pro.instr_mem.size();
                                    instr_1.Operand_Addr.push_back(cur_in_from_pro.instr_mem.size());
                                    Instr_2 instr_2;
                                    instr_2.Node_Addr = cur_in_from_lut.node_addr;
                                    instr_2.Data_Mem_Select = "0";
                                    instr_2.Operand_Addr = cur_in_from_lut.res_pos_at_mem;
                                    string lut_instr_2 = cat_instr_2(instr_2);
                                    processors[processors_id].instr_mem.push_back(lut_instr_2);
                                    luts[lut_num].res_pos_at_mem = cur_in_from_pro.instr_mem.size();
                                }
                            }
                            else    // other part
                            {
                                // LutType cur_in_from_lut = luts[cur_lut.in_net_from_id];
                                // instr_1.Node_Addr = cur_in_from_lut.node_addr;
                                // Processor cur_in_from_pro = processors[N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1]];
                                // if (cur_in_from_pro.instr_mem.size() == 1)
                                // {
                                //     instr_4_num = 1;
                                //     instr_1.Operand_Addr.push_back(1);
                                //     Instr_2 instr_2;
                                //     instr_2.Node_Addr = cur_in_from_lut.node_addr;
                                //     instr_2.Data_Mem_Select = "0";
                                //     instr_2.Operand_Addr = 0;
                                //     string lut_instr_2 = cat_instr_2(instr_2);
                                //     cur_in_from_pro.instr_mem.push_back(lut_instr_2);
                                // }
                                // else if (cur_in_from_pro.instr_mem.size() > 1)
                                // {
                                //     instr_4_num = cur_in_from_pro.instr_mem.size();
                                //     instr_1.Operand_Addr.push_back(cur_in_from_pro.instr_mem.size());
                                //     Instr_2 instr_2;
                                //     instr_2.Node_Addr = cur_in_from_lut.node_addr;
                                //     instr_2.Data_Mem_Select = "0";
                                //     instr_2.Operand_Addr = cur_in_from_pro.instr_mem.size() - 1;
                                //     string lut_instr_2 = cat_instr_2(instr_2);
                                //     cur_in_from_pro.instr_mem.push_back(lut_instr_2);
                                // }
                            }
                            Data_Mem_Select.append("1");
                        }
                    }
                    instr_1.Data_Mem_Select = Data_Mem_Select;
                    string lut_instr_1 = cat_instr_1(instr_1);
                    string lut_instr_4 = cat_instr_4(instr_4);
                    lut_instrs.insert(lut_instrs.begin(), instr_4_num, lut_instr_4);
                    lut_instrs.push_back(lut_instr_1);
                    cur_processor.instr_mem = lut_instrs;
                }
                // mul inputs from out net of other luts
                else 
                {
                    Instr_1 instr_1;
                    instr_1.LUT_Value = cur_lut.lut_res;
                    vector<int> cur_in_net_from_id = cur_lut.in_net_from_id;
                    vector<int> cur_in_net_from_part = cur_lut.in_net_from_part;
                    map<int, int> from_id_opr_addr; // <in_from_id, operand_addr>
                    for (vector<int>::iterator it = cur_in_net_from_id.begin(); it != cur_in_net_from_id.end(); )
                    {
                        if (*it == -1 || *it == -2)
                        {
                            it = cur_in_net_from_id.erase(it);
                        }
                        else
                        {
                            it++;
                        }
                    }
                    for (vector<int>::iterator it = cur_in_net_from_part.begin(); it != cur_in_net_from_part.end(); )
                    {
                        if (*it == -1 || *it == -2)
                        {
                            it = cur_in_net_from_part.erase(it);
                        }
                        else
                        {
                            it++;
                        }
                    }

                    if (count(cur_in_net_from_part.begin(), cur_in_net_from_part.end(), -3) == cur_in_net_from_part.size()) // all from current part
                    {
                        vector<pair<int, int>> id_instr; // <cur_in_net_from_id, pro_instr_num>
                        vector<int> _cur_in_net_from_id;
                        vector<int> pro_instr_num;
                        vector<int> instr_start_pos;
                        for (int in = 0; in < cur_in_net_from_id.size(); in++)
                        {
                            LutType cur_in_from_lut = luts[cur_in_net_from_id[in]];
                            int processors_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                            Processor cur_in_from_pro = processors[processors_id];
                            // pro_instr_num.push_back(cur_in_from_pro.instr_mem.size());
                            id_instr.push_back(pair<int, int>(cur_in_net_from_id[in], cur_in_from_pro.instr_mem.size()));
                        }
                        sort(id_instr.begin(), id_instr.end(), cmp);
                        for (vector<pair<int, int>>::iterator it = id_instr.begin(); it != id_instr.end(); it++)
                        {
                            _cur_in_net_from_id.push_back(it->first);
                            pro_instr_num.push_back(it->second);
                            instr_start_pos.push_back(it->second + 1);
                        }
                        for (vector<int>::iterator it = instr_start_pos.begin() + 1; it != instr_start_pos.end(); it++)
                        {
                            if (*it == *(it - 1))
                            {
                                *it += 1;
                            }
                            else if (*it < *(it - 1))
                            {
                                *it += (*(it - 1) - *it + 1);
                            }
                        }
                        for (int i = 0; i < _cur_in_net_from_id.size(); i++)
                        {
                            from_id_opr_addr.insert(pair<int, int>(_cur_in_net_from_id[i], instr_start_pos[i]));
                        }
                        for (int i = 0; i < _cur_in_net_from_id.size(); i++)
                        {
                            int cur_id = _cur_in_net_from_id[i];
                            int cur_num = pro_instr_num[i];
                            int cur_pos = instr_start_pos[i];
                            int cur_lut_idle_num = cur_pos;
                            int from_lut_idle_num = cur_pos - cur_num - 1;
                            LutType cur_in_from_lut = luts[cur_id];
                            int processors_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                            Processor cur_in_from_pro = processors[processors_id];
                            if (i == _cur_in_net_from_id.size() - 1)
                            {
                                // current lut
                                Instr_4 instr_4;
                                instr_4.Node_Addr = cur_in_from_lut.node_addr;
                                string lut_instr_4 = cat_instr_4(instr_4);
                                lut_instrs.insert(lut_instrs.end(), cur_lut_idle_num - lut_instrs.size() - 1, lut_instr_4);
                                instr_1.Node_Addr = cur_in_from_lut.node_addr;
                                // cur_processor.instr_mem = lut_instrs;
                                // in from lut
                                Instr_2 instr_2;
                                instr_2.Node_Addr = cur_in_from_lut.node_addr;
                                instr_2.Data_Mem_Select = "0";
                                instr_2.Operand_Addr = cur_in_from_lut.res_pos_at_mem;
                                string lut_instr_2 = cat_instr_2(instr_2);
                                processors[processors_id].instr_mem.insert(processors[processors_id].instr_mem.end(), from_lut_idle_num, lut_instr_4);
                                processors[processors_id].instr_mem.push_back(lut_instr_2);
                            }
                            else
                            {
                                // current lut
                                Instr_4 instr_4;
                                instr_4.Node_Addr = cur_in_from_lut.node_addr;
                                string lut_instr_4 = cat_instr_4(instr_4);
                                lut_instrs.insert(lut_instrs.end(), cur_lut_idle_num - lut_instrs.size(), lut_instr_4);
                                // cur_processor.instr_mem = lut_instrs;
                                // in from lut
                                Instr_2 instr_2;
                                instr_2.Node_Addr = cur_in_from_lut.node_addr;
                                instr_2.Data_Mem_Select = "0";
                                instr_2.Operand_Addr = cur_in_from_lut.res_pos_at_mem;
                                string lut_instr_2 = cat_instr_2(instr_2);
                                processors[processors_id].instr_mem.insert(processors[processors_id].instr_mem.end(), from_lut_idle_num, lut_instr_4);
                                processors[processors_id].instr_mem.push_back(lut_instr_2);
                            }
                        }
                    }
                    else // some from other parts
                    {

                    }

                    // Data_Mem_Select && Operand_Addr
                    for (int in = 0; in < cur_lut.in_ports.size(); in++)
                    {
                        if (cur_lut.in_net_from_id[in] == -1) // -1: in net from initial module
                        {
                            // Data_Mem_Select
                            if (vcd_values.find(cur_lut.in_net_from_info[in]) != vcd_values.end())
                            {
                                vector<unsigned int> *tvs = vcd_times[cur_lut.in_net_from_info[in]];
                                vector<short> *tvs_val = vcd_values[cur_lut.in_net_from_info[in]];
                                vector<unsigned int>::iterator it = find(tvs->begin(), tvs->end(), current_time);
                                auto id = distance(tvs->begin(), it);
                                if (*(tvs_val->begin() + id) == 0)
                                    instr_1.Data_Mem_Select.append("0");
                                else
                                    instr_1.Data_Mem_Select.append("1");
                            }
                            else
                            {
                                cout << "ERROR: No initial info of signal " << cur_lut.in_net_from_info[in] << " in vcd file!" << endl;
                            }
                            instr_1.Operand_Addr.push_back(MEM_DEPTH - 1);
                        }
                        else if (cur_lut.in_net_from_id[in] == -2) // -2: in net from assign pin bit
                        {
                            if (cur_lut.in_net_from_pos_at_level[in] == 0)
                                instr_1.Data_Mem_Select.append("0");
                            else
                                instr_1.Data_Mem_Select.append("1");
                            instr_1.Operand_Addr.push_back(MEM_DEPTH - 1);
                        }
                        else // in net from out net from luts
                        {
                            instr_1.Data_Mem_Select.append("1");
                            map<int, int>::iterator it;
                            it = from_id_opr_addr.find(cur_lut.in_net_from_id[in]);
                            instr_1.Operand_Addr.push_back(it->second - 1);
                        }
                    }
                    string lut_instr_1 = cat_instr_1(instr_1);
                    lut_instrs.push_back(lut_instr_1);
                    cur_processor.instr_mem = lut_instrs;
                    luts[lut_num].res_pos_at_mem = lut_instrs.size() - 1;
                }
                
                processors[N_PROCESSORS_PER_CLUSTER * cluster_num + processor_num] = cur_processor;
                if (processor_num == N_PROCESSORS_PER_CLUSTER || processor_num == cur_luts.size() - 1)
                {
                    cluster_num += 1;
                    processor_num = 0;
                }
                else
                {
                    processor_num += 1;
                }
            }
        }
    }

    ofstream outinstr(instr_out);
    for (map<int, Processor>::iterator it = processors.begin(); it != processors.end(); it++)
    {
        outinstr << "processor " << it->first << ":" << endl;
        vector<string> instrs = it->second.instr_mem;
        for (vector<string>::iterator i = instrs.begin(); i != instrs.end(); i++)
        {
            outinstr << *i << endl;
        }
        outinstr << endl;
    }
    outinstr.close();

    ofstream outdir(out_dir);
    for (map<string, string>::iterator i = pin_bits.begin(); i != pin_bits.end(); i++)
    {
        if (i->second == "output")
        {
            for (map<int, LutType>::iterator it = luts.begin(); it != luts.end(); it++)
            {
                if (it->second.out_ports == i->first)
                {
                    outdir << "Pin: ";
                    outdir.setf(ios::left);
                    outdir.width(10);
                    outdir << i->first;
                    outdir << "Clutser: ";
                    outdir.setf(ios::left);
                    outdir.width(10);
                    outdir << it->second.node_addr[0];
                    outdir << "Processor: ";
                    outdir.setf(ios::left);
                    outdir.width(10);
                    outdir << it->second.node_addr[1] << endl;
                }
            }            
        }
    }
    outdir.close();

    auto end_ins = std::chrono::steady_clock::now();
    long duration_ins = std::chrono::duration_cast<std::chrono::milliseconds>(end_ins - start_ins).count();
    cout << "Successfully finished instruction generation. (" << duration_ins << "ms)" << endl;
    cout << endl;

    auto end_total = std::chrono::steady_clock::now();
    long duration_total = std::chrono::duration_cast<std::chrono::milliseconds>(end_total - start_total).count();
    cout << "COMPILING DONE!    TIME:" << duration_total << "ms" << endl;
    cout << endl;
}


vector<idx_t> part_func(vector<idx_t> &xadj, vector<idx_t> &adjncy, /*vector<idx_t> &adjwgt, */ decltype(METIS_PartGraphKway) *METIS_PartGraphFunc)
{
    idx_t nVertices = xadj.size() - 1;
    idx_t nEdges = adjncy.size() / 2;
    idx_t nWeights = 1;
    idx_t nParts = (nVertices + N_PROCESSORS_PER_CLUSTER - 1) / N_PROCESSORS_PER_CLUSTER;
    assert(nParts <= MAX_CLUSTERS);
    idx_t objval;
    vector<idx_t> part(nVertices, 0);

    int ret = METIS_PartGraphFunc(&nVertices, &nWeights, xadj.data(), adjncy.data(),
                                  NULL, NULL, NULL, &nParts, NULL,
                                  NULL, NULL, &objval, part.data());

    if (ret != rstatus_et::METIS_OK)
    {
        cout << "METIS ERROR!" << endl;
    }
    cout << "METIS OK!" << endl;

    // debug
    // cout << "objval: " << objval << endl;
    // for (unsigned part_i = 0; part_i < part.size(); part_i++)
    // {
    //     cout << part_i + 1 << " " << part[part_i] << endl;
    // }

    return part;
}

string toBinary(int n)
{
    string r;
    while (n != 0)
    {
        r += (n % 2 == 0 ? "0" : "1");
        n /= 2;
    }
    reverse(r.begin(), r.end());
    return r;
}

string HextoBinary(string HexDigit)
{
    string BinDigit;
    for (int i = 0; i < HexDigit.length(); i++)
    {
        char e = HexDigit[i];
        if (e >= 'a' && e <= 'f')
        {
            int a = static_cast<int>(e - 'a' + 10);
            switch (a)
            {
            case 10:
                BinDigit += "1010";
                break;
            case 11:
                BinDigit += "1011";
                break;
            case 12:
                BinDigit += "1100";
                break;
            case 13:
                BinDigit += "1101";
                break;
            case 14:
                BinDigit += "1110";
                break;
            case 15:
                BinDigit += "1111";
                break;
            }
        }
        else if (isdigit(e))
        {
            int b = static_cast<int>(e - '0');
            switch (b)
            {
            case 1:
                BinDigit += "0001";
                break;
            case 2:
                BinDigit += "0010";
                break;
            case 3:
                BinDigit += "0011";
                break;
            case 4:
                BinDigit += "0100";
                break;
            case 5:
                BinDigit += "0101";
                break;
            case 6:
                BinDigit += "0110";
                break;
            case 7:
                BinDigit += "0111";
                break;
            case 8:
                BinDigit += "1000";
                break;
            case 9:
                BinDigit += "1001";
                break;
            }
        }
    }
    return BinDigit;
}

bool cmp(pair<int, int> a, pair<int, int> b)
{
    return a.second < b.second;
}

string cat_instr_1(Instr_1 &instr_1)
{
    string LUT_Value = HextoBinary(instr_1.LUT_Value);
    int filling_num = 4 - instr_1.Operand_Addr.size();
    for (int i = 0; i < filling_num; i++)
    {
        instr_1.Operand_Addr.insert(instr_1.Operand_Addr.begin(), MEM_DEPTH - 1);
    }
    string cat_instr_1;
    stringstream ss;
    ss << "00" << bitset<16>(LUT_Value) << bitset<6>(toBinary(instr_1.Node_Addr[0])) << bitset<6>(toBinary(instr_1.Node_Addr[1])) << bitset<4>(instr_1.Data_Mem_Select)
       << bitset<9>(toBinary(instr_1.Operand_Addr[0])) << bitset<9>(toBinary(instr_1.Operand_Addr[1])) << bitset<9>(toBinary(instr_1.Operand_Addr[2])) << bitset<9>(toBinary(instr_1.Operand_Addr[3]))
       << string(10, '0');
    cat_instr_1 = ss.str();
    return cat_instr_1;
}

string cat_instr_2(Instr_2 &instr_2)
{
    string cat_instr_2;
    stringstream ss;
    ss << "01" << string(16, '0') << bitset<6>(toBinary(instr_2.Node_Addr[0])) << bitset<6>(toBinary(instr_2.Node_Addr[1])) << instr_2.Data_Mem_Select << bitset<9>(toBinary(instr_2.Operand_Addr)) << string(40, '0');
    cat_instr_2 = ss.str();
    return cat_instr_2;
}

string cat_instr_3(Instr_3 &instr_3)
{
    string cat_instr_3;
    stringstream ss;
    ss << "10" << string(16, '0') << bitset<6>(toBinary(instr_3.Node_Addr[0])) << bitset<6>(toBinary(instr_3.Node_Addr[1])) << instr_3.Static_Binary_Value << string(49, '0');
    cat_instr_3 = ss.str();
    return cat_instr_3;
}

string cat_instr_4(Instr_4 &instr_4)
{
    string cat_instr_4;
    stringstream ss;
    ss << "11" << string(16, '0') << bitset<6>(toBinary(instr_4.Node_Addr[0])) << bitset<6>(toBinary(instr_4.Node_Addr[1])) << string(50, '0');
    cat_instr_4 = ss.str();
    return cat_instr_4;
}