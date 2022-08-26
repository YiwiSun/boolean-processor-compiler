#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <limits>
#include <cstddef>
#include <bitset>
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
    std::string v_path, vcdFilePath, instr_out; /*database_path, saif_out, dumpon_time_str, dumpoff_time_str*/
    // int dumpon_time, dumpoff_time;
    if (argc == 4)
    {
        // database_path = argv[1];
        v_path = argv[1];
        vcdFilePath = argv[2]; 
        instr_out = argv[3];
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

    vector<idx_t> part = part(xadj, adjncy, /*adjwgt, */ METIS_PartGraphRecursive);
    // vector<idx_t> part = func(xadj, adjncy, adjwgt, METIS_PartGraphKway);

    ofstream outpartition("../test/partition.txt");
    for (int i = 0; i < part.size(); i++)
    {
        outpartition << i << " " << part[i] << endl;
    }
    outpartition.close();

    auto end_par = std::chrono::steady_clock::now();
    long duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par).count();
    cout << "Successfully finished graph partition. (" << duration_par << "ms)" << endl;
    cout << endl;



    /**************************** pre processing ******************************/
    cout << "4. Executing pre-processing." << endl;
    auto start_pre = std::chrono::steady_clock::now();

    int parts = (_parser.luts.size() + N_PROCESSORS_PER_CLUSTER - 1) / N_PROCESSORS_PER_CLUSTER;    // part nums
    map<int, vector<int>> luts_in_part;     // partition : luts
    for (int i = 0; i < parts; i++) {
        for (int j = 0; j < part.size(); j++) {
            if (part[j] == i) {
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
        for (int l = 0; l < cur_part_levels.size(); l++) {
            if (cur_part_levels[l].empty()) {
                cur_part_levels.erase(cur_part_levels.begin() + l);
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
     


    /******************************* vcd parsing *******************************/
    cout << "5. Executing vcd file parsing." << endl;
    auto start_vcd = std::chrono::steady_clock::now();
    VCDParser _vcdparser;               // = new VCDParser();
    _vcdparser.parse(vcdFilePath);      //, initial_net_map))
    auto end_vcd = std::chrono::steady_clock::now();
    long duration_vcd = std::chrono::duration_cast<std::chrono::milliseconds>(end_vcd - start_vcd).count();
    cout << "Successfully finished vcd file parsing. (" << duration_vcd << "ms)" << endl;
    cout << endl;
    


    /**************************** instruction generation *******************************/
    int cluster_num = 0;
    int processor_num = 0;
    for (int p = 0; p < parts; p++) {
        vector<vector<int>> cur_part_levels = parts_levels[p];
        vector<int> cur_luts = luts_in_part[p];
        for (int l = 0; l < cur_part_levels.size(); l++) {
            vector<int> cur_level = cur_part_levels[l];
            for (int i = 0; i < cur_level.size(); i++) {
                int lut_num = cur_level[i];
                LutType cur_lut = luts[lut_num];
                cur_lut.node_addr = {cluster_num, processor_num};
                string lut_instr;
                vector<string> lut_instrs;
                Processor cur_processor;
                int in_num = 0;
                int cal = 0;
                string lut_input_select;
                for (int in = 0; in < cur_lut.in_ports.size(); in++) {
                    int in_net_from_id = cur_lut.in_net_from_id[in]; // in net from out net from luts; -1: in net from initial module; -2: in net from assign pin bit
                    string in_net_from_info = cur_lut.in_net_from_info[in];
                    int in_net_from_part = cur_lut.in_net_from_part[in]; // -1: in net from initial module; -2: in net from assign pin bit; -3: current part; other: part num
                    int in_net_from_level = cur_lut.in_net_from_level[in];
                    int in_net_from_pos_at_level = cur_lut.in_net_from_pos_at_level[in];
                    // in net from initial module
                    if (in_net_from_id == -1) {
                        // instr
                        // cur_processor.instr_mem.push_back();
                        // cur_processor.inter_mem.push_back();
                        in_num += 1;
                        lut_input_select.append("0");
                    }
                    // in net from assign pin bit
                    else if (in_net_from_id == -2) {
                        // instr
                        // cur_processor.instr_mem.push_back();
                        // cur_processor.inter_mem.push_back();
                        in_num += 1;
                        lut_input_select.append("0");
                    }
                    // in net from out net from luts
                    else {
                        if (in_net_from_part == -3) {
                            if (in_num == (cur_lut.in_ports.size() - 1)) {
                                Instr_1 instr_1;
                                instr_1.LUT_Value = cur_lut.lut_res;
                                instr_1.Node_Addr = luts[in_net_from_id].node_addr;
                            }
                            else {
                                Instr_4 instr_4;
                                instr_4.Node_Addr = luts[in_net_from_id].node_addr;
                            }

                        }
                        // in net from lut of other part
                        else {

                        }
                    }

                }
                processors[N_PROCESSORS_PER_CLUSTER * cluster_num + processor_num] = cur_processor;
                if (processor_num == N_PROCESSORS_PER_CLUSTER || processor_num == cur_luts.size())
                {
                    cluster_num += 1;
                    processor_num = 0;
                }
                else {
                    processor_num += 1;
                }
            }
        }
    }
































        // int tmpp = -1;
        // long duration_pro0 = 0;     // process input timedvalue per level
        // long duration_pro1 = 0;     // process input merging
        // long duration_pre = 0;      // duration_pro0 + duration_pro1
        // long duration_gpu = 0; 
        // long duration_cuda = 0;
        // long duration_cuda2 = 0;
        // long duration_out = 0;
        // long duration_out2 = 0;
        // long duration_data = 0;
        
        // vcd signals
        // std::vector<VCDSignal*> signals = parser.get_signals();
        // std::vector<VCDTime>    times = parser.get_timestamps();
        VCDTimeUnit time_unit = _vcdparser.time_units;
        unsigned time_res = _vcdparser.time_resolution;
        VCDScope *root = _vcdparser.root_scope;
        string root_name = root->name;

        // std::unordered_map<std::string, TimedValues*> val_map = parser.get_val_map();
        // std::vector<std::vector<unsigned int> > times = parser->times;
        // std::vector<std::vector<short> > values = parser->values;
        // std::unordered_map<std::string, std::vector<unsigned int>* > times = parser.times;
        // std::unordered_map<std::string, std::vector<short>* > values = parser.values;

        // delete parser;

        











        /** Simulate **/
        // cout << "start simulator..." << endl;
        // auto start = std::chrono::steady_clock::now();
        
        // std::unordered_map<std::string, std::vector<unsigned int>* > out_times;
        // std::unordered_map<std::string, std::vector<short>* > out_values;
        // for (unsigned i = 0; i < _inter.levels.size(); ++i)
        // {
        //     cout << "level " << i << endl;
        //     const int _cur_level = i;
        //     std::vector<int> cur_level = (_inter.levels)[i];
        //     const int lut_num = cur_level.size();
            
        //     int blocks_per_lut = 1;
        //     int n_blocks = lut_num;

        //     int height = lut_num;   // lut num of current level
        //     //data to processor
        //     int _time_unit = time_unit;
        //     //int *dev_time_unit;

        //     //int *host_out_data;
            
        //     //size_t pitch_out;
        //     int out_width = 0;
        //     int out_width2 = 0;

        //     //short *host_in_data;
           
        //     //int *dev_in_data;  // 2D
        //     //size_t pitch_data;
        //     int data_width = 0;
        //     //int _data_width=0;
        //     int process_data_width = 0;

        //     //int *host_data_val_num;
            
        //     //size_t pitch_val_num;
        //     unsigned int val_num_width=0;

            
        //     //size_t pitch_function;
        //     int functions_width=0;

        //     //unsigned long long *host_times;
        //     unsigned int times_width=0;
        //     //unsigned long long *dev_in_times;
        //     //unsigned long long *dev_times;
        //     //unsigned long long *dev_out_times;
        //     //size_t pitch_times;
        //     //size_t pitch_out_times;
    
        //     // int delay_width=0;

        //     // some flag
        //     short *data_in_num = (short *)malloc(sizeof(short) * height);   // inports num of each lut in current level
        //     short *data_out_num = (short *)malloc(sizeof(short) * height);  // outports num of each lut in current level

        //     unsigned int _end = stoull(dumpoff_time_str);
        //     short *input_datas = (short *)malloc(sizeof(short) * _end * 4);
        //     // cout << lut_num << endl;

        //     unsigned int input_datas_size = 0;
        //     unsigned int *total_input_times = (unsigned int *)malloc(sizeof(unsigned int) * (_end * 2)); // inst_num*N_PROCESSORS_PER_CLUSTER*N_INS_PER_PROCESSOR*2);
        //     unsigned int total_input_times_size = 0;
        //     std::vector<unsigned int> total_times_sizes(lut_num);   // num of timestamps of each lut
        //     std::vector<unsigned int> total_times_start(lut_num);   // start time of each lut

        //     unsigned int *input_start = (unsigned int *)malloc(sizeof(unsigned int) * lut_num);
        //     unsigned int *output_start = (unsigned int *)malloc(sizeof(unsigned int) * lut_num);
        //     unsigned int *output_size = (unsigned int *)malloc(sizeof(unsigned int) * lut_num);
        //     unsigned int *output_start2 = (unsigned int *)malloc(sizeof(unsigned int) * lut_num);
        //     unsigned int *output_size2 = (unsigned int *)malloc(sizeof(unsigned int) * lut_num);
        //     if (output_size == NULL)
        //     {
        //         cout << "error: memory full!" << endl;
        //         exit(-1);
        //     }


        //     auto start_pro = std::chrono::steady_clock::now();
        //     unsigned int max_out = 0;
        //     unsigned int times_start = 0;
        //     for (int j = 0; j < lut_num; ++j)
        //     {
        //         LutType *cur_lut = _inter._parser.luts.find(cur_level[j]); 
        //         std::vector<string> cur_in = cur_lut->in_ports;
        //         data_in_num[j] = cur_in.size();
        //         std::vector<string> cur_out = cur_lut->out_ports;
        //         data_out_num[j] = cur_out.size();
        //         // std::map<std::string, TimedValues> bitValues;
        //         // std::vector<TimedValues> tvs_vec(cur_in.size());
        //         // std::vector<VCDBit> last_vec(cur_in.size());
        //         std::vector<unsigned int> tvs_size(cur_in.size());
        //         // std::vector<int> tvs_flag(cur_in.size());
        //         // std::vector<int> _tvs_flag(cur_in.size());
        //         // int tmp_times_size = 0;

        //         // std::vector<TimedValues*> tvs(cur_in.size());
        //         std::vector<std::vector<unsigned int> *> tvs(cur_in.size());
        //         std::vector<std::vector<short> *> tvs_val(cur_in.size());
        //         unsigned int sum = 0;   // sum of timestamps per lut
        //         // auto start_pro0 = std::chrono::steady_clock::now();
        //         for (unsigned it = 0; it < cur_in.size(); it++)
        //         {
        //             string in_name = cur_in[it];
        //             // cout << "name:" << in_name << endl;
        //             //  = bitValues[cur_in[it]];
        //             if ((_vcdparser.times).find(in_name) != (_vcdparser.times).end())
        //             {
        //                 // TimedValues cur_tv_deq;
        //                 tvs[it] = (_vcdparser.times)[in_name];
        //                 tvs_val[it] = (_vcdparser.values)[in_name];
        //                 // bitValues[in_name] = cur_tv_deq;
        //             }
        //             else if (out_values.find(in_name) != out_values.end())
        //             {
        //                 tvs[it] = out_times[in_name];
        //                 tvs_val[it] = out_values[in_name];
        //             }
        //             else if (pinbitValues.find(in_name) != pinbitValues.end())
        //             {
        //                 // tvs[it] = pinbitValues[in_name];
        //                 std::vector<unsigned int> *_tvs = new std::vector<unsigned int>();
        //                 std::vector<short> *_tvs_val = new std::vector<short>();
        //                 _tvs->push_back(0);
        //                 TimedValues *te = pinbitValues[in_name];
        //                 TimedValue tev = *(te->begin());
        //                 _tvs_val->push_back(tev.value);
        //                 tvs[it] = _tvs;
        //                 tvs_val[it] = _tvs_val;
        //             }
        //             else if (assign_pairs.find(in_name) != assign_pairs.end())
        //             {
        //                 string temp = assign_pairs[in_name];
        //                 if ((_vcdparser.times).find(temp) != (_vcdparser.times).end())
        //                 {
        //                     // tvs[it] = val_map[temp];
        //                     tvs[it] = (_vcdparser.times)[temp];
        //                     tvs_val[it] = (_vcdparser.values)[temp];
        //                     // bitValues[in_name] = cur_tv_deq;
        //                 }
        //                 else if (out_values.find(temp) != out_values.end())
        //                 {
        //                     tvs[it] = out_times[temp];
        //                     tvs_val[it] = out_values[temp];
        //                 }
        //                 else if (pinbitValues.find(temp) != pinbitValues.end())
        //                 {
        //                     std::vector<unsigned int> *_tvs = new std::vector<unsigned int>();
        //                     std::vector<short> *_tvs_val = new std::vector<short>();
        //                     _tvs->push_back(0);
        //                     TimedValues *te = pinbitValues[temp];
        //                     TimedValue tev = *(te->begin());
        //                     _tvs_val->push_back(tev.value);
        //                     tvs[it] = _tvs;
        //                     tvs_val[it] = _tvs_val;
        //                 }
        //             }
        //             else
        //             {
        //                 cout << "error." << endl;
        //                 exit(-1);
        //             }

        //             // another method
        //             // tvs_vec[it] = tvs[it];
        //             // last_vec[it] = VCD_X;
        //             unsigned int step = tvs[it]->size();
        //             tvs_size[it] = step;
        //             sum += step;
        //             // tmp_times_size += tvs_size[it];
        //             // tvs_flag[it] = 0;
        //             //_tvs_flag[it] = 0;
        //         }
        //         // auto end_pro0 = std::chrono::steady_clock::now();
        //         // duration_pro0 += std::chrono::duration_cast<std::chrono::milliseconds>(end_pro0 - start_pro0).count();


        //         // auto start_pro1 = std::chrono::steady_clock::now();
        //         unsigned int tmp_size = tvs_size[0];
        //         if (data_in_num[j] == 1)
        //         {
        //             unsigned _size = tvs_size[0];
        //             auto pos = tvs[0]->begin();
        //             auto pos_val = tvs_val[0]->begin();
        //             for (unsigned it = 0; it < _size; ++it)
        //             {
        //                 total_input_times[times_start + it] = *(pos + it);
        //                 input_datas[data_width + it] = *(pos_val + it);
        //             }
        //         }
        //         else if (data_in_num[j] % 2 == 0)
        //         {
        //             unsigned _size = data_in_num[j];
        //             unsigned sorted_num = _size / 2;
        //             std::vector<unsigned int> s(sorted_num);
        //             unsigned int shift = 0;
        //             unsigned int val_shift = 0;
        //             for (unsigned it = 0; it < sorted_num; it++)
        //             {
        //                 unsigned end1 = tvs_size[it * 2];
        //                 unsigned end2 = tvs_size[it * 2 + 1];
        //                 unsigned start = times_start + shift;        //(it == 0 ? 0 : s[it-1]);
        //                 unsigned val_start = data_width + val_shift; //(it == 0 ? 0 : (s[it-1]*2));
        //                 unsigned int *list1 = &(*(tvs[it * 2]->begin()));
        //                 unsigned int *list2 = &(*(tvs[it * 2 + 1]->begin()));
        //                 short *val_list1 = &(*(tvs_val[it * 2]->begin()));
        //                 short *val_list2 = &(*(tvs_val[it * 2 + 1]->begin()));
        //                 merge2(total_input_times, start, tmp_size,
        //                        input_datas, val_start, // val_tmp_size,
        //                        // tvs[it*2],tvs_val[it*2], end1, tvs[it*2+1],tvs_val[it*2+1], end2);
        //                        list1, val_list1, end1, list2, val_list2, end2);
        //                 s[it] = tmp_size;
        //                 shift += tmp_size;
        //                 val_shift = shift * (it + 1) * 2;
        //             }
        //             tmp_size = s[0];
        //             unsigned val_tmp_size = s[0] * 2;
        //             shift = s[0];
        //             val_shift = s[0] * 2;
        //             for (unsigned it = 1; it < sorted_num; it++)
        //             {
        //                 unsigned id = 2 * it;
        //                 unsigned start2 = times_start + shift;
        //                 unsigned val_start2 = data_width + val_shift;
        //                 unsigned end1 = times_start + tmp_size;
        //                 unsigned val_end1 = data_width + val_tmp_size;
        //                 shift += s[it];
        //                 val_shift = shift * (it + 1) * 2;
        //                 unsigned end2 = times_start + shift;
        //                 unsigned val_end2 = val_start2 + 2 * s[it];
        //                 merge_ptr(total_input_times, times_start, start2, end1, end2, tmp_size,
        //                           input_datas, data_width, val_start2, val_end1, val_end2, val_tmp_size,
        //                           id);
        //             }
        //         }
        //         else
        //         {
        //             unsigned _size = data_in_num[j];
        //             unsigned sorted_num = _size / 2;
        //             std::vector<unsigned int> s(sorted_num);
        //             unsigned int shift = 0;
        //             unsigned int val_shift = 0;
        //             for (unsigned it = 0; it < sorted_num; it++)
        //             {
        //                 unsigned end1 = tvs_size[it * 2];
        //                 unsigned end2 = tvs_size[it * 2 + 1];
        //                 unsigned start = times_start + shift;        //(it == 0 ? 0 : s[it-1]);
        //                 unsigned val_start = data_width + val_shift; //(it == 0 ? 0 : (s[it-1]*2));
        //                 unsigned int *list1 = &(*(tvs[it * 2]->begin()));
        //                 unsigned int *list2 = &(*(tvs[it * 2 + 1]->begin()));
        //                 short *val_list1 = &(*(tvs_val[it * 2]->begin()));
        //                 short *val_list2 = &(*(tvs_val[it * 2 + 1]->begin()));
        //                 merge2(total_input_times, start, tmp_size,
        //                        input_datas, val_start, // val_tmp_size,
        //                        // tvs[it*2],tvs_val[it*2], end1, tvs[it*2+1],tvs_val[it*2+1], end2);
        //                        list1, val_list1, end1, list2, val_list2, end2);
        //                 s[it] = tmp_size;
        //                 shift += tmp_size;
        //                 val_shift = shift * (it + 1) * 2;
        //             }
        //             tmp_size = s[0];
        //             unsigned val_tmp_size = s[0] * 2;
        //             shift = s[0];
        //             val_shift = s[0] * 2;
        //             unsigned id = 2;
        //             unsigned it;
        //             for (it = 1; it < sorted_num; it++)
        //             {
        //                 id = 2 * it;
        //                 unsigned start2 = times_start + shift;
        //                 unsigned val_start2 = data_width + val_shift;
        //                 unsigned end1 = times_start + tmp_size;
        //                 unsigned val_end1 = data_width + val_tmp_size;
        //                 shift += s[it];
        //                 val_shift = shift * (it + 1) * 2;
        //                 unsigned end2 = times_start + shift;
        //                 unsigned val_end2 = val_start2 + 2 * s[it];
        //                 merge_ptr(total_input_times, times_start, start2, end1, end2, tmp_size,
        //                           input_datas, data_width, val_start2, val_end1, val_end2, val_tmp_size,
        //                           id);
        //             }
        //             id = 2 * it;
        //             unsigned int *list1 = &(*(tvs[_size - 1]->begin()));
        //             short *val_list1 = &(*(tvs_val[_size - 1]->begin()));
        //             merge(total_input_times, times_start, tmp_size,
        //                   input_datas, data_width, val_tmp_size, id,
        //                   // tvs[it], tvs_val[it], tvs_size[it]);
        //                   list1, val_list1, tvs_size[_size - 1]);
        //         }
        //         // auto end_pro1 = std::chrono::steady_clock::now();
        //         // duration_pro1 += std::chrono::duration_cast<std::chrono::milliseconds>(end_pro1 - start_pro1).count();

        //         total_times_sizes[j] = tmp_size;
        //         total_times_start[j] = times_start;
        //         input_start[j] = data_width;
        //         times_start += tmp_size;

        //         int _blocks_per_lut = (tmp_size + N_PROCESSORS_PER_CLUSTER * N_INS_PER_PROCESSOR - 1) / (N_PROCESSORS_PER_CLUSTER * N_INS_PER_PROCESSOR);
        //         if (_blocks_per_lut * lut_num > MAX_CLUSTERS)
        //         {
        //             _blocks_per_lut = MAX_CLUSTERS / lut_num;
        //         }
        //         blocks_per_lut = max(blocks_per_lut, _blocks_per_lut);

        //         data_width += tmp_size * data_in_num[j];
        //     }
        //     n_blocks = lut_num * blocks_per_lut;
        //     cout << "blocks_per_lut: " << blocks_per_lut << endl;
        //     cout << "data_width size: " << data_width << endl;
        //     // cout << "max_out size: " << max_out << endl;
        //     // out_width = total_input_times->size() * max_out;
        //     auto end_pro = std::chrono::steady_clock::now();
        //     long duration_pro = std::chrono::duration_cast<std::chrono::milliseconds>(end_pro - start_pro).count();
        //     cout << "total time of proccess: " << duration_pro << "ms" << endl;
        //     duration_pre += duration_pro;


        //     unsigned int *dev_in_times;
        //     auto start_data = std::chrono::steady_clock::now();
        //     unsigned int *dev_times;
        //     total_input_times_size = times_start;

            
        //     short *out_data = (short *)malloc(sizeof(short) * out_width);
        //     unsigned int *out_time = (unsigned int *)malloc(sizeof(unsigned int) * out_width);
        //     short *out_splited_width = (short *)malloc(sizeof(short) * height);
        //     for (int j = 0; j < cur_level.size(); ++j)
        //     {
        //         // string _name = (_inter.p.get_instance_names())[cur_level[j]];
        //         Instance *cur_inst = _inter.find_inst(cur_level[j]);
        //         //&((_inter.p.get_instances())[_name]);
        //         std::vector<string> cur_out = cur_inst->out_net;
        //         unsigned int out_num = cur_out.size();
        //         unsigned int cur_width = out_splited_width[j];
        //         // cout << "out_splited_width: " << cur_width << endl;
        //         for (int k = 0; k < out_num; ++k)
        //         {
        //             string cur_out_name = cur_out[k];
        //             std::vector<unsigned int> *tvs = new std::vector<unsigned int>();
        //             std::vector<short> *tvs_val = new std::vector<short>();
        //             tvs->push_back(0);
        //             tvs_val->push_back(VCD_X);
        //             // out_times[cur_out_name].push_back(0);
        //             // out_datas[cur_out_name].push_back(2);
        //             // int out_start = j*out_width+k*(out_width/out_num);
        //             unsigned int out_start = output_start[j] + output_size[j] * k;
        //             // for (int l = out_start; l < out_start + out_width/out_num; ++l)
        //             unsigned int last_out_time = 0;
        //             short last_out_data = VCD_X;
        //             for (int m = 0; m < N_PROCESSORS_PER_CLUSTER * blocks_per_inst; ++m)
        //             {
        //                 int l;
        //                 for (l = out_start + m * cur_width; l < out_start + (m + 1) * cur_width; ++l)
        //                 {
        //                     if (l >= out_start + output_size[j])
        //                     {
        //                         break;
        //                     }
        //                     if (l == out_start + m * cur_width && m != 0)
        //                     {
        //                         if (out_data[l] != last_out_data && out_data[l] != -1)
        //                         {
        //                             cout << "WARNING: out:" << cur_out_name << " conflict at thread " << m << " and position " << l << ": curout is " << out_data[l] << ", lastout is " << last_out_data << endl;
        //                         }
        //                         continue;
        //                     }

        //                     if (out_data[l] == -1)
        //                     {
        //                         last_out_time = (tvs->back());
        //                         last_out_data = (tvs_val->back());
        //                         // if(cur_out_name == "tile_ICCADs_core_ICCADs_div_ICCADs_intadd_6_ICCADs_n18" && m >= 6341 && m <= 6343)
        //                         //     cout << "thread: " << m << ", the last -- last_time:" << last_out_time << ", last_value:" << last_out_data << endl;
        //                         break;
        //                     }
        //                     if (l == out_start + m * cur_width + 1 && m != 0)
        //                     {
        //                         // if(cur_out_name == "tile_ICCADs_core_ICCADs_div_ICCADs_intadd_6_ICCADs_n18" && m >= 6341 && m <= 6343)
        //                         //     cout << "WARNING: out:" << cur_out_name << "(thread:" << m << ") time conflict, current out time " << out_time[l] << " less than last_out_time " << last_out_time << "; cur_out: " << out_data[l] <<", next: " << out_data[l+1] << ", last_out: " << last_out_data << ", pos: " << l - out_start << endl;
        //                         while (out_time[l] <= last_out_time && out_time[l] > 0)
        //                         {

        //                             if (tvs->size() <= 1)
        //                             {
        //                                 // out_times[cur_out_name].pop_back();
        //                                 // out_datas[cur_out_name].pop_back();
        //                                 tvs->pop_back();
        //                                 tvs_val->pop_back();
        //                                 break;
        //                             }
        //                             // out_times[cur_out_name].pop_back();
        //                             // out_datas[cur_out_name].pop_back();
        //                             tvs->pop_back();
        //                             tvs_val->pop_back();
        //                             last_out_time = (tvs->back());
        //                             last_out_data = (tvs_val->back());
        //                             // if(cur_out_name == "tile_ICCADs_core_ICCADs_div_ICCADs_intadd_6_ICCADs_n18")
        //                             //     cout << "\tAfter pop: out:" << cur_out_name << "(thread:" << m << ") time conflict, current out time " << out_time[l] << " less than last_out_time " << last_out_time << "; cur_out: " << out_data[l] <<", next: " << out_data[l+1] << ", last_out: " << last_out_data << ", pos: " << l - out_start << endl;
        //                         }
        //                     }
        //                     unsigned int cur_out_time = out_time[l];
        //                     short cur_out_data = out_data[l];
        //                     if (cur_out_data == -1)
        //                     {
        //                         break;
        //                     }
        //                     // if(cur_out_name == "tile_ICCADs_core_ICCADs_div_ICCADs_intadd_6_ICCADs_n18" && m >= 6341 && m <= 6343)
        //                     //     cout << "thread: " << m << ", last_time:" << last_out_time << ", last_value:" << last_out_data << ";cur: " << cur_out_time << ", " << cur_out_data << endl;
        //                     if (cur_out_data != last_out_data)
        //                     {
        //                         tvs->push_back(cur_out_time);
        //                         tvs_val->push_back(cur_out_data);
        //                         // out_times[cur_out_name].push_back(cur_out_time);
        //                         // out_datas[cur_out_name].push_back(cur_out_data);
        //                     }
        //                     last_out_time = (tvs->back());
        //                     last_out_data = (tvs_val->back());
        //                     // if(cur_out_name == "tile_ICCADs_core_ICCADs_div_ICCADs_intadd_6_ICCADs_n18" && m >= 6341 && m <= 6343)
        //                     //     cout << "thread: " << m << ", last_time:" << last_out_time << ", last_value:" << last_out_data << endl;
        //                 }
        //                 if (l >= out_start + output_size[j])
        //                 {
        //                     break;
        //                 }
        //             }
        //             out_times[cur_out_name] = tvs;
        //             out_values[cur_out_name] = tvs_val;
        //         }
        //     }

        // }


}


// void merge_ptr(unsigned int *list, unsigned int start1, unsigned int start2, unsigned int end1, unsigned int end2, unsigned int &length,
//                short *val_list, unsigned int val_start1, unsigned int val_start2, unsigned int val_end1, unsigned int val_end2, unsigned int &val_length,
//                unsigned int id)
// {
//     unsigned int *sorted = (unsigned int *)malloc(sizeof(unsigned int) * (end1 - start1 + end2 - start2));
//     short *val_sorted = (short *)malloc(sizeof(short) * (end1 - start1 + end2 - start2) * (id + 2)); // must id and 2 merged
//     unsigned int ti = 0, val_ti = 0, i = start1, j = start2, val_i = val_start1, val_j = val_start2;
//     while (i < end1 || j < end2)
//     {
//         if (j == end2)
//         {
//             sorted[ti] = list[i++];
//             for (unsigned k = 0; k < id; ++k)
//             {
//                 val_sorted[val_ti + k] = val_list[val_i + k];
//             }
//             val_sorted[val_ti + id] = val_list[val_j - 2];
//             val_sorted[val_ti + id + 1] = val_list[val_j - 1];
//             val_i += id;
//         }
//         else if (i == end1)
//         {
//             sorted[ti] = list[j++];
//             for (unsigned k = 0; k < id; ++k)
//             {
//                 val_sorted[val_ti + k] = val_list[val_i - id + k];
//             }
//             val_sorted[val_ti + id] = val_list[val_j];
//             val_sorted[val_ti + id + 1] = val_list[val_j + 1];
//             val_j += 2;
//         }
//         else if (list[i] < list[j])
//         {
//             sorted[ti] = list[i++];
//             for (unsigned k = 0; k < id; ++k)
//             {
//                 val_sorted[val_ti + k] = val_list[val_i + k];
//             }
//             val_sorted[val_ti + id] = val_list[val_j - 2];
//             val_sorted[val_ti + id + 1] = val_list[val_j - 1];
//             val_i += id;
//         }
//         else if (list[i] > list[j])
//         {
//             sorted[ti] = list[j++];
//             for (unsigned k = 0; k < id; ++k)
//             {
//                 val_sorted[val_ti + k] = val_list[val_i - id + k];
//             }
//             val_sorted[val_ti + id] = val_list[val_j];
//             val_sorted[val_ti + id + 1] = val_list[val_j + 1];
//             val_j += 2;
//         }
//         else
//         {
//             sorted[ti] = list[i++];
//             j++;
//             for (unsigned k = 0; k < id; ++k)
//             {
//                 val_sorted[val_ti + k] = val_list[val_i + k];
//             }
//             val_sorted[val_ti + id] = val_list[val_j];
//             val_sorted[val_ti + id + 1] = val_list[val_j + 1];
//             val_i += id;
//             val_j += 2;
//         }
//         ti++;
//         val_ti += (id + 2);
//     }
//     length = ti;
//     val_length = val_ti;
//     for (val_ti = 0, ti = 0; ti < length; ti++, val_ti += (id + 2))
//     {
//         list[ti + start1] = sorted[ti];
//         for (int k = 0; k < id + 2; k++)
//         {
//             val_list[k + val_ti + val_start1] = val_sorted[val_ti + k];
//         }
//     }
//     free(sorted);
//     free(val_sorted);
// }

// void merge2(unsigned int *list, unsigned int start, unsigned int &length,
//             short *val_list, unsigned int val_start, // unsigned int &val_length,
//             // std::vector<unsigned int>* list1, std::vector<short>* val_list1, unsigned int end1,
//             // std::vector<unsigned int>* list2, std::vector<short>* val_list2, unsigned int end2)
//             unsigned int *list1, short *val_list1, unsigned int end1,
//             unsigned int *list2, short *val_list2, unsigned int end2)
// {
//     unsigned int ti = start, val_ti = val_start, i = 0, j = 0;
//     while (i < end1 || j < end2)
//     {
//         if (j == end2)
//         {
//             list[ti] = list1[i];
//             val_list[val_ti] = val_list1[i];
//             val_list[val_ti + 1] = val_list2[j - 1];
//             i++;
//         }
//         else if (i == end1)
//         {
//             list[ti] = list2[j];
//             val_list[val_ti] = val_list1[i - 1];
//             val_list[val_ti + 1] = val_list2[j];
//             j++;
//         }
//         else if (list1[i] < list2[j])
//         {
//             list[ti] = list1[i];
//             val_list[val_ti] = val_list1[i];
//             val_list[val_ti + 1] = val_list2[j - 1];
//             i++;
//         }
//         else if (list1[i] > list2[j])
//         {
//             list[ti] = list2[j];
//             val_list[val_ti] = val_list1[i - 1];
//             if (start == 33038210)
//             {
//                 cout << "j: " << j << ", val_ti: " << val_ti << endl;
//                 cout << "time: " << ti << "--" << list[ti] << endl;
//             }
//             val_list[val_ti + 1] = val_list2[j];
//             j++;
//         }
//         else
//         {
//             list[ti] = list1[i];
//             val_list[val_ti] = val_list1[i];
//             val_list[val_ti + 1] = val_list2[j];
//             j++;
//             i++;
//         }
//         ti++;
//         val_ti += 2;
//     }
//     length = ti - start;
//     // val_length = val_ti - val_start;
// }

// void merge(unsigned int *list, unsigned int start, unsigned int &length,
//            short *val_list, unsigned int val_start, unsigned int &val_length, unsigned int id,
//            // std::vector<unsigned int>* list1, std::vector<short>* val_list1, unsigned int end2)
//            unsigned int *list1, short *val_list1, unsigned int end2)
// {
//     unsigned int *sorted = (unsigned int *)malloc(sizeof(unsigned int) * (end2 + length));
//     short *val_sorted = (short *)malloc(sizeof(short) * (end2 + length) * (id + 1));
//     unsigned int ti = 0, val_ti = 0, i = start, j = 0, val_i = val_start, end1 = start + length;
//     // auto start1 = list1->begin();
//     // auto val_start1 = val_list1->begin();
//     while (i < end1 || j < end2)
//     {
//         if (j == end2)
//         {
//             sorted[ti] = list[i++];
//             for (int k = 0; k < id; ++k)
//             {
//                 val_sorted[val_ti + k] = val_list[val_i + k];
//             }
//             // val_sorted[val_ti+id] = *(val_start1+j-1);
//             val_sorted[val_ti + id] = val_list1[j - 1];
//             val_i += id;
//         }
//         else if (i == end1)
//         {
//             // sorted[ti] = *(start1+j);
//             sorted[ti] = list1[j];
//             for (int k = 0; k < id; ++k)
//             {
//                 val_sorted[val_ti + k] = val_list[val_i - id + k];
//             }
//             // val_sorted[val_ti+id] = *(val_start1+j);
//             val_sorted[val_ti + id] = val_list1[j];
//             j++;
//         }
//         // else if (*(start1+j) < list[i]) {
//         else if (list1[j] < list[i])
//         {
//             // sorted[ti] = *(start1+j);
//             sorted[ti] = list1[j];
//             for (int k = 0; k < id; ++k)
//             {
//                 val_sorted[val_ti + k] = val_list[val_i - id + k];
//             }
//             // val_sorted[val_ti+id] = *(val_start1+j);
//             val_sorted[val_ti + id] = val_list1[j];
//             j++;
//         }
//         else if (list1[j] > list[i])
//         {
//             // else if (*(start1+j) > list[i]) {
//             sorted[ti] = list[i++];
//             for (int k = 0; k < id; ++k)
//             {
//                 val_sorted[val_ti + k] = val_list[val_i + k];
//             }
//             // val_sorted[val_ti+id] = *(val_start1+j-1);
//             val_sorted[val_ti + id] = val_list1[j - 1];
//             val_i += id;
//         }
//         else
//         {
//             sorted[ti] = list[i++];
//             for (int k = 0; k < id; ++k)
//             {
//                 val_sorted[val_ti + k] = val_list[val_i + k];
//             }
//             // val_sorted[val_ti+id] = *(val_start1+j);
//             val_sorted[val_ti + id] = val_list1[j];
//             val_i += id;
//             j++;
//         }
//         ti++;
//         val_ti += (id + 1);
//     }
//     length = ti;
//     val_length = val_ti;
//     for (ti = 0; ti < length; ti++)
//     {
//         list[ti + start] = sorted[ti];
//     }
//     for (val_ti = 0, ti = 0; ti < length; ti++, val_ti += (id + 1))
//     {
//         list[ti + start] = sorted[ti];
//         for (int k = 0; k < id + 1; k++)
//         {
//             val_list[k + val_ti + val_start] = val_sorted[val_ti + k];
//         }
//     }
//     free(sorted);
//     free(val_sorted);
// }

vector<idx_t> part(vector<idx_t> &xadj, vector<idx_t> &adjncy, /*vector<idx_t> &adjwgt, */ decltype(METIS_PartGraphKway) *METIS_PartGraphFunc)
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
    return r;
}

string cat_instr_1(Instr_1 &instr_1)
{
    string cat_instr_1;
    stringstream ss;
    ss << "00" << instr_1.LUT_Value << bitset<6>(toBinary(instr_1.Node_Addr[0])) << bitset<6>(toBinary(instr_1.Node_Addr[1])) << instr_1.LUT_Input_Select \
       << bitset<9>(toBinary(instr_1.Operand_Addr[0])) << bitset<9>(toBinary(instr_1.Operand_Addr[1])) << bitset<9>(toBinary(instr_1.Operand_Addr[2])) << bitset<9>(toBinary(instr_1.Operand_Addr[3])) \
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