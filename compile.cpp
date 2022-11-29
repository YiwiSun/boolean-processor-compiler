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
#include <iomanip>
#include <metis.h>

#include "compile.h"
#include "Parser.h"
#include "VCDParser.h"
#include "VCDTypes.h"
#include "ASAP.h"

using namespace std;


int main(int argc, char const *argv[])
{
    std::string v_path, vcdFilePath, instr_out, out_dir; 
    if (argc == 5)
    {
        v_path = argv[1];
        vcdFilePath = argv[2]; 
        instr_out = argv[3];
        out_dir = argv[4];
    }
    else
    {
        std::cout << "[USAGE] ./compile v_path vcd_path [instr_path] [outdir_path]" << endl;
        exit(-1);
    }

    auto start_total = std::chrono::steady_clock::now();

    /****************************************************************************************************/
    /*                                         netlist parsing                                          */
    /****************************************************************************************************/
    std::cout << "1. Executing netlist file parsing." << endl;
    auto start_v = std::chrono::steady_clock::now();

    Parser _parser;
    _parser.parse_v(v_path);

    auto end_v = std::chrono::steady_clock::now();
    long duration_v = std::chrono::duration_cast<std::chrono::milliseconds>(end_v - start_v).count();
    std::cout << "Successfully finished netlist file parsing. (" << duration_v << "ms) " << endl;
    std::cout << endl;

    map<int, LutType> luts = _parser.get_luts();
    map<int, DffType> dffs = _parser.get_dffs();
    map<string, TimedValues *> pinbitValues = _parser.get_pinbitValues();
    map<string, string> assign_pairs = _parser.get_assign_pairs();
    map<string, string> pin_bits = _parser.get_pin_bits();
    map<string, vector<int>> net_for_id = _parser.net_for_id;
    map<string, int> net_from_id = _parser.net_from_id;
    map<string, vector<int>> net_for_id_dff = _parser.net_for_id_dff;
    map<string, int> net_from_id_dff = _parser.net_from_id_dff;
    int n = luts.size();

    /****************************************************************************************************/
    /*                                           levelization                                           */
    /****************************************************************************************************/
    cout << "2. Executing levelization." << endl;
    auto start_ASAP = std::chrono::steady_clock::now();

    ASAP _asap;
    _asap.ASAP_Levelization(luts, net_for_id, n);

    auto end_ASAP = std::chrono::steady_clock::now();
    long duration_ASAP = std::chrono::duration_cast<std::chrono::milliseconds>(end_ASAP - start_ASAP).count();
    cout << "Successfully finished ASAP levelization. (" << duration_ASAP << "ms)" << endl;
    cout << endl;

    std::vector<std::vector<int>> levels = _asap.levels;
    vector<int> lut_level(luts.size());
    vector<int> lut_pos_at_level(luts.size());
    for (unsigned i = 0; i < levels.size(); i++)
    {
        vector<int> cur_level = levels[i];
        for (unsigned j = 0; j < cur_level.size(); j++)
        {
            int cur_id = cur_level[j];
            lut_level[cur_id] = i;
            lut_pos_at_level[cur_id] = j;
        }
    }

    /****************************************************************************************************/
    /*                                             partition                                            */
    /****************************************************************************************************/
    std::cout << "3. Executing graph partition." << endl;
    auto start_par = std::chrono::steady_clock::now();

    int edges = 0;
    vector<vector<int>> adjncys(luts.size() + dffs.size());
    auto total_size = luts.size() + dffs.size();
    for (unsigned i = 0; i < total_size; ++i)
    {
        if (i < luts.size())
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
                    adjncys[luts[i].num].push_back(_id);
                    adjncys[_id].push_back(luts[i].num);
                }
                else if (net_from_id_dff.find(cur_in) != net_from_id_dff.end())
                {
                    edges += 1;
                    int _id = net_from_id_dff[cur_in];
                    adjncys[luts[i].num].push_back(_id + luts.size());
                    adjncys[_id + luts.size()].push_back(luts[i].num);
                }
                else if (assign_pairs.find(cur_in) != assign_pairs.end())
                {
                    string temp = assign_pairs[cur_in];
                    if (net_from_id.find(temp) != net_from_id.end())
                    {
                        edges += 1;
                        int _id = net_from_id[temp];
                        adjncys[luts[i].num].push_back(_id);
                        adjncys[_id].push_back(luts[i].num);
                    }
                    else if (net_from_id_dff.find(temp) != net_from_id_dff.end())
                    {
                        edges += 1;
                        int _id = net_from_id_dff[temp];
                        adjncys[luts[i].num].push_back(_id + luts.size());
                        adjncys[_id + luts.size()].push_back(luts[i].num);
                    }
                }
            }
        }
        else
        {
            DffType cur_dff = dffs[i - luts.size()];
            std::vector<string> _in_net = cur_dff.dff_in_ports;
            for (unsigned j = 0; j < _in_net.size(); ++j)
            {
                string cur_in = _in_net[j];
                assert((net_from_id_dff.find(cur_in) == net_from_id_dff.end()));
                if (net_from_id.find(cur_in) != net_from_id.end())
                {
                    edges += 1;
                    int _id = net_from_id[cur_in];
                    adjncys[dffs[i - luts.size()].num + luts.size()].push_back(_id);
                    adjncys[_id].push_back(dffs[i - luts.size()].num + luts.size());
                }
                else if (assign_pairs.find(cur_in) != assign_pairs.end())
                {
                    string temp = assign_pairs[cur_in];
                    assert(net_from_id_dff.find(temp) == net_from_id_dff.end());
                    if (net_from_id.find(temp) != net_from_id.end())
                    {
                        edges += 1;
                        int _id = net_from_id[temp];
                        adjncys[dffs[i - luts.size()].num + luts.size()].push_back(_id);
                        adjncys[_id].push_back(dffs[i - luts.size()].num + luts.size());
                    }
                }
            }
        }
    }
    idx_t nVertices = luts.size() + dffs.size();
    idx_t nEdges = edges;
    vector<vector<idx_t>> nAdjncys = adjncys;

    // generation the adjacency structure of the graph
    vector<idx_t> xadj(0);
    vector<idx_t> adjncy(0); 
    for (int i = 0; i < nAdjncys.size(); i++)
    {
        xadj.push_back(adjncy.size()); // csr: row offsets (https://www.cnblogs.com/xbinworld/p/4273506.html)
        for (int j = 0; j < nAdjncys[i].size(); j++) 
        {
            adjncy.push_back(nAdjncys[i][j]); 
        } 
    }
    xadj.push_back(adjncy.size());
    assert(xadj.size() == (nVertices + 1));
    assert(adjncy.size() == (nEdges * 2));

    vector<idx_t> part = part_func(xadj, adjncy, METIS_PartGraphRecursive);
    // ofstream outpartition(par_out_path);
    // for (int i = 0; i < part.size(); i++)
    // {
    //     outpartition << i << " " << part[i] << endl;
    // }
    // outpartition.close();

    auto end_par = std::chrono::steady_clock::now();
    long duration_par = std::chrono::duration_cast<std::chrono::milliseconds>(end_par - start_par).count();
    std::cout << "Successfully finished graph partition. (" << duration_par << "ms)" << endl;
    std::cout << endl;

    /****************************************************************************************************/
    /*                                            vcd parsing                                           */
    /****************************************************************************************************/
    std::cout << "4. Executing vcd file parsing." << endl;
    auto start_vcd = std::chrono::steady_clock::now();

    VCDParser _vcdparser;               
    _vcdparser.parse(vcdFilePath);      
    
    auto end_vcd = std::chrono::steady_clock::now();
    long duration_vcd = std::chrono::duration_cast<std::chrono::milliseconds>(end_vcd - start_vcd).count();
    std::cout << "Successfully finished vcd file parsing. (" << duration_vcd << "ms)" << endl;
    std::cout << endl;

    std::unordered_map<std::string, std::vector<unsigned int> *> vcd_times = _vcdparser.times;
    std::unordered_map<std::string, std::vector<short> *> vcd_values = _vcdparser.values;

    /****************************************************************************************************/
    /*                                          pre-processing                                          */
    /****************************************************************************************************/
    std::cout << "5. Executing pre-processing." << endl;
    auto start_pre = std::chrono::steady_clock::now();

    int parts = (_parser.luts.size() + _parser.dffs.size() + N_PROCESSORS_PER_CLUSTER * N_LUTS_PER_PROCESSOR - 1) / (N_PROCESSORS_PER_CLUSTER * N_LUTS_PER_PROCESSOR); // part nums
    map<int, vector<int>> luts_in_part;     // partition : nodes(luts & dffs)
    if (parts == 1 && part[0] == 1) 
    {   
        for (int i = 0; i < part.size(); i++)
        {
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

    map<int, vector<vector<int>>> parts_levels;     // partition : node_levels
    for (int i = 0; i < parts; i++) {
        vector<int> cur_luts = luts_in_part[i];
        vector<vector<int>> cur_part_levels(levels.size());
        for (int j = 0; j < cur_luts.size(); j++) {
            if (cur_luts[j] < lut_level.size())
            {
                int cur_level = lut_level[cur_luts[j]];
                cur_part_levels[cur_level].push_back(cur_luts[j]);
            }                
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

    int initial_id = 0;
    std::map<std::string, int> initial_net_map;     // net : initial id
    for (int p = 0; p < parts; p++)
    {
        vector<int> cur_nodes = luts_in_part[p];
        for (int i = 0; i < cur_nodes.size(); i++)
        {
            if (cur_nodes[i] < luts.size())
            {
                int lut_num = cur_nodes[i];
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
                    else if (net_from_id_dff.find(cur_in) != net_from_id_dff.end())
                    {
                        int _id = net_from_id_dff[cur_in] + luts.size();
                        luts[lut_num].in_net_from_id.push_back(_id);
                        luts[lut_num].in_net_from_info.push_back(" ");
                        if (part[_id] == p)
                        {
                            luts[lut_num].in_net_from_part.push_back(-3);
                            luts[lut_num].in_net_from_level.push_back(-3);
                            luts[lut_num].in_net_from_pos_at_level.push_back(-3);
                        }
                        else
                        {
                            luts[lut_num].in_net_from_part.push_back(part[_id]);
                            luts[lut_num].in_net_from_level.push_back(-3);
                            luts[lut_num].in_net_from_pos_at_level.push_back(-3);
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
                        else if (net_from_id_dff.find(temp) != net_from_id_dff.end())
                        {
                            int _id = net_from_id_dff[temp] + luts.size();
                            luts[lut_num].in_net_from_id.push_back(_id);
                            luts[lut_num].in_net_from_info.push_back(" ");
                            if (part[_id] == p)
                            {
                                luts[lut_num].in_net_from_part.push_back(-3);
                                luts[lut_num].in_net_from_level.push_back(-3);
                                luts[lut_num].in_net_from_pos_at_level.push_back(-3);
                            }
                            else
                            {
                                luts[lut_num].in_net_from_part.push_back(part[_id]);
                                luts[lut_num].in_net_from_level.push_back(-3);
                                luts[lut_num].in_net_from_pos_at_level.push_back(-3);
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
            else
            {
                int dff_num = cur_nodes[i] - luts.size();
                DffType cur_dff = dffs[dff_num];
                std::vector<string> _in_net = cur_dff.dff_in_ports;
                for (unsigned j = 0; j < _in_net.size(); ++j)
                {
                    string cur_in = _in_net[j];
                    if (net_from_id.find(cur_in) != net_from_id.end())
                    {
                        int _id = net_from_id[cur_in];
                        dffs[dff_num].in_net_from_id.push_back(_id);
                        dffs[dff_num].in_net_from_info.push_back(" ");
                        if (part[_id] == p)
                        {
                            dffs[dff_num].in_net_from_part.push_back(-3);
                            dffs[dff_num].in_net_from_level.push_back(lut_level_in_part[_id]);
                            dffs[dff_num].in_net_from_pos_at_level.push_back(lut_pos_at_level_in_part[_id]);
                        }
                        else
                        {
                            dffs[dff_num].in_net_from_part.push_back(part[_id]);
                            dffs[dff_num].in_net_from_level.push_back(lut_level_in_part[_id]);
                            dffs[dff_num].in_net_from_pos_at_level.push_back(lut_pos_at_level_in_part[_id]);
                        }
                    }
                    else if (pinbitValues.find(cur_in) != pinbitValues.end())
                    {
                        TimedValues *tvs = pinbitValues[cur_in];
                        int cur_val = (*(tvs->begin())).value;
                        // at this case, 'in net from pos at level' means value
                        dffs[dff_num].in_net_from_id.push_back(-2);
                        dffs[dff_num].in_net_from_info.push_back(cur_in);
                        dffs[dff_num].in_net_from_part.push_back(-2);
                        dffs[dff_num].in_net_from_level.push_back(-2);
                        dffs[dff_num].in_net_from_pos_at_level.push_back(cur_val);
                    }
                    else if (assign_pairs.find(cur_in) != assign_pairs.end())
                    {
                        string temp = assign_pairs[cur_in];
                        if (net_from_id.find(temp) != net_from_id.end())
                        {
                            int _id = net_from_id[temp];
                            dffs[dff_num].in_net_from_id.push_back(_id);
                            dffs[dff_num].in_net_from_info.push_back(" ");
                            if (part[_id] == p)
                            {
                                dffs[dff_num].in_net_from_part.push_back(-3);
                                dffs[dff_num].in_net_from_level.push_back(lut_level_in_part[_id]);
                                dffs[dff_num].in_net_from_pos_at_level.push_back(lut_pos_at_level_in_part[_id]);
                            }
                            else
                            {
                                dffs[dff_num].in_net_from_part.push_back(part[_id]);
                                dffs[dff_num].in_net_from_level.push_back(lut_level_in_part[_id]);
                                dffs[dff_num].in_net_from_pos_at_level.push_back(lut_pos_at_level_in_part[_id]);
                            }
                        }
                        else if (pinbitValues.find(temp) != pinbitValues.end())
                        {
                            TimedValues *tvs = pinbitValues[temp];
                            int cur_val = (*(tvs->begin())).value;
                            // at this case, 'in net from pos at level' means value
                            dffs[dff_num].in_net_from_id.push_back(-2);
                            dffs[dff_num].in_net_from_info.push_back(temp);
                            dffs[dff_num].in_net_from_part.push_back(-2);
                            dffs[dff_num].in_net_from_level.push_back(-2);
                            dffs[dff_num].in_net_from_pos_at_level.push_back(cur_val);
                        }
                        else
                        {
                            if (initial_net_map.find(temp) == initial_net_map.end())
                            {
                                initial_net_map[temp] = initial_id;
                                initial_id++;
                            }
                            // at this case, 'in net from pos at level' means initial id
                            dffs[dff_num].in_net_from_id.push_back(-1);
                            dffs[dff_num].in_net_from_info.push_back(temp);
                            dffs[dff_num].in_net_from_part.push_back(-1);
                            dffs[dff_num].in_net_from_level.push_back(-1);
                            dffs[dff_num].in_net_from_pos_at_level.push_back(initial_net_map[temp]);
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
                        dffs[dff_num].in_net_from_id.push_back(-1);
                        dffs[dff_num].in_net_from_info.push_back(cur_in);
                        dffs[dff_num].in_net_from_part.push_back(-1);
                        dffs[dff_num].in_net_from_level.push_back(-1);
                        dffs[dff_num].in_net_from_pos_at_level.push_back(initial_net_map[cur_in]);
                    }
                }
            }
        }
    }

    auto end_pre = std::chrono::steady_clock::now();
    long duration_pre = std::chrono::duration_cast<std::chrono::milliseconds>(end_pre - start_pre).count();
    std::cout << "Successfully finished pre-processing. (" << duration_pre << "ms)" << endl;
    std::cout << endl;

    /****************************************************************************************************/
    /*                                            scheduling                                            */
    /****************************************************************************************************/
    std::cout << "6. Executing scheduling." << endl;
    auto start_sch = std::chrono::steady_clock::now();

    // equal distribution randomly
    vector<int> processor_id(parts, 0);
    map<int, vector<int>> processor_node;
    int n_luts_per_cluster = ((luts.size() + dffs.size()) + parts - 1) / parts;
    int used_processors_per_cluster = (n_luts_per_cluster + N_LUTS_PER_PROCESSOR  - 1) / N_LUTS_PER_PROCESSOR;
    for (auto l = levels.begin(); l != levels.end(); l++)
    {
        for (auto i = l->begin(); i != l->end(); i++)
        {
            assert(*i < luts.size());
            int cur_part = part[*i];
            luts[*i].node_addr.push_back(cur_part);
            luts[*i].node_addr.push_back(processor_id[cur_part]);
            processor_node[cur_part * N_PROCESSORS_PER_CLUSTER + processor_id[cur_part]].push_back(*i);
            if (processor_id[cur_part] == used_processors_per_cluster - 1)
            {
                processor_id[cur_part] = 0;
            }
            else
            {
                processor_id[cur_part] += 1;
            }
        }
    }
    for (auto d = dffs.begin(); d != dffs.end(); d++)
    {
        int cur_part = part[d->first + luts.size()];
        dffs[d->first].node_addr.push_back(cur_part);
        dffs[d->first].node_addr.push_back(processor_id[cur_part]);
        processor_node[cur_part * N_PROCESSORS_PER_CLUSTER + processor_id[cur_part]].push_back(d->first + luts.size());
        if (processor_id[cur_part] == used_processors_per_cluster - 1)
        {
            processor_id[cur_part] = 0;
        }
        else
        {
            processor_id[cur_part] += 1;
        }
    }

    for (auto i = processor_node.begin(); i != processor_node.end(); i++)
    {
        cout << "Processor: " << i->first << endl;
        for (auto j = i->second.begin(); j != i->second.end(); j++)
        {
            cout << *j << " ";
        }
        cout << endl;
    }

    // equal distribution randomly with unique interactive signal per BP
    // vector<int> outsig_in_part(parts, 0);
    // for (int i = 0; i < (luts.size() + dffs.size()); i++)
    // {
    //     if (i < luts.size())
    //     {
    //         if (pin_bits[luts[i].out_ports] == "output")
    //         {
    //             outsig_in_part[part[i]]++;
    //         }
    //     }
    //     else
    //     {
    //         if (pin_bits[dffs[i - luts.size()].dff_out] == "output")
    //         {
    //             outsig_in_part[part[i]]++;
    //         }
    //     }
    // }
    // vector<int> processor_id(parts, 0);
    // map<int, vector<int>> processor_node;
    // vector<int> interact_in_bp(parts * N_PROCESSORS_PER_CLUSTER, 0);
    // for (int i = 0; i < part.size(); i++)
    // {
    //     int cur_part = part[i];
    //     if (i < luts.size())
    //     {
    //         if (pin_bits[luts[i].out_ports] == "output")
    //         {
    //             luts[i].node_addr.push_back(cur_part);               
    //             while (interact_in_bp[cur_part * N_PROCESSORS_PER_CLUSTER + processor_id[cur_part]] == 1)
    //             {
    //                 if (processor_id[cur_part] == N_PROCESSORS_PER_CLUSTER - 1)
    //                 {
    //                     processor_id[cur_part] = 0;
    //                 }
    //                 else
    //                 {
    //                     processor_id[cur_part] += 1;
    //                 }
    //             }
    //             luts[i].node_addr.push_back(processor_id[cur_part]);
    //             interact_in_bp[cur_part * N_PROCESSORS_PER_CLUSTER + processor_id[cur_part]] = 1;                                  
    //         }
    //         else
    //         {
    //             luts[i].node_addr.push_back(cur_part);
    //             luts[i].node_addr.push_back(processor_id[cur_part]);
    //         }           
    //     }
    //     else
    //     {
    //         if (pin_bits[dffs[i - luts.size()].dff_out] == "output")
    //         {
    //             dffs[i - luts.size()].node_addr.push_back(cur_part);               
    //             while (interact_in_bp[cur_part * N_PROCESSORS_PER_CLUSTER + processor_id[cur_part]] == 1)
    //             {
    //                 if (processor_id[cur_part] == N_PROCESSORS_PER_CLUSTER - 1)
    //                 {
    //                     processor_id[cur_part] = 0;
    //                 }
    //                 else
    //                 {
    //                     processor_id[cur_part] += 1;
    //                 }
    //             }
    //             dffs[i - luts.size()].node_addr.push_back(processor_id[cur_part]);
    //             interact_in_bp[cur_part * N_PROCESSORS_PER_CLUSTER + processor_id[cur_part]] = 1;              
    //         }
    //         else
    //         {
    //             dffs[i - luts.size()].node_addr.push_back(cur_part);
    //             dffs[i - luts.size()].node_addr.push_back(processor_id[cur_part]);
    //         }           
    //     }
    //     processor_node[cur_part * N_PROCESSORS_PER_CLUSTER + processor_id[cur_part]].push_back(i);
    //     if (processor_id[cur_part] == N_PROCESSORS_PER_CLUSTER - 1)
    //     {
    //         processor_id[cur_part] = 0;
    //     }
    //     else
    //     {
    //         processor_id[cur_part] += 1;
    //     }
    // }
    // // debug
    // // for (int i = 0; i < luts.size(); i++)
    // // {
    // //     std::cout << "LUT: ";
    // //     std::cout.setf(ios::left);
    // //     std::cout.width(10);
    // //     std::cout << i;
    // //     std::cout << "Clutser: ";
    // //     std::cout.setf(ios::left);
    // //     std::cout.width(10);
    // //     std::cout << luts[i].node_addr[0];
    // //     std::cout << "Processor: ";
    // //     std::cout.setf(ios::left);
    // //     std::cout.width(10);
    // //     std::cout << luts[i].node_addr[1] << endl;
    // // }
    // for (auto i = processor_node.begin(); i != processor_node.end(); i++)
    // {
    //     cout << "Processor: " << i->first << endl;
    //     for (auto j = i->second.begin(); j != i->second.end(); j++)
    //     {
    //         cout << *j << " ";
    //     }
    //     cout << endl;
    // }

    auto end_sch = std::chrono::steady_clock::now();
    long duration_sch = std::chrono::duration_cast<std::chrono::milliseconds>(end_sch - start_sch).count();
    std::cout << "Successfully finished scheduling. (" << duration_sch << "ms)" << endl;
    std::cout << endl;

    /****************************************************************************************************/
    /*                                      instruction generation                                      */
    /****************************************************************************************************/
    std::cout << "7. Executing instruction generation." << endl;
    auto start_ins = std::chrono::steady_clock::now();

    // generating instructions for LUTs
    for (vector<vector<int>>::iterator l = levels.begin(); l != levels.end(); l++)
    {
        for (vector<int>::iterator i = l->begin(); i != l->end(); i++)
        {
            int lut_num = *i;
            LutType cur_lut = luts[*i];
            int cur_processor_id = N_PROCESSORS_PER_CLUSTER * cur_lut.node_addr[0] + cur_lut.node_addr[1];
            
            Instr_1 instr_1;
            instr_1.LUT_Value = cur_lut.lut_res;
            instr_1.Node_Addr = cur_lut.node_addr;
            instr_1.PC_Jump = 0;
            instr_1.BM_Jump = 0;
            vector<int> cur_in_net_from_id = cur_lut.in_net_from_id;
            vector<int> cur_in_net_from_part = cur_lut.in_net_from_part;
            for (int it = 0; it < cur_in_net_from_id.size();)
            {
                if (cur_in_net_from_id[it] == -1 || cur_in_net_from_id[it] == -2 || (cur_in_net_from_id[it] < luts.size() && luts[cur_in_net_from_id[it]].node_addr == cur_lut.node_addr) || (cur_in_net_from_id[it] >= luts.size()))
                {
                    cur_in_net_from_id.erase(cur_in_net_from_id.begin() + it);
                    cur_in_net_from_part.erase(cur_in_net_from_part.begin() + it);
                }
                else
                    it++;
            }
            vector<pair<int, int>> id_instr; // <cur_in_net_from_id, idealy first_get_res_addr>
            vector<int> _cur_in_net_from_id;
            vector<int> get_res_instr_pos;
            vector<int> instr_start_pos;
            vector<int> config_instr_pos;
            map<int, int> from_id_from_part; // <in_from_id, id_from_part>
            map<int, int> from_id_opr_addr;  // <in_from_id, operand_addr>
            if (cur_in_net_from_id.size() >= 1)
            {
                for (int in = 0; in < cur_in_net_from_id.size(); in++)
                {
                    LutType cur_in_from_lut = luts[cur_in_net_from_id[in]];
                    int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                    from_id_from_part.insert(pair<int, int>(cur_in_net_from_id[in], cur_in_net_from_part[in]));
                    if (processors[from_processor_id].id_outaddr.first == cur_in_net_from_id[in] && processors[from_processor_id].id_outaddr.second > 0 && processors[from_processor_id].id_outaddr.second < N_INS_PER_PROCESSOR)
                    {
                        int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                        id_instr.push_back(pair<int, int>(cur_in_net_from_id[in], first_get_res_addr));
                    }
                    else
                    {
                        Instr_2 instr_2;
                        instr_2.PC_Jump = 0;
                        instr_2.BM_Jump = 0;
                        instr_2.Node_Addr = cur_in_from_lut.node_addr;
                        instr_2.Data_Mem_Select = "0";
                        instr_2.Operand_Addr = cur_in_from_lut.res_pos_at_mem;
                        string lut_instr_2 = cat_instr_2(instr_2);
                        processors[from_processor_id].instr_mem.push_back(lut_instr_2);
                        processors[from_processor_id].id_outaddr = pair<int, int>(cur_in_net_from_id[in], int(processors[from_processor_id].instr_mem.size()) - 1);
                        int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                        id_instr.push_back(pair<int, int>(cur_in_net_from_id[in], first_get_res_addr));
                    }
                }
                sort(id_instr.begin(), id_instr.end(), cmp);
                for (vector<pair<int, int>>::iterator it = id_instr.begin(); it != id_instr.end(); it++)
                {
                    _cur_in_net_from_id.push_back(it->first);
                    get_res_instr_pos.push_back(it->second);
                    instr_start_pos.push_back(it->second);
                }
                if (instr_start_pos.size() >= 1)
                {
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
                }
                config_instr_pos = instr_start_pos;
                for (int i = 0; i < _cur_in_net_from_id.size(); i++)
                {
                    from_id_opr_addr.insert(pair<int, int>(_cur_in_net_from_id[i], instr_start_pos[i]));
                    if (from_id_from_part[_cur_in_net_from_id[i]] == -3) // current part
                        config_instr_pos[i] += (INTER_CLUSTER_CLK - 1);
                    else // other part
                        config_instr_pos[i] += (ACR_CLUSTER_CLK - 1);
                    LutType cur_in_from_lut = luts[_cur_in_net_from_id[i]];
                    int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                    int cur_id = _cur_in_net_from_id[i];
                    int cur_num = get_res_instr_pos[i];
                    int cur_pos = instr_start_pos[i];
                    int cur_lut_idle_num = cur_pos - (int(processors[cur_processor_id].instr_mem.size()) - 1);
                    int from_lut_idle_num = ((cur_pos - int(processors[from_processor_id].instr_mem.size()) + 1) > 0) ? (cur_pos - int(processors[from_processor_id].instr_mem.size()) + 1) : 0;
                    // current lut
                    Instr_4 instr_4;
                    instr_4.PC_Jump = 0;
                    instr_4.BM_Jump = 0;
                    instr_4.Node_Addr = cur_in_from_lut.node_addr;
                    string lut_instr_4 = cat_instr_4(instr_4);
                    processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), cur_lut_idle_num, lut_instr_4);
                    // in from lut
                    processors[from_processor_id].instr_mem.insert(processors[from_processor_id].instr_mem.end(), from_lut_idle_num, lut_instr_4);
                }
                auto max_config_addr = max_element(config_instr_pos.begin(), config_instr_pos.end());
                Instr_4 instr_4;
                instr_4.PC_Jump = 0;
                instr_4.BM_Jump = 0;
                instr_4.Node_Addr = cur_lut.node_addr;
                string lut_instr_4 = cat_instr_4(instr_4);
                processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), *max_config_addr - int(processors[cur_processor_id].instr_mem.size()), lut_instr_4);
            }
            
            // Data_Mem_Select && Operand_Addr
            for (int in = 0; in < cur_lut.in_ports.size(); in++)
            {
                if (cur_lut.in_net_from_id[in] == -1) // -1: in net from initial module
                {
                    if (vcd_values.find(cur_lut.in_net_from_info[in]) != vcd_values.end())
                    {
                        vector<short> *tvs_val = vcd_values[cur_lut.in_net_from_info[in]];
                        if (*(tvs_val->begin()) == 0)
                            instr_1.Data_Mem_Select.append("0");
                        else
                            instr_1.Data_Mem_Select.append("1");
                        // for input initial signal INITIAL_JUMP_SIG changing
                        if (cur_lut.in_net_from_info[in] == INITIAL_JUMP_SIG)
                            instr_1.Operand_Addr.push_back(INITIAL_JUMP_ADDR);
                        else
                            instr_1.Operand_Addr.push_back(MEM_DEPTH - 1);
                    }
                    else
                    {
                        std::cout << "ERROR: No initial info of signal " << cur_lut.in_net_from_info[in] << " (LUT " << lut_num << ")"
                                    << " in vcd file!" << endl;
                    }
                }
                else if (cur_lut.in_net_from_id[in] == -2) // -2: in net from assign pin bit
                {
                    if (cur_lut.in_net_from_pos_at_level[in] == 0)
                        instr_1.Data_Mem_Select.append("0");
                    else
                        instr_1.Data_Mem_Select.append("1");
                    instr_1.Operand_Addr.push_back(MEM_DEPTH - 1);
                }
                else if ((cur_lut.in_net_from_id[in] < luts.size()) && luts[cur_lut.in_net_from_id[in]].node_addr == cur_lut.node_addr)
                {
                    instr_1.Data_Mem_Select.append("0");
                    instr_1.Operand_Addr.push_back(luts[cur_lut.in_net_from_id[in]].res_pos_at_mem);
                }
                else if (cur_lut.in_net_from_id[in] >= luts.size())
                {
                    if (dffs[cur_lut.in_net_from_id[in] - luts.size()].node_addr == cur_lut.node_addr)
                        instr_1.Data_Mem_Select.append("0");
                    else
                        instr_1.Data_Mem_Select.append("1");
                    blank_addr ba;
                    ba.dff_num = cur_lut.in_net_from_id[in] - luts.size();
                    ba.LUT_Node_Addr = cur_lut.node_addr;
                    ba.LUT_Instr_Addr = processors[cur_processor_id].instr_mem.size();
                    int operand_addr_num = instr_1.Operand_Addr.size();
                    int input_num = cur_lut.in_ports.size();
                    int range_left = (35 - 9 * (4 - input_num)) - operand_addr_num * 9;
                    int range_right = (27 - 9 * (4 - input_num)) - operand_addr_num * 9;
                    ba.Replaced_Instr_Range = pair<int, int>(range_left, range_right);
                    blank_addrs[lut_num].push_back(ba);
                    instr_1.Operand_Addr.push_back(dffs[cur_lut.in_net_from_id[in] - luts.size()].res_pos_at_mem);
                }
                else
                {
                    instr_1.Data_Mem_Select.append("1");
                    auto it = from_id_opr_addr.find(cur_lut.in_net_from_id[in]);
                    instr_1.Operand_Addr.push_back(it->second);
                }
            }
            string lut_instr_1 = cat_instr_1(instr_1);
            processors[cur_processor_id].instr_mem.push_back(lut_instr_1);
            luts[lut_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
            processors[cur_processor_id].id_outaddr = pair<int, int>(lut_num, -1);           
        }
    }

    // instruction alignment with IDLE
    int longest_instr {0};
    for (auto it = processors.begin(); it != processors.end(); it++)
    {
        if (it->second.instr_mem.size() > longest_instr)
            longest_instr = it->second.instr_mem.size();
    }
    for (auto it = processors.begin(); it != processors.end(); it++)
    {
        if (it->second.instr_mem.size() < longest_instr)
        {
            Instr_4 instr_4;
            instr_4.PC_Jump = 0;
            instr_4.BM_Jump = 0;
            instr_4.Node_Addr.push_back(it->first / N_PROCESSORS_PER_CLUSTER);
            instr_4.Node_Addr.push_back(it->first % N_PROCESSORS_PER_CLUSTER);
            string lut_instr_4 = cat_instr_4(instr_4);
            processors[it->first].instr_mem.insert(processors[it->first].instr_mem.end(), (longest_instr - int(it->second.instr_mem.size())), lut_instr_4);
        }
    }
    for (auto it = processors.begin(); it != processors.end(); it++)
    {
        assert(int(it->second.instr_mem.size()) == longest_instr);
    }

    // generating instructions for DFFs
    for (auto d = dffs.begin(); d != dffs.end(); d++)
    {
        int dff_num = d->first;
        DffType cur_dff = dffs[dff_num];
        int cur_processor_id = N_PROCESSORS_PER_CLUSTER * cur_dff.node_addr[0] + cur_dff.node_addr[1];
        int ready_num = 0; // num of inputs from initial module, assign pinbit or same BP
        for (vector<int>::iterator infi = cur_dff.in_net_from_id.begin(); infi != cur_dff.in_net_from_id.end(); infi++)
        {
            assert(*infi < luts.size() || *infi == -1 || *infi == -2);
            if (*infi == -1 || *infi == -2)
                ready_num++;
            else if (luts[*infi].node_addr == cur_dff.node_addr)
                ready_num++;
            else
                continue;
        }
        // all inputs from initial module, assign pinbit or same BP
        if (ready_num == cur_dff.dff_in_ports.size())
        {
            if (cur_dff.type == 0)
            {
                assert(cur_dff.assignsig_condsig.size() == 1);
                if (cur_dff.assignsig_condsig.begin()->first == "0" || cur_dff.assignsig_condsig.begin()->first == "1")
                {
                    Instr_9 instr_9;
                    instr_9.PC_Jump = 0;
                    instr_9.BM_Jump = 0;
                    instr_9.Node_Addr = cur_dff.node_addr;
                    instr_9.im = atoi((cur_dff.assignsig_condsig.begin()->first).c_str());
                    string dff_instr = cat_instr_9(instr_9);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else
                {
                    Instr_10 instr_10;
                    instr_10.PC_Jump = 0;
                    instr_10.BM_Jump = 0;
                    instr_10.Node_Addr = cur_dff.node_addr; 
                    assert(cur_dff.in_net_from_id.size() == 1);
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_10.data_mem_select = 0;
                            else
                                instr_10.data_mem_select = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_10.Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_10.Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")" << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_10.data_mem_select = 0;
                        else
                            instr_10.data_mem_select = 1;
                        instr_10.Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_10.data_mem_select = 0;
                        instr_10.Addr = luts[*(cur_dff.in_net_from_id.begin())].res_pos_at_mem;
                    }                   
                    string dff_instr = cat_instr_10(instr_10);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
            }
            else if (cur_dff.type == 1)
            {
                assert(cur_dff.assignsig_condsig.size() == 1);
                if (cur_dff.assignsig_condsig.begin()->first == "0" || cur_dff.assignsig_condsig.begin()->first == "1")
                {
                    Instr_11 instr_11;
                    instr_11.PC_Jump = 0;
                    instr_11.BM_Jump = 0;
                    instr_11.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 1);
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_11.data_mem_select = 0;
                            else
                                instr_11.data_mem_select = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_11.Operand_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_11.Operand_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")" << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_11.data_mem_select = 0;
                        else
                            instr_11.data_mem_select = 1;
                        instr_11.Operand_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_11.data_mem_select = 0;
                        instr_11.Operand_Addr = luts[*(cur_dff.in_net_from_id.begin())].res_pos_at_mem;
                    }
                    instr_11.value = cur_dff.assignsig_condsig.begin()->second.second;
                    instr_11.T = atoi(cur_dff.assignsig_condsig.begin()->first.c_str());
                    string dff_instr = cat_instr_11(instr_11);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else
                {
                    Instr_12 instr_12;
                    instr_12.PC_Jump = 0;
                    instr_12.BM_Jump = 0;
                    instr_12.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 2);
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_12.data_mem_select1 = 0;
                            else
                                instr_12.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_12.Operand_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_12.Operand_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")" << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_12.data_mem_select1 = 0;
                        else
                            instr_12.data_mem_select1 = 1;
                        instr_12.Operand_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_12.data_mem_select1 = 0;
                        instr_12.Operand_Addr = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }                
                    instr_12.value = cur_dff.assignsig_condsig.begin()->second.second;
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_12.data_mem_select2 = 0;
                            else
                                instr_12.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_12.T_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_12.T_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")" << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_12.data_mem_select2 = 0;
                        else
                            instr_12.data_mem_select2 = 1;
                        instr_12.T_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_12.data_mem_select2 = 0;
                        instr_12.T_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    string dff_instr = cat_instr_12(instr_12);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
            }
            else if (cur_dff.type == 2)
            {
                assert(cur_dff.assignsig_condsig.size() == 2);
                if ((cur_dff.assignsig_condsig.begin()->first == "0" || cur_dff.assignsig_condsig.begin()->first == "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first == "0" || (cur_dff.assignsig_condsig.begin() + 1)->first == "1"))
                {
                    Instr_5 instr_5;
                    instr_5.PC_Jump = 0;
                    instr_5.BM_Jump = 0;
                    instr_5.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 1);
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_5.data_mem_select = 0;
                            else
                                instr_5.data_mem_select = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_5.Operand_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_5.Operand_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")" << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_5.data_mem_select = 0;
                        else
                            instr_5.data_mem_select = 1;
                        instr_5.Operand_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_5.data_mem_select = 0;
                        instr_5.Operand_Addr = luts[*(cur_dff.in_net_from_id.begin())].res_pos_at_mem;
                    }
                    instr_5.value = cur_dff.assignsig_condsig.begin()->second.second;
                    instr_5.T = stoi(cur_dff.assignsig_condsig.begin()->first.c_str());
                    instr_5.F = stoi((cur_dff.assignsig_condsig.begin() + 1)->first.c_str());
                    string dff_instr = cat_instr_5(instr_5);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first == "0" || cur_dff.assignsig_condsig.begin()->first == "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first != "0" && (cur_dff.assignsig_condsig.begin() + 1)->first != "1"))
                {
                    Instr_6 instr_6;
                    instr_6.PC_Jump = 0;
                    instr_6.BM_Jump = 0;
                    instr_6.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 2);
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_6.data_mem_select1 = 0;
                            else
                                instr_6.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_6.Operand_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_6.Operand_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_6.data_mem_select1 = 0;
                        else
                            instr_6.data_mem_select1 = 1;
                        instr_6.Operand_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_6.data_mem_select1 = 0;
                        instr_6.Operand_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    instr_6.value = cur_dff.assignsig_condsig.begin()->second.second;
                    instr_6.T = stoi(cur_dff.assignsig_condsig.begin()->first.c_str());
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_6.data_mem_select2 = 0;
                            else
                                instr_6.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_6.F_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_6.F_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_6.data_mem_select2 = 0;
                        else
                            instr_6.data_mem_select2 = 1;
                        instr_6.F_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_6.data_mem_select2 = 0;
                        instr_6.F_Addr = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    string dff_instr = cat_instr_6(instr_6);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first != "0" && cur_dff.assignsig_condsig.begin()->first != "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first != "0" && (cur_dff.assignsig_condsig.begin() + 1)->first != "1"))
                {
                    Instr_7 instr_7;
                    instr_7.PC_Jump = 0;
                    instr_7.BM_Jump = 0;
                    instr_7.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 3);
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_7.data_mem_select1 = 0;
                            else
                                instr_7.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_7.Operand_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_7.Operand_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_7.data_mem_select1 = 0;
                        else
                            instr_7.data_mem_select1 = 1;
                        instr_7.Operand_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_7.data_mem_select1 = 0;
                        instr_7.Operand_Addr = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    instr_7.value = cur_dff.assignsig_condsig.begin()->second.second;
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_7.data_mem_select2 = 0;
                            else
                                instr_7.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_7.T_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_7.T_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_7.data_mem_select2 = 0;
                        else
                            instr_7.data_mem_select2 = 1;
                        instr_7.T_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_7.data_mem_select2 = 0;
                        instr_7.T_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    if (cur_dff.in_net_from_id[2] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[2]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[2]];
                            if (*(tvs_val->begin()) == 0)
                                instr_7.data_mem_select3 = 0;
                            else
                                instr_7.data_mem_select3 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[2] == INITIAL_JUMP_SIG)
                                instr_7.F_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_7.F_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[2] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[2] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[2] == 0)
                            instr_7.data_mem_select3 = 0;
                        else
                            instr_7.data_mem_select3 = 1;
                        instr_7.F_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_7.data_mem_select3 = 0;
                        instr_7.F_Addr = luts[cur_dff.in_net_from_id[2]].res_pos_at_mem;
                    }
                    string dff_instr = cat_instr_7(instr_7);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first != "0" && cur_dff.assignsig_condsig.begin()->first != "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first == "0" || (cur_dff.assignsig_condsig.begin() + 1)->first == "1"))
                {
                    Instr_8 instr_8;
                    instr_8.PC_Jump = 0;
                    instr_8.BM_Jump = 0;
                    instr_8.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 2);
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_8.data_mem_select1 = 0;
                            else
                                instr_8.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_8.Operand_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_8.Operand_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_8.data_mem_select1 = 0;
                        else
                            instr_8.data_mem_select1 = 1;
                        instr_8.Operand_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_8.data_mem_select1 = 0;
                        instr_8.Operand_Addr = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    instr_8.value = cur_dff.assignsig_condsig.begin()->second.second;
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_8.data_mem_select2 = 0;
                            else
                                instr_8.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_8.T_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_8.T_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_8.data_mem_select2 = 0;
                        else
                            instr_8.data_mem_select2 = 1;
                        instr_8.T_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_8.data_mem_select2 = 0;
                        instr_8.T_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    instr_8.F = atoi((cur_dff.assignsig_condsig.begin() + 1)->first.c_str());
                    string dff_instr = cat_instr_8(instr_8);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
            }
            else if (cur_dff.type == 3)
            {
                assert(cur_dff.assignsig_condsig.size() == 2);
                if ((cur_dff.assignsig_condsig.begin()->first == "0" || cur_dff.assignsig_condsig.begin()->first == "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first == "0" || (cur_dff.assignsig_condsig.begin() + 1)->first == "1"))
                {
                    Instr_13 instr_13;
                    instr_13.PC_Jump = 0;
                    instr_13.BM_Jump = 0;
                    instr_13.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 2);
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_13.data_mem_select1 = 0;
                            else
                                instr_13.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_13.Operand_Addr1 = INITIAL_JUMP_ADDR;
                            else
                                instr_13.Operand_Addr1 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_13.data_mem_select1 = 0;
                        else
                            instr_13.data_mem_select1 = 1;
                        instr_13.Operand_Addr1 = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_13.data_mem_select1 = 0;
                        instr_13.Operand_Addr1 = luts[*(cur_dff.in_net_from_id.begin())].res_pos_at_mem;
                    }
                    instr_13.value1 = cur_dff.assignsig_condsig.begin()->second.second;
                    instr_13.T1 = stoi(cur_dff.assignsig_condsig.begin()->first.c_str());
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_13.data_mem_select2 = 0;
                            else
                                instr_13.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_13.Operand_Addr2 = INITIAL_JUMP_ADDR;
                            else
                                instr_13.Operand_Addr2 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_13.data_mem_select2 = 0;
                        else
                            instr_13.data_mem_select2 = 1;
                        instr_13.Operand_Addr2 = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_13.data_mem_select2 = 0;
                        instr_13.Operand_Addr2 = luts[*(cur_dff.in_net_from_id.begin() + 1)].res_pos_at_mem;
                    }
                    instr_13.value2 = (cur_dff.assignsig_condsig.begin() + 1)->second.second;
                    instr_13.T2 = stoi((cur_dff.assignsig_condsig.begin() + 1)->first.c_str());
                    string dff_instr = cat_instr_13(instr_13);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first == "0" || cur_dff.assignsig_condsig.begin()->first == "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first != "0" && (cur_dff.assignsig_condsig.begin() + 1)->first != "1"))
                {
                    Instr_14 instr_14;
                    instr_14.PC_Jump = 0;
                    instr_14.BM_Jump = 0;
                    instr_14.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 3);
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_14.data_mem_select1 = 0;
                            else
                                instr_14.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_14.Operand_Addr1 = INITIAL_JUMP_ADDR;
                            else
                                instr_14.Operand_Addr1 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_14.data_mem_select1 = 0;
                        else
                            instr_14.data_mem_select1 = 1;
                        instr_14.Operand_Addr1 = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_14.data_mem_select1 = 0;
                        instr_14.Operand_Addr1 = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    instr_14.value1 = cur_dff.assignsig_condsig.begin()->second.second;
                    instr_14.T1 = stoi(cur_dff.assignsig_condsig.begin()->first.c_str());
                    if (cur_dff.in_net_from_id[2] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[2]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[2]];
                            if (*(tvs_val->begin()) == 0)
                                instr_14.data_mem_select2 = 0;
                            else
                                instr_14.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[2] == INITIAL_JUMP_SIG)
                                instr_14.Operand_Addr2 = INITIAL_JUMP_ADDR;
                            else
                                instr_14.Operand_Addr2 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[2] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[2] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[2] == 0)
                            instr_14.data_mem_select2 = 0;
                        else
                            instr_14.data_mem_select2 = 1;
                        instr_14.Operand_Addr2 = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_14.data_mem_select2 = 0;
                        instr_14.Operand_Addr2 = luts[cur_dff.in_net_from_id[2]].res_pos_at_mem;
                    }
                    instr_14.value2 = (cur_dff.assignsig_condsig.begin() + 1)->second.second;
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_14.data_mem_select3 = 0;
                            else
                                instr_14.data_mem_select3 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_14.T2_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_14.T2_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_14.data_mem_select3 = 0;
                        else
                            instr_14.data_mem_select3 = 1;
                        instr_14.T2_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_14.data_mem_select3 = 0;
                        instr_14.T2_Addr = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    string dff_instr = cat_instr_14(instr_14);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first != "0" && cur_dff.assignsig_condsig.begin()->first != "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first == "0" || (cur_dff.assignsig_condsig.begin() + 1)->first == "1"))
                {
                    Instr_15 instr_15;
                    instr_15.PC_Jump = 0;
                    instr_15.BM_Jump = 0;
                    instr_15.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 3);
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_15.data_mem_select1 = 0;
                            else
                                instr_15.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_15.Operand_Addr1 = INITIAL_JUMP_ADDR;
                            else
                                instr_15.Operand_Addr1 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_15.data_mem_select1 = 0;
                        else
                            instr_15.data_mem_select1 = 1;
                        instr_15.Operand_Addr1 = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_15.data_mem_select1 = 0;
                        instr_15.Operand_Addr1 = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    instr_15.value1 = cur_dff.assignsig_condsig.begin()->second.second;
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_15.data_mem_select2 = 0;
                            else
                                instr_15.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_15.T1_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_15.T1_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_15.data_mem_select2 = 0;
                        else
                            instr_15.data_mem_select2 = 1;
                        instr_15.T1_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_15.data_mem_select2 = 0;
                        instr_15.T1_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    if (cur_dff.in_net_from_id[2] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[2]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[2]];
                            if (*(tvs_val->begin()) == 0)
                                instr_15.data_mem_select3 = 0;
                            else
                                instr_15.data_mem_select3 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[2] == INITIAL_JUMP_SIG)
                                instr_15.Operand_Addr2 = INITIAL_JUMP_ADDR;
                            else
                                instr_15.Operand_Addr2 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[2] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[2] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[2] == 0)
                            instr_15.data_mem_select3 = 0;
                        else
                            instr_15.data_mem_select3 = 1;
                        instr_15.Operand_Addr2 = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_15.data_mem_select3 = 0;
                        instr_15.Operand_Addr2 = luts[cur_dff.in_net_from_id[2]].res_pos_at_mem;
                    }
                    instr_15.value2 = (cur_dff.assignsig_condsig.begin() + 1)->second.second;
                    instr_15.T2 = stoi((cur_dff.assignsig_condsig.begin() + 1)->first.c_str());
                    string dff_instr = cat_instr_15(instr_15);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first != "0" && cur_dff.assignsig_condsig.begin()->first != "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first != "0" && (cur_dff.assignsig_condsig.begin() + 1)->first != "1"))
                {
                    Instr_16 instr_16;
                    instr_16.PC_Jump = 0;
                    instr_16.BM_Jump = 0;
                    instr_16.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 4);
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_16.data_mem_select1 = 0;
                            else
                                instr_16.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_16.Operand_Addr1 = INITIAL_JUMP_ADDR;
                            else
                                instr_16.Operand_Addr1 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_16.data_mem_select1 = 0;
                        else
                            instr_16.data_mem_select1 = 1;
                        instr_16.Operand_Addr1 = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_16.data_mem_select1 = 0;
                        instr_16.Operand_Addr1 = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    instr_16.value1 = cur_dff.assignsig_condsig.begin()->second.second;
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_16.data_mem_select2 = 0;
                            else
                                instr_16.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_16.T1_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_16.T1_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_16.data_mem_select2 = 0;
                        else
                            instr_16.data_mem_select2 = 1;
                        instr_16.T1_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_16.data_mem_select2 = 0;
                        instr_16.T1_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    if (cur_dff.in_net_from_id[3] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[3]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[3]];
                            if (*(tvs_val->begin()) == 0)
                                instr_16.data_mem_select3 = 0;
                            else
                                instr_16.data_mem_select3 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[3] == INITIAL_JUMP_SIG)
                                instr_16.Operand_Addr2 = INITIAL_JUMP_ADDR;
                            else
                                instr_16.Operand_Addr2 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[3] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[3] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[3] == 0)
                            instr_16.data_mem_select3 = 0;
                        else
                            instr_16.data_mem_select3 = 1;
                        instr_16.Operand_Addr2 = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_16.data_mem_select3 = 0;
                        instr_16.Operand_Addr2 = luts[cur_dff.in_net_from_id[3]].res_pos_at_mem;
                    }
                    instr_16.value2 = (cur_dff.assignsig_condsig.begin() + 1)->second.second;
                    if (cur_dff.in_net_from_id[2] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[2]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[2]];
                            if (*(tvs_val->begin()) == 0)
                                instr_16.data_mem_select4 = 0;
                            else
                                instr_16.data_mem_select4 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[2] == INITIAL_JUMP_SIG)
                                instr_16.T2_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_16.T2_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[2] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[2] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[2] == 0)
                            instr_16.data_mem_select4 = 0;
                        else
                            instr_16.data_mem_select4 = 1;
                        instr_16.T2_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_16.data_mem_select4 = 0;
                        instr_16.T2_Addr = luts[cur_dff.in_net_from_id[2]].res_pos_at_mem;
                    }
                    string dff_instr = cat_instr_16(instr_16);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
            }
        }
        // inputs from other bps (clusters)
        else
        {
            if (cur_dff.type == 0)
            {
                assert(cur_dff.assignsig_condsig.size() == 1);
                assert(cur_dff.assignsig_condsig.begin()->first != "0" && cur_dff.assignsig_condsig.begin()->first != "1");
                assert(cur_dff.in_net_from_id.size() == 1);
                vector<int> in_net_from_id = cur_dff.in_net_from_id;
                vector<pair<int, int>> id_instr; // <cur_in_net_from_id, idealy first_get_res_addr>
                LutType cur_in_from_lut = luts[in_net_from_id[0]];
                int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                // from_id_from_part.insert(pair<int, int>(in_net_from_id[0], cur_in_net_from_part[in]));
                if (processors[from_processor_id].id_outaddr.first == in_net_from_id[0] && processors[from_processor_id].id_outaddr.second > 0 && processors[from_processor_id].id_outaddr.second < N_INS_PER_PROCESSOR)
                {
                    int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                    id_instr.push_back(pair<int, int>(in_net_from_id[0], first_get_res_addr));
                }
                else
                {
                    Instr_2 instr_2;
                    instr_2.PC_Jump = 0;
                    instr_2.BM_Jump = 0;
                    instr_2.Node_Addr = cur_in_from_lut.node_addr;
                    instr_2.Data_Mem_Select = "0";
                    instr_2.Operand_Addr = cur_in_from_lut.res_pos_at_mem;
                    string lut_instr_2 = cat_instr_2(instr_2);
                    processors[from_processor_id].instr_mem.push_back(lut_instr_2);
                    processors[from_processor_id].id_outaddr = pair<int, int>(in_net_from_id[0], int(processors[from_processor_id].instr_mem.size()) - 1);
                    int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                    id_instr.push_back(pair<int, int>(in_net_from_id[0], first_get_res_addr));
                }
                int config_instr_pos = id_instr.begin()->second;
                if (cur_dff.in_net_from_part[0] == -3) // current part
                    config_instr_pos += (INTER_CLUSTER_CLK - 1);
                else                                   // other part
                    config_instr_pos += (ACR_CLUSTER_CLK - 1);
                int cur_lut_idle_num = id_instr.begin()->second - (int(processors[cur_processor_id].instr_mem.size()) - 1);
                int from_lut_idle_num = ((id_instr.begin()->second - int(processors[from_processor_id].instr_mem.size()) + 1) > 0) ? (id_instr.begin()->second - int(processors[from_processor_id].instr_mem.size()) + 1) : 0;
                // current lut
                Instr_4 instr_4;
                instr_4.PC_Jump = 0;
                instr_4.BM_Jump = 0;
                instr_4.Node_Addr = cur_in_from_lut.node_addr;
                string lut_instr_4 = cat_instr_4(instr_4);
                processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), cur_lut_idle_num, lut_instr_4);
                // in from lut
                processors[from_processor_id].instr_mem.insert(processors[from_processor_id].instr_mem.end(), from_lut_idle_num, lut_instr_4);

                Instr_4 _instr_4;
                _instr_4.PC_Jump = 0;
                _instr_4.BM_Jump = 0;
                _instr_4.Node_Addr = cur_dff.node_addr;
                string _lut_instr_4 = cat_instr_4(_instr_4);
                processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), config_instr_pos - int(processors[cur_processor_id].instr_mem.size()), _lut_instr_4);
                
                Instr_10 instr_10;
                instr_10.PC_Jump = 0;
                instr_10.BM_Jump = 0;
                instr_10.Node_Addr = cur_dff.node_addr;
                instr_10.data_mem_select = 1;
                instr_10.Addr = id_instr.begin()->second;
                string dff_instr = cat_instr_10(instr_10);
                processors[cur_processor_id].instr_mem.push_back(dff_instr);
                dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
            }
            else if (cur_dff.type == 1)
            {
                assert(cur_dff.assignsig_condsig.size() == 1);
                if (cur_dff.assignsig_condsig.begin()->first == "0" || cur_dff.assignsig_condsig.begin()->first == "1")
                {
                    assert(cur_dff.in_net_from_id.size() == 1);
                    vector<int> in_net_from_id = cur_dff.in_net_from_id;
                    vector<pair<int, int>> id_instr; // <cur_in_net_from_id, idealy first_get_res_addr>
                    LutType cur_in_from_lut = luts[in_net_from_id[0]];
                    int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                    // from_id_from_part.insert(pair<int, int>(in_net_from_id[0], cur_in_net_from_part[in]));
                    if (processors[from_processor_id].id_outaddr.first == in_net_from_id[0] && processors[from_processor_id].id_outaddr.second > 0 && processors[from_processor_id].id_outaddr.second < N_INS_PER_PROCESSOR)
                    {
                        int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                        id_instr.push_back(pair<int, int>(in_net_from_id[0], first_get_res_addr));
                    }
                    else
                    {
                        Instr_2 instr_2;
                        instr_2.PC_Jump = 0;
                        instr_2.BM_Jump = 0;
                        instr_2.Node_Addr = cur_in_from_lut.node_addr;
                        instr_2.Data_Mem_Select = "0";
                        instr_2.Operand_Addr = cur_in_from_lut.res_pos_at_mem;
                        string lut_instr_2 = cat_instr_2(instr_2);
                        processors[from_processor_id].instr_mem.push_back(lut_instr_2);
                        processors[from_processor_id].id_outaddr = pair<int, int>(in_net_from_id[0], int(processors[from_processor_id].instr_mem.size()) - 1);
                        int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                        id_instr.push_back(pair<int, int>(in_net_from_id[0], first_get_res_addr));
                    }
                    int config_instr_pos = id_instr.begin()->second;
                    if (cur_dff.in_net_from_part[0] == -3) // current part
                        config_instr_pos += (INTER_CLUSTER_CLK - 1);
                    else                                   // other part
                        config_instr_pos += (ACR_CLUSTER_CLK - 1);
                    int cur_lut_idle_num = id_instr.begin()->second - (int(processors[cur_processor_id].instr_mem.size()) - 1);
                    int from_lut_idle_num = ((id_instr.begin()->second - int(processors[from_processor_id].instr_mem.size()) + 1) > 0) ? (id_instr.begin()->second - int(processors[from_processor_id].instr_mem.size()) + 1) : 0;
                    // current dff
                    Instr_4 instr_4;
                    instr_4.PC_Jump = 0;
                    instr_4.BM_Jump = 0;
                    instr_4.Node_Addr = cur_in_from_lut.node_addr;
                    string lut_instr_4 = cat_instr_4(instr_4);
                    processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), cur_lut_idle_num, lut_instr_4);
                    // in from lut
                    processors[from_processor_id].instr_mem.insert(processors[from_processor_id].instr_mem.end(), from_lut_idle_num, lut_instr_4);
                    Instr_4 _instr_4;
                    _instr_4.PC_Jump = 0;
                    _instr_4.BM_Jump = 0;
                    _instr_4.Node_Addr = cur_dff.node_addr;
                    string _lut_instr_4 = cat_instr_4(_instr_4);
                    processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), config_instr_pos - int(processors[cur_processor_id].instr_mem.size()), _lut_instr_4);

                    Instr_11 instr_11;
                    instr_11.PC_Jump = 0;
                    instr_11.BM_Jump = 0;
                    instr_11.Node_Addr = cur_dff.node_addr;
                    instr_11.data_mem_select = 1;
                    instr_11.Operand_Addr = id_instr.begin()->second;
                    instr_11.value = cur_dff.assignsig_condsig.begin()->second.second;
                    instr_11.T = atoi(cur_dff.assignsig_condsig.begin()->first.c_str());
                    string dff_instr = cat_instr_11(instr_11);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else
                {
                    vector<int> cur_in_net_from_id = cur_dff.in_net_from_id;
                    vector<int> cur_in_net_from_part = cur_dff.in_net_from_part;
                    for (int it = 0; it < cur_in_net_from_id.size();)
                    {
                        if (cur_in_net_from_id[it] == -1 || cur_in_net_from_id[it] == -2 || luts[cur_in_net_from_id[it]].node_addr == cur_dff.node_addr)
                        {
                            cur_in_net_from_id.erase(cur_in_net_from_id.begin() + it);
                            cur_in_net_from_part.erase(cur_in_net_from_part.begin() + it);
                        }
                        else
                            it++;
                    }
                    vector<pair<int, int>> id_instr; // <cur_in_net_from_id, idealy first_get_res_addr>
                    vector<int> _cur_in_net_from_id;
                    vector<int> get_res_instr_pos;
                    vector<int> instr_start_pos;
                    vector<int> config_instr_pos;
                    map<int, int> from_id_from_part; // <in_from_id, id_from_part>
                    map<int, int> from_id_opr_addr;  // <in_from_id, operand_addr>
                    for (int in = 0; in < cur_in_net_from_id.size(); in++)
                    {
                        LutType cur_in_from_lut = luts[cur_in_net_from_id[in]];
                        int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                        from_id_from_part.insert(pair<int, int>(cur_in_net_from_id[in], cur_in_net_from_part[in]));
                        if (processors[from_processor_id].id_outaddr.first == cur_in_net_from_id[in] && processors[from_processor_id].id_outaddr.second > 0 && processors[from_processor_id].id_outaddr.second < N_INS_PER_PROCESSOR)
                        {
                            int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                            id_instr.push_back(pair<int, int>(cur_in_net_from_id[in], first_get_res_addr));
                        }
                        else
                        {
                            Instr_2 instr_2;
                            instr_2.PC_Jump = 0;
                            instr_2.BM_Jump = 0;
                            instr_2.Node_Addr = cur_in_from_lut.node_addr;
                            instr_2.Data_Mem_Select = "0";
                            instr_2.Operand_Addr = cur_in_from_lut.res_pos_at_mem;
                            string lut_instr_2 = cat_instr_2(instr_2);
                            processors[from_processor_id].instr_mem.push_back(lut_instr_2);
                            processors[from_processor_id].id_outaddr = pair<int, int>(cur_in_net_from_id[in], int(processors[from_processor_id].instr_mem.size()) - 1);
                            int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                            id_instr.push_back(pair<int, int>(cur_in_net_from_id[in], first_get_res_addr));
                        }
                    }
                    sort(id_instr.begin(), id_instr.end(), cmp);
                    for (vector<pair<int, int>>::iterator it = id_instr.begin(); it != id_instr.end(); it++)
                    {
                        _cur_in_net_from_id.push_back(it->first);
                        get_res_instr_pos.push_back(it->second);
                        instr_start_pos.push_back(it->second);
                    }
                    if (instr_start_pos.size() >= 1)
                    {
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
                    }
                    config_instr_pos = instr_start_pos;
                    for (int i = 0; i < _cur_in_net_from_id.size(); i++)
                    {
                        from_id_opr_addr.insert(pair<int, int>(_cur_in_net_from_id[i], instr_start_pos[i]));
                        if (from_id_from_part[_cur_in_net_from_id[i]] == -3) // current part
                            config_instr_pos[i] += (INTER_CLUSTER_CLK - 1);
                        else // other part
                            config_instr_pos[i] += (ACR_CLUSTER_CLK - 1);
                        LutType cur_in_from_lut = luts[_cur_in_net_from_id[i]];
                        int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                        int cur_id = _cur_in_net_from_id[i];
                        int cur_num = get_res_instr_pos[i];
                        int cur_pos = instr_start_pos[i];
                        int cur_lut_idle_num = cur_pos - (int(processors[cur_processor_id].instr_mem.size()) - 1);
                        int from_lut_idle_num = ((cur_pos - int(processors[from_processor_id].instr_mem.size()) + 1) > 0) ? (cur_pos - int(processors[from_processor_id].instr_mem.size()) + 1) : 0;
                        // current dff
                        Instr_4 instr_4;
                        instr_4.PC_Jump = 0;
                        instr_4.BM_Jump = 0;
                        instr_4.Node_Addr = cur_in_from_lut.node_addr;
                        string lut_instr_4 = cat_instr_4(instr_4);
                        processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), cur_lut_idle_num, lut_instr_4);
                        // in from lut
                        processors[from_processor_id].instr_mem.insert(processors[from_processor_id].instr_mem.end(), from_lut_idle_num, lut_instr_4);
                    }
                    auto max_config_addr = max_element(config_instr_pos.begin(), config_instr_pos.end());
                    Instr_4 instr_4;
                    instr_4.PC_Jump = 0;
                    instr_4.BM_Jump = 0;
                    instr_4.Node_Addr = cur_dff.node_addr;
                    string lut_instr_4 = cat_instr_4(instr_4);
                    processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), *max_config_addr - int(processors[cur_processor_id].instr_mem.size()), lut_instr_4);
                    
                    Instr_12 instr_12;
                    instr_12.PC_Jump = 0;
                    instr_12.BM_Jump = 0;
                    instr_12.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 2);
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_12.data_mem_select1 = 0;
                            else
                                instr_12.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_12.Operand_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_12.Operand_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_12.data_mem_select1 = 0;
                        else
                            instr_12.data_mem_select1 = 1;
                        instr_12.Operand_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[1]].node_addr == cur_dff.node_addr)
                    {
                        instr_12.data_mem_select1 = 0;
                        instr_12.Operand_Addr = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_12.data_mem_select1 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[1]);
                        instr_12.Operand_Addr = it->second;
                    }
                    instr_12.value = cur_dff.assignsig_condsig.begin()->second.second;
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_12.data_mem_select2 = 0;
                            else
                                instr_12.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_12.T_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_12.T_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_12.data_mem_select2 = 0;
                        else
                            instr_12.data_mem_select2 = 1;
                        instr_12.T_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[0]].node_addr == cur_dff.node_addr)
                    {
                        instr_12.data_mem_select2 = 0;
                        instr_12.T_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_12.data_mem_select2 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[0]);
                        instr_12.T_Addr = it->second;
                    }
                    string dff_instr = cat_instr_12(instr_12);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
            }
            else if (cur_dff.type == 2)
            {
                assert(cur_dff.assignsig_condsig.size() == 2);
                vector<int> cur_in_net_from_id = cur_dff.in_net_from_id;
                vector<int> cur_in_net_from_part = cur_dff.in_net_from_part;
                for (int it = 0; it < cur_in_net_from_id.size();)
                {
                    if (cur_in_net_from_id[it] == -1 || cur_in_net_from_id[it] == -2 || luts[cur_in_net_from_id[it]].node_addr == cur_dff.node_addr)
                    {
                        cur_in_net_from_id.erase(cur_in_net_from_id.begin() + it);
                        cur_in_net_from_part.erase(cur_in_net_from_part.begin() + it);
                    }
                    else
                        it++;
                }
                vector<pair<int, int>> id_instr; // <cur_in_net_from_id, idealy first_get_res_addr>
                vector<int> _cur_in_net_from_id;
                vector<int> get_res_instr_pos;
                vector<int> instr_start_pos;
                vector<int> config_instr_pos;
                map<int, int> from_id_from_part; // <in_from_id, id_from_part>
                map<int, int> from_id_opr_addr;  // <in_from_id, operand_addr>
                for (int in = 0; in < cur_in_net_from_id.size(); in++)
                {
                    LutType cur_in_from_lut = luts[cur_in_net_from_id[in]];
                    int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                    from_id_from_part.insert(pair<int, int>(cur_in_net_from_id[in], cur_in_net_from_part[in]));
                    if (processors[from_processor_id].id_outaddr.first == cur_in_net_from_id[in] && processors[from_processor_id].id_outaddr.second > 0 && processors[from_processor_id].id_outaddr.second < N_INS_PER_PROCESSOR)
                    {
                        int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                        id_instr.push_back(pair<int, int>(cur_in_net_from_id[in], first_get_res_addr));
                    }
                    else
                    {
                        Instr_2 instr_2;
                        instr_2.PC_Jump = 0;
                        instr_2.BM_Jump = 0;
                        instr_2.Node_Addr = cur_in_from_lut.node_addr;
                        instr_2.Data_Mem_Select = "0";
                        instr_2.Operand_Addr = cur_in_from_lut.res_pos_at_mem;
                        string lut_instr_2 = cat_instr_2(instr_2);
                        processors[from_processor_id].instr_mem.push_back(lut_instr_2);
                        processors[from_processor_id].id_outaddr = pair<int, int>(cur_in_net_from_id[in], int(processors[from_processor_id].instr_mem.size()) - 1);
                        int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                        id_instr.push_back(pair<int, int>(cur_in_net_from_id[in], first_get_res_addr));
                    }
                }
                sort(id_instr.begin(), id_instr.end(), cmp);
                for (vector<pair<int, int>>::iterator it = id_instr.begin(); it != id_instr.end(); it++)
                {
                    _cur_in_net_from_id.push_back(it->first);
                    get_res_instr_pos.push_back(it->second);
                    instr_start_pos.push_back(it->second);
                }
                if (instr_start_pos.size() >= 1)
                {
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
                }
                config_instr_pos = instr_start_pos;
                for (int i = 0; i < _cur_in_net_from_id.size(); i++)
                {
                    from_id_opr_addr.insert(pair<int, int>(_cur_in_net_from_id[i], instr_start_pos[i]));
                    if (from_id_from_part[_cur_in_net_from_id[i]] == -3) // current part
                        config_instr_pos[i] += (INTER_CLUSTER_CLK - 1);
                    else // other part
                        config_instr_pos[i] += (ACR_CLUSTER_CLK - 1);
                    LutType cur_in_from_lut = luts[_cur_in_net_from_id[i]];
                    int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                    int cur_id = _cur_in_net_from_id[i];
                    int cur_num = get_res_instr_pos[i];
                    int cur_pos = instr_start_pos[i];
                    int cur_lut_idle_num = cur_pos - (int(processors[cur_processor_id].instr_mem.size()) - 1);
                    int from_lut_idle_num = ((cur_pos - int(processors[from_processor_id].instr_mem.size()) + 1) > 0) ? (cur_pos - int(processors[from_processor_id].instr_mem.size()) + 1) : 0;
                    // current dff
                    Instr_4 instr_4;
                    instr_4.PC_Jump = 0;
                    instr_4.BM_Jump = 0;
                    instr_4.Node_Addr = cur_in_from_lut.node_addr;
                    string lut_instr_4 = cat_instr_4(instr_4);
                    processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), cur_lut_idle_num, lut_instr_4);
                    // in from lut
                    processors[from_processor_id].instr_mem.insert(processors[from_processor_id].instr_mem.end(), from_lut_idle_num, lut_instr_4);
                }
                auto max_config_addr = max_element(config_instr_pos.begin(), config_instr_pos.end());
                Instr_4 instr_4;
                instr_4.PC_Jump = 0;
                instr_4.BM_Jump = 0;
                instr_4.Node_Addr = cur_dff.node_addr;
                string lut_instr_4 = cat_instr_4(instr_4);
                processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), *max_config_addr - int(processors[cur_processor_id].instr_mem.size()), lut_instr_4);

                if ((cur_dff.assignsig_condsig.begin()->first == "0" || cur_dff.assignsig_condsig.begin()->first == "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first == "0" || (cur_dff.assignsig_condsig.begin() + 1)->first == "1"))
                {
                    Instr_5 instr_5;
                    instr_5.PC_Jump = 0;
                    instr_5.BM_Jump = 0;
                    instr_5.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 1);
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_5.data_mem_select = 0;
                            else
                                instr_5.data_mem_select = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_5.Operand_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_5.Operand_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_5.data_mem_select = 0;
                        else
                            instr_5.data_mem_select = 1;
                        instr_5.Operand_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[0]].node_addr == cur_dff.node_addr)
                    {
                        instr_5.data_mem_select = 0;
                        instr_5.Operand_Addr = luts[*(cur_dff.in_net_from_id.begin())].res_pos_at_mem;
                    }
                    else
                    {
                        instr_5.data_mem_select = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[0]);
                        instr_5.Operand_Addr = it->second;
                    }
                    
                    instr_5.value = cur_dff.assignsig_condsig.begin()->second.second;
                    instr_5.T = stoi(cur_dff.assignsig_condsig.begin()->first.c_str());
                    instr_5.F = stoi((cur_dff.assignsig_condsig.begin() + 1)->first.c_str());
                    string dff_instr = cat_instr_5(instr_5);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first == "0" || cur_dff.assignsig_condsig.begin()->first == "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first != "0" && (cur_dff.assignsig_condsig.begin() + 1)->first != "1"))
                {
                    Instr_6 instr_6;
                    instr_6.PC_Jump = 0;
                    instr_6.BM_Jump = 0;
                    instr_6.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 2);
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_6.data_mem_select1 = 0;
                            else
                                instr_6.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_6.Operand_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_6.Operand_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_6.data_mem_select1 = 0;
                        else
                            instr_6.data_mem_select1 = 1;
                        instr_6.Operand_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[0]].node_addr == cur_dff.node_addr)
                    {
                        instr_6.data_mem_select1 = 0;
                        instr_6.Operand_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_6.data_mem_select1 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[0]);
                        instr_6.Operand_Addr = it->second;
                    }
                    instr_6.value = cur_dff.assignsig_condsig.begin()->second.second;
                    instr_6.T = stoi(cur_dff.assignsig_condsig.begin()->first.c_str());
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_6.data_mem_select2 = 0;
                            else
                                instr_6.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_6.F_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_6.F_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_6.data_mem_select2 = 0;
                        else
                            instr_6.data_mem_select2 = 1;
                        instr_6.F_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[1]].node_addr == cur_dff.node_addr)
                    {
                        instr_6.data_mem_select2 = 0;
                        instr_6.F_Addr = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_6.data_mem_select2 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[1]);
                        instr_6.F_Addr = it->second;
                    }
                    string dff_instr = cat_instr_6(instr_6);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first != "0" && cur_dff.assignsig_condsig.begin()->first != "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first != "0" && (cur_dff.assignsig_condsig.begin() + 1)->first != "1"))
                {
                    Instr_7 instr_7;
                    instr_7.PC_Jump = 0;
                    instr_7.BM_Jump = 0;
                    instr_7.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 3);
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_7.data_mem_select1 = 0;
                            else
                                instr_7.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_7.Operand_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_7.Operand_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_7.data_mem_select1 = 0;
                        else
                            instr_7.data_mem_select1 = 1;
                        instr_7.Operand_Addr = MEM_DEPTH - 1;
                    }
                    else
                    {
                        instr_7.data_mem_select1 = 0;
                        instr_7.Operand_Addr = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    instr_7.value = cur_dff.assignsig_condsig.begin()->second.second;
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_7.data_mem_select2 = 0;
                            else
                                instr_7.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_7.T_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_7.T_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_7.data_mem_select2 = 0;
                        else
                            instr_7.data_mem_select2 = 1;
                        instr_7.T_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[0]].node_addr == cur_dff.node_addr)
                    {
                        instr_7.data_mem_select2 = 0;
                        instr_7.T_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_7.data_mem_select2 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[0]);
                        instr_7.T_Addr = it->second;
                    }
                    if (cur_dff.in_net_from_id[2] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[2]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[2]];
                            if (*(tvs_val->begin()) == 0)
                                instr_7.data_mem_select3 = 0;
                            else
                                instr_7.data_mem_select3 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[2] == INITIAL_JUMP_SIG)
                                instr_7.F_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_7.F_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[2] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[2] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[2] == 0)
                            instr_7.data_mem_select3 = 0;
                        else
                            instr_7.data_mem_select3 = 1;
                        instr_7.F_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[2]].node_addr == cur_dff.node_addr)
                    {
                        instr_7.data_mem_select3 = 0;
                        instr_7.F_Addr = luts[cur_dff.in_net_from_id[2]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_7.data_mem_select3 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[2]);
                        instr_7.F_Addr = it->second;
                    }
                    string dff_instr = cat_instr_7(instr_7);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first != "0" && cur_dff.assignsig_condsig.begin()->first != "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first == "0" || (cur_dff.assignsig_condsig.begin() + 1)->first == "1"))
                {
                    Instr_8 instr_8;
                    instr_8.PC_Jump = 0;
                    instr_8.BM_Jump = 0;
                    instr_8.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 2);
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_8.data_mem_select1 = 0;
                            else
                                instr_8.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_8.Operand_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_8.Operand_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_8.data_mem_select1 = 0;
                        else
                            instr_8.data_mem_select1 = 1;
                        instr_8.Operand_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[1]].node_addr == cur_dff.node_addr)
                    {
                        instr_8.data_mem_select1 = 0;
                        instr_8.Operand_Addr = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_8.data_mem_select1 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[1]);
                        instr_8.Operand_Addr = it->second;
                    }
                    instr_8.value = cur_dff.assignsig_condsig.begin()->second.second;
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_8.data_mem_select2 = 0;
                            else
                                instr_8.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_8.T_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_8.T_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_8.data_mem_select2 = 0;
                        else
                            instr_8.data_mem_select2 = 1;
                        instr_8.T_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[0]].node_addr == cur_dff.node_addr)
                    {
                        instr_8.data_mem_select2 = 0;
                        instr_8.T_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_8.data_mem_select2 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[0]);
                        instr_8.T_Addr = it->second;
                    }
                    instr_8.F = atoi((cur_dff.assignsig_condsig.begin() + 1)->first.c_str());
                    string dff_instr = cat_instr_8(instr_8);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
            }
            else if (cur_dff.type == 3)
            {
                assert(cur_dff.assignsig_condsig.size() == 2);
                vector<int> cur_in_net_from_id = cur_dff.in_net_from_id;
                vector<int> cur_in_net_from_part = cur_dff.in_net_from_part;
                for (int it = 0; it < cur_in_net_from_id.size();)
                {
                    if (cur_in_net_from_id[it] == -1 || cur_in_net_from_id[it] == -2 || luts[cur_in_net_from_id[it]].node_addr == cur_dff.node_addr)
                    {
                        cur_in_net_from_id.erase(cur_in_net_from_id.begin() + it);
                        cur_in_net_from_part.erase(cur_in_net_from_part.begin() + it);
                    }
                    else
                        it++;
                }
                vector<pair<int, int>> id_instr; // <cur_in_net_from_id, idealy first_get_res_addr>
                vector<int> _cur_in_net_from_id;
                vector<int> get_res_instr_pos;
                vector<int> instr_start_pos;
                vector<int> config_instr_pos;
                map<int, int> from_id_from_part; // <in_from_id, id_from_part>
                map<int, int> from_id_opr_addr;  // <in_from_id, operand_addr>
                for (int in = 0; in < cur_in_net_from_id.size(); in++)
                {
                    LutType cur_in_from_lut = luts[cur_in_net_from_id[in]];
                    int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                    from_id_from_part.insert(pair<int, int>(cur_in_net_from_id[in], cur_in_net_from_part[in]));
                    if (processors[from_processor_id].id_outaddr.first == cur_in_net_from_id[in] && processors[from_processor_id].id_outaddr.second > 0 && processors[from_processor_id].id_outaddr.second < N_INS_PER_PROCESSOR)
                    {
                        int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                        id_instr.push_back(pair<int, int>(cur_in_net_from_id[in], first_get_res_addr));
                    }
                    else
                    {
                        Instr_2 instr_2;
                        instr_2.PC_Jump = 0;
                        instr_2.BM_Jump = 0;
                        instr_2.Node_Addr = cur_in_from_lut.node_addr;
                        instr_2.Data_Mem_Select = "0";
                        instr_2.Operand_Addr = cur_in_from_lut.res_pos_at_mem;
                        string lut_instr_2 = cat_instr_2(instr_2);
                        processors[from_processor_id].instr_mem.push_back(lut_instr_2);
                        processors[from_processor_id].id_outaddr = pair<int, int>(cur_in_net_from_id[in], int(processors[from_processor_id].instr_mem.size()) - 1);
                        int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                        id_instr.push_back(pair<int, int>(cur_in_net_from_id[in], first_get_res_addr));
                    }
                }
                sort(id_instr.begin(), id_instr.end(), cmp);
                for (vector<pair<int, int>>::iterator it = id_instr.begin(); it != id_instr.end(); it++)
                {
                    _cur_in_net_from_id.push_back(it->first);
                    get_res_instr_pos.push_back(it->second);
                    instr_start_pos.push_back(it->second);
                }
                if (instr_start_pos.size() >= 1)
                {
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
                }                
                config_instr_pos = instr_start_pos;
                for (int i = 0; i < _cur_in_net_from_id.size(); i++)
                {
                    from_id_opr_addr.insert(pair<int, int>(_cur_in_net_from_id[i], instr_start_pos[i]));
                    if (from_id_from_part[_cur_in_net_from_id[i]] == -3) // current part
                        config_instr_pos[i] += (INTER_CLUSTER_CLK - 1);
                    else // other part
                        config_instr_pos[i] += (ACR_CLUSTER_CLK - 1);
                    LutType cur_in_from_lut = luts[_cur_in_net_from_id[i]];
                    int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_lut.node_addr[0] + cur_in_from_lut.node_addr[1];
                    int cur_id = _cur_in_net_from_id[i];
                    int cur_num = get_res_instr_pos[i];
                    int cur_pos = instr_start_pos[i];
                    int cur_lut_idle_num = cur_pos - (int(processors[cur_processor_id].instr_mem.size()) - 1);
                    int from_lut_idle_num = ((cur_pos - int(processors[from_processor_id].instr_mem.size()) + 1) > 0) ? (cur_pos - int(processors[from_processor_id].instr_mem.size()) + 1) : 0;
                    // current dff
                    Instr_4 instr_4;
                    instr_4.PC_Jump = 0;
                    instr_4.BM_Jump = 0;
                    instr_4.Node_Addr = cur_in_from_lut.node_addr;
                    string lut_instr_4 = cat_instr_4(instr_4);
                    processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), cur_lut_idle_num, lut_instr_4);
                    // in from lut
                    processors[from_processor_id].instr_mem.insert(processors[from_processor_id].instr_mem.end(), from_lut_idle_num, lut_instr_4);
                }
                auto max_config_addr = max_element(config_instr_pos.begin(), config_instr_pos.end());
                Instr_4 instr_4;
                instr_4.PC_Jump = 0;
                instr_4.BM_Jump = 0;
                instr_4.Node_Addr = cur_dff.node_addr;
                string lut_instr_4 = cat_instr_4(instr_4);
                processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), *max_config_addr - int(processors[cur_processor_id].instr_mem.size()), lut_instr_4);

                if ((cur_dff.assignsig_condsig.begin()->first == "0" || cur_dff.assignsig_condsig.begin()->first == "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first == "0" || (cur_dff.assignsig_condsig.begin() + 1)->first == "1"))
                {
                    Instr_13 instr_13;
                    instr_13.PC_Jump = 0;
                    instr_13.BM_Jump = 0;
                    instr_13.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 2);
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_13.data_mem_select1 = 0;
                            else
                                instr_13.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_13.Operand_Addr1 = INITIAL_JUMP_ADDR;
                            else
                                instr_13.Operand_Addr1 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_13.data_mem_select1 = 0;
                        else
                            instr_13.data_mem_select1 = 1;
                        instr_13.Operand_Addr1 = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[0]].node_addr == cur_dff.node_addr)
                    {
                        instr_13.data_mem_select1 = 0;
                        instr_13.Operand_Addr1 = luts[*(cur_dff.in_net_from_id.begin())].res_pos_at_mem;
                    }
                    else
                    {
                        instr_13.data_mem_select1 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[0]);
                        instr_13.Operand_Addr1 = it->second;
                    }
                    instr_13.value1 = cur_dff.assignsig_condsig.begin()->second.second;
                    instr_13.T1 = stoi(cur_dff.assignsig_condsig.begin()->first.c_str());
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_13.data_mem_select2 = 0;
                            else
                                instr_13.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_13.Operand_Addr2 = INITIAL_JUMP_ADDR;
                            else
                                instr_13.Operand_Addr2 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_13.data_mem_select2 = 0;
                        else
                            instr_13.data_mem_select2 = 1;
                        instr_13.Operand_Addr2 = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[1]].node_addr == cur_dff.node_addr)
                    {
                        instr_13.data_mem_select2 = 0;
                        instr_13.Operand_Addr2 = luts[*(cur_dff.in_net_from_id.begin() + 1)].res_pos_at_mem;
                    }
                    else
                    {
                        instr_13.data_mem_select2 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[1]);
                        instr_13.Operand_Addr2 = it->second;
                    }
                    instr_13.value2 = (cur_dff.assignsig_condsig.begin() + 1)->second.second;
                    instr_13.T2 = stoi((cur_dff.assignsig_condsig.begin() + 1)->first.c_str());
                    string dff_instr = cat_instr_13(instr_13);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first == "0" || cur_dff.assignsig_condsig.begin()->first == "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first != "0" && (cur_dff.assignsig_condsig.begin() + 1)->first != "1"))
                {
                    Instr_14 instr_14;
                    instr_14.PC_Jump = 0;
                    instr_14.BM_Jump = 0;
                    instr_14.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 3);
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_14.data_mem_select1 = 0;
                            else
                                instr_14.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_14.Operand_Addr1 = INITIAL_JUMP_ADDR;
                            else
                                instr_14.Operand_Addr1 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_14.data_mem_select1 = 0;
                        else
                            instr_14.data_mem_select1 = 1;
                        instr_14.Operand_Addr1 = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[0]].node_addr == cur_dff.node_addr)
                    {
                        instr_14.data_mem_select1 = 0;
                        instr_14.Operand_Addr1 = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_14.data_mem_select1 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[0]);
                        instr_14.Operand_Addr1 = it->second;
                    }
                    instr_14.value1 = cur_dff.assignsig_condsig.begin()->second.second;
                    instr_14.T1 = stoi(cur_dff.assignsig_condsig.begin()->first.c_str());
                    if (cur_dff.in_net_from_id[2] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[2]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[2]];
                            if (*(tvs_val->begin()) == 0)
                                instr_14.data_mem_select2 = 0;
                            else
                                instr_14.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[2] == INITIAL_JUMP_SIG)
                                instr_14.Operand_Addr2 = INITIAL_JUMP_ADDR;
                            else
                                instr_14.Operand_Addr2 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[2] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[2] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[2] == 0)
                            instr_14.data_mem_select2 = 0;
                        else
                            instr_14.data_mem_select2 = 1;
                        instr_14.Operand_Addr2 = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[2]].node_addr == cur_dff.node_addr)
                    {
                        instr_14.data_mem_select2 = 0;
                        instr_14.Operand_Addr2 = luts[cur_dff.in_net_from_id[2]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_14.data_mem_select2 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[2]);
                        instr_14.Operand_Addr2 = it->second;
                    }
                    instr_14.value2 = (cur_dff.assignsig_condsig.begin() + 1)->second.second;
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_14.data_mem_select3 = 0;
                            else
                                instr_14.data_mem_select3 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_14.T2_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_14.T2_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_14.data_mem_select3 = 0;
                        else
                            instr_14.data_mem_select3 = 1;
                        instr_14.T2_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[1]].node_addr == cur_dff.node_addr)
                    {
                        instr_14.data_mem_select3 = 0;
                        instr_14.T2_Addr = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_14.data_mem_select3 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[1]);
                        instr_14.T2_Addr = it->second;
                    }
                    string dff_instr = cat_instr_14(instr_14);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first != "0" && cur_dff.assignsig_condsig.begin()->first != "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first == "0" || (cur_dff.assignsig_condsig.begin() + 1)->first == "1"))
                {
                    Instr_15 instr_15;
                    instr_15.PC_Jump = 0;
                    instr_15.BM_Jump = 0;
                    instr_15.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 3);
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_15.data_mem_select1 = 0;
                            else
                                instr_15.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_15.Operand_Addr1 = INITIAL_JUMP_ADDR;
                            else
                                instr_15.Operand_Addr1 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_15.data_mem_select1 = 0;
                        else
                            instr_15.data_mem_select1 = 1;
                        instr_15.Operand_Addr1 = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[1]].node_addr == cur_dff.node_addr)
                    {
                        instr_15.data_mem_select1 = 0;
                        instr_15.Operand_Addr1 = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_15.data_mem_select1 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[1]);
                        instr_15.Operand_Addr1 = it->second;
                    }
                    instr_15.value1 = cur_dff.assignsig_condsig.begin()->second.second;
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_15.data_mem_select2 = 0;
                            else
                                instr_15.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_15.T1_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_15.T1_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_15.data_mem_select2 = 0;
                        else
                            instr_15.data_mem_select2 = 1;
                        instr_15.T1_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[0]].node_addr == cur_dff.node_addr)
                    {
                        instr_15.data_mem_select2 = 0;
                        instr_15.T1_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_15.data_mem_select2 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[0]);
                        instr_15.T1_Addr = it->second;
                    }
                    if (cur_dff.in_net_from_id[2] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[2]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[2]];
                            if (*(tvs_val->begin()) == 0)
                                instr_15.data_mem_select3 = 0;
                            else
                                instr_15.data_mem_select3 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[2] == INITIAL_JUMP_SIG)
                                instr_15.Operand_Addr2 = INITIAL_JUMP_ADDR;
                            else
                                instr_15.Operand_Addr2 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[2] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[2] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[2] == 0)
                            instr_15.data_mem_select3 = 0;
                        else
                            instr_15.data_mem_select3 = 1;
                        instr_15.Operand_Addr2 = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[2]].node_addr == cur_dff.node_addr)
                    {
                        instr_15.data_mem_select3 = 0;
                        instr_15.Operand_Addr2 = luts[cur_dff.in_net_from_id[2]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_15.data_mem_select3 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[2]);
                        instr_15.Operand_Addr2 = it->second;
                    }
                    instr_15.value2 = (cur_dff.assignsig_condsig.begin() + 1)->second.second;
                    instr_15.T2 = stoi((cur_dff.assignsig_condsig.begin() + 1)->first.c_str());
                    string dff_instr = cat_instr_15(instr_15);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
                else if ((cur_dff.assignsig_condsig.begin()->first != "0" && cur_dff.assignsig_condsig.begin()->first != "1") && ((cur_dff.assignsig_condsig.begin() + 1)->first != "0" && (cur_dff.assignsig_condsig.begin() + 1)->first != "1"))
                {
                    Instr_16 instr_16;
                    instr_16.PC_Jump = 0;
                    instr_16.BM_Jump = 0;
                    instr_16.Node_Addr = cur_dff.node_addr;
                    assert(cur_dff.in_net_from_id.size() == 4);
                    if (cur_dff.in_net_from_id[1] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[1]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[1]];
                            if (*(tvs_val->begin()) == 0)
                                instr_16.data_mem_select1 = 0;
                            else
                                instr_16.data_mem_select1 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[1] == INITIAL_JUMP_SIG)
                                instr_16.Operand_Addr1 = INITIAL_JUMP_ADDR;
                            else
                                instr_16.Operand_Addr1 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[1] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[1] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[1] == 0)
                            instr_16.data_mem_select1 = 0;
                        else
                            instr_16.data_mem_select1 = 1;
                        instr_16.Operand_Addr1 = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[1]].node_addr == cur_dff.node_addr)
                    {
                        instr_16.data_mem_select1 = 0;
                        instr_16.Operand_Addr1 = luts[cur_dff.in_net_from_id[1]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_16.data_mem_select1 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[1]);
                        instr_16.Operand_Addr1 = it->second;
                    }
                    instr_16.value1 = cur_dff.assignsig_condsig.begin()->second.second;
                    if (cur_dff.in_net_from_id[0] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[0]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[0]];
                            if (*(tvs_val->begin()) == 0)
                                instr_16.data_mem_select2 = 0;
                            else
                                instr_16.data_mem_select2 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[0] == INITIAL_JUMP_SIG)
                                instr_16.T1_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_16.T1_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[0] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[0] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[0] == 0)
                            instr_16.data_mem_select2 = 0;
                        else
                            instr_16.data_mem_select2 = 1;
                        instr_16.T1_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[0]].node_addr == cur_dff.node_addr)
                    {
                        instr_16.data_mem_select2 = 0;
                        instr_16.T1_Addr = luts[cur_dff.in_net_from_id[0]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_16.data_mem_select2 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[0]);
                        instr_16.T1_Addr = it->second;
                    }
                    if (cur_dff.in_net_from_id[3] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[3]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[3]];
                            if (*(tvs_val->begin()) == 0)
                                instr_16.data_mem_select3 = 0;
                            else
                                instr_16.data_mem_select3 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[3] == INITIAL_JUMP_SIG)
                                instr_16.Operand_Addr2 = INITIAL_JUMP_ADDR;
                            else
                                instr_16.Operand_Addr2 = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[3] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[3] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[3] == 0)
                            instr_16.data_mem_select3 = 0;
                        else
                            instr_16.data_mem_select3 = 1;
                        instr_16.Operand_Addr2 = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[3]].node_addr == cur_dff.node_addr)
                    {
                        instr_16.data_mem_select3 = 0;
                        instr_16.Operand_Addr2 = luts[cur_dff.in_net_from_id[3]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_16.data_mem_select3 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[3]);
                        instr_16.Operand_Addr2 = it->second;
                    }
                    instr_16.value2 = (cur_dff.assignsig_condsig.begin() + 1)->second.second;
                    if (cur_dff.in_net_from_id[2] == -1)
                    {
                        if (vcd_values.find(cur_dff.in_net_from_info[2]) != vcd_values.end())
                        {
                            vector<short> *tvs_val = vcd_values[cur_dff.in_net_from_info[2]];
                            if (*(tvs_val->begin()) == 0)
                                instr_16.data_mem_select4 = 0;
                            else
                                instr_16.data_mem_select4 = 1;
                            // for input initial signal INITIAL_JUMP_SIG changing
                            if (cur_dff.in_net_from_info[2] == INITIAL_JUMP_SIG)
                                instr_16.T2_Addr = INITIAL_JUMP_ADDR;
                            else
                                instr_16.T2_Addr = MEM_DEPTH - 1;
                        }
                        else
                        {
                            std::cout << "ERROR: No initial info of signal " << cur_dff.in_net_from_info[2] << " (DFF " << dff_num << ")"
                                      << " in vcd file!" << endl;
                        }
                    }
                    else if (cur_dff.in_net_from_id[2] == -2)
                    {
                        if (cur_dff.in_net_from_pos_at_level[2] == 0)
                            instr_16.data_mem_select4 = 0;
                        else
                            instr_16.data_mem_select4 = 1;
                        instr_16.T2_Addr = MEM_DEPTH - 1;
                    }
                    else if (luts[cur_dff.in_net_from_id[2]].node_addr == cur_dff.node_addr)
                    {
                        instr_16.data_mem_select4 = 0;
                        instr_16.T2_Addr = luts[cur_dff.in_net_from_id[2]].res_pos_at_mem;
                    }
                    else
                    {
                        instr_16.data_mem_select4 = 1;
                        auto it = from_id_opr_addr.find(cur_dff.in_net_from_id[2]);
                        instr_16.T2_Addr = it->second;
                    }
                    string dff_instr = cat_instr_16(instr_16);
                    processors[cur_processor_id].instr_mem.push_back(dff_instr);
                    dffs[dff_num].res_pos_at_mem = int(processors[cur_processor_id].instr_mem.size()) - 1;
                    processors[cur_processor_id].id_outaddr = pair<int, int>(dff_num + luts.size(), -1);
                }
            }
        }
    }

    // when the input of LUT is from the output of DFF, getting the DFF output address to replace the instruction for LUT
    for (auto bas = blank_addrs.begin(); bas != blank_addrs.end(); bas++)
    {
        int lut_num = bas->first;
        LutType cur_lut = luts[lut_num];
        int cur_processor_id = N_PROCESSORS_PER_CLUSTER * cur_lut.node_addr[0] + cur_lut.node_addr[1];
        vector<int> cur_in_net_from_id;
        for (auto id = bas->second.begin(); id != bas->second.end(); id++)
        {
            cur_in_net_from_id.push_back(id->dff_num);
        }
        vector<int> cur_in_net_from_part;
        for (auto for_part = cur_in_net_from_id.begin(); for_part != cur_in_net_from_id.end(); for_part++)
        {
            if (part[*for_part + luts.size()] == part[lut_num])
                cur_in_net_from_part.push_back(-3);
            else
                cur_in_net_from_part.push_back(part[*for_part + luts.size()]);
        }
        for (int it = 0; it < cur_in_net_from_id.size();)
        {
            if (dffs[cur_in_net_from_id[it]].node_addr == cur_lut.node_addr)
            {
                cur_in_net_from_id.erase(cur_in_net_from_id.begin() + it);
                cur_in_net_from_part.erase(cur_in_net_from_part.begin() + it);
            }
            else
                it++;
        }
        vector<pair<int, int>> id_instr; // <cur_in_net_from_id, idealy first_get_res_addr>
        vector<int> _cur_in_net_from_id;
        vector<int> get_res_instr_pos;
        vector<int> instr_start_pos;
        vector<int> config_instr_pos;
        map<int, int> from_id_from_part; // <in_from_id, id_from_part>
        map<int, int> from_id_opr_addr;  // <in_from_id, operand_addr>
        for (int in = 0; in < cur_in_net_from_id.size(); in++)
        {
            DffType cur_in_from_dff = dffs[cur_in_net_from_id[in]];
            int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_dff.node_addr[0] + cur_in_from_dff.node_addr[1];
            from_id_from_part.insert(pair<int, int>(cur_in_net_from_id[in], cur_in_net_from_part[in]));
            if (processors[from_processor_id].id_outaddr.first == cur_in_net_from_id[in] && processors[from_processor_id].id_outaddr.second > 0 && processors[from_processor_id].id_outaddr.second < N_INS_PER_PROCESSOR)
            {
                int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                id_instr.push_back(pair<int, int>(cur_in_net_from_id[in], first_get_res_addr));
            }
            else
            {
                Instr_2 instr_2;
                instr_2.PC_Jump = 0;
                instr_2.BM_Jump = 0;
                instr_2.Node_Addr = cur_in_from_dff.node_addr;
                instr_2.Data_Mem_Select = "0";
                instr_2.Operand_Addr = cur_in_from_dff.res_pos_at_mem;
                string lut_instr_2 = cat_instr_2(instr_2);
                processors[from_processor_id].instr_mem.push_back(lut_instr_2);
                processors[from_processor_id].id_outaddr = pair<int, int>(cur_in_net_from_id[in], int(processors[from_processor_id].instr_mem.size()) - 1);
                int first_get_res_addr = max(processors[from_processor_id].id_outaddr.second, int(processors[cur_processor_id].instr_mem.size()));
                id_instr.push_back(pair<int, int>(cur_in_net_from_id[in], first_get_res_addr));
            }
        }
        sort(id_instr.begin(), id_instr.end(), cmp);
        for (vector<pair<int, int>>::iterator it = id_instr.begin(); it != id_instr.end(); it++)
        {
            _cur_in_net_from_id.push_back(it->first);
            get_res_instr_pos.push_back(it->second);
            instr_start_pos.push_back(it->second);
        }
        if (instr_start_pos.size() >= 1)
        {
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
        }       
        for (int i = 0; i < _cur_in_net_from_id.size(); i++)
        {
            from_id_opr_addr.insert(pair<int, int>(_cur_in_net_from_id[i], instr_start_pos[i]));
            DffType cur_in_from_dff = dffs[_cur_in_net_from_id[i]];
            int from_processor_id = N_PROCESSORS_PER_CLUSTER * cur_in_from_dff.node_addr[0] + cur_in_from_dff.node_addr[1];
            int cur_id = _cur_in_net_from_id[i];
            int cur_num = get_res_instr_pos[i];
            int cur_pos = instr_start_pos[i];
            int cur_lut_idle_num = cur_pos - (int(processors[cur_processor_id].instr_mem.size()) - 1);
            int from_lut_idle_num = ((cur_pos - int(processors[from_processor_id].instr_mem.size()) + 1) > 0) ? (cur_pos - int(processors[from_processor_id].instr_mem.size()) + 1) : 0;
            // current lut
            Instr_4 instr_4;
            instr_4.PC_Jump = 0;
            instr_4.BM_Jump = 0;
            instr_4.Node_Addr = cur_in_from_dff.node_addr;
            string lut_instr_4 = cat_instr_4(instr_4);
            processors[cur_processor_id].instr_mem.insert(processors[cur_processor_id].instr_mem.end(), cur_lut_idle_num, lut_instr_4);
            // in from lut
            processors[from_processor_id].instr_mem.insert(processors[from_processor_id].instr_mem.end(), from_lut_idle_num, lut_instr_4);
        }

        for (auto i = bas->second.begin(); i != bas->second.end(); i++)
        {
            if (from_id_opr_addr.find(i->dff_num) != from_id_opr_addr.end())
            {
                int new_addr = from_id_opr_addr[i->dff_num];
                int bp_id = N_PROCESSORS_PER_CLUSTER * i->LUT_Node_Addr[0] + i->LUT_Node_Addr[1];
                string instr = processors[bp_id].instr_mem[i->LUT_Instr_Addr];
                int range_left = i->Replaced_Instr_Range.first;
                int range_right = i->Replaced_Instr_Range.second;
                string new_instr;
                stringstream ss;
                ss << instr.substr(0, (79 - (range_left + 1) + 1)) << bitset<9>(toBinary(new_addr)) << instr.substr((79 - range_right + 1), range_right);
                new_instr = ss.str();
                processors[bp_id].instr_mem[i->LUT_Instr_Addr] = new_instr;
            }
            else
            {
                int new_addr = dffs[i->dff_num].res_pos_at_mem;
                int bp_id = N_PROCESSORS_PER_CLUSTER * i->LUT_Node_Addr[0] + i->LUT_Node_Addr[1];
                string instr = processors[bp_id].instr_mem[i->LUT_Instr_Addr];
                int range_left = i->Replaced_Instr_Range.first;
                int range_right = i->Replaced_Instr_Range.second;
                string new_instr;
                stringstream ss;
                ss << instr.substr(0, (79 - (range_left + 1) + 1)) << bitset<9>(toBinary(new_addr)) << instr.substr((79 - range_right + 1), range_right);
                new_instr = ss.str();
                processors[bp_id].instr_mem[i->LUT_Instr_Addr] = new_instr;
            }
        }
    }

    // instruction alignment with IDLE
    int longest_instr1{0};
    for (auto it = processors.begin(); it != processors.end(); it++)
    {
        if (it->second.instr_mem.size() > longest_instr1)
            longest_instr1 = it->second.instr_mem.size();
    }
    for (auto it = processors.begin(); it != processors.end(); it++)
    {
        if (it->second.instr_mem.size() < longest_instr1)
        {
            Instr_4 instr_4;
            instr_4.PC_Jump = 0;
            instr_4.BM_Jump = 0;
            instr_4.Node_Addr.push_back(it->first / N_PROCESSORS_PER_CLUSTER);
            instr_4.Node_Addr.push_back(it->first % N_PROCESSORS_PER_CLUSTER);
            string lut_instr_4 = cat_instr_4(instr_4);
            processors[it->first].instr_mem.insert(processors[it->first].instr_mem.end(), (longest_instr1 - int(it->second.instr_mem.size())), lut_instr_4);
        }
    }
    for (auto it = processors.begin(); it != processors.end(); it++)
    {
        assert(int(it->second.instr_mem.size()) == longest_instr1);
    }

    // instruction for PC jumping and input changing
    for (auto it = processors.begin(); it != processors.end(); it++)
    {
        Instr_3 instr_3;
        instr_3.PC_Jump = 1;
        instr_3.BM_Jump = 0;
        instr_3.Node_Addr.push_back(it->first / N_PROCESSORS_PER_CLUSTER);
        instr_3.Node_Addr.push_back(it->first % N_PROCESSORS_PER_CLUSTER);
        instr_3.Static_Binary_Value = "1";
        string lut_instr_3 = cat_instr_3(instr_3);
        processors[it->first].instr_mem.insert(processors[it->first].instr_mem.end(), 1, lut_instr_3);
    }

    auto end_ins = std::chrono::steady_clock::now();
    long duration_ins = std::chrono::duration_cast<std::chrono::milliseconds>(end_ins - start_ins).count();
    std::cout << "Successfully finished instruction generation. (" << duration_ins << "ms)" << endl;
    std::cout << endl;

    /****************************************************************************************************/
    /*                                        outfile generation                                        */
    /****************************************************************************************************/
    std::cout << "8. Executing output files generation." << endl;
    auto start_out = std::chrono::steady_clock::now();

    for (auto it = processors.begin(); it != processors.end(); it++)
    {
        auto i = it->first;
        int cluster_num = (i / N_PROCESSORS_PER_CLUSTER) + 1;
        int bp_num = (i % N_PROCESSORS_PER_CLUSTER) + 1;
        string cur_instr_out = instr_out + "instrmem_" + to_string(cluster_num) + "_" + to_string(bp_num) + ".coe";
        ofstream outinstr(cur_instr_out);
        outinstr << "MEMORY_INITIALIZATION_RADIX = " << MEMORY_INITIALIZATION_RADIX << ";" << endl;
        outinstr << "MEMORY_INITIALIZATION_VECTOR =" << endl;
        vector<string> instrs = it->second.instr_mem; 
        assert(instrs.size() < N_INS_PER_PROCESSOR);
        for (vector<string>::iterator iter = instrs.begin(); iter != instrs.end(); iter++)
        {
            if (iter == instrs.end() - 1)
            {
                outinstr << *iter << ";" << endl;
            }
            else
            {
                outinstr << *iter << "," << endl;
            }
        }
        outinstr.close();
    }
     
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
                    outdir << setw(30) << i->first;
                    outdir << "LUT: ";
                    outdir << setw(10) << it->first;
                    outdir << "Clutser: ";
                    outdir << setw(10) << it->second.node_addr[0];
                    outdir << "Processor: ";
                    outdir << setw(10) << it->second.node_addr[1];
                    outdir << "Addr: ";
                    outdir << setw(10) << it->second.res_pos_at_mem << endl;
                }
            }
            for (auto it = dffs.begin(); it != dffs.end(); it++)
            {
                if (it->second.dff_out == i->first)
                {
                    outdir << "Pin: ";
                    outdir.setf(ios::left);
                    outdir << setw(30) << i->first;
                    outdir << "DFF: ";
                    outdir << setw(10) << it->first;
                    outdir << "Clutser: ";
                    outdir << setw(10) << it->second.node_addr[0];
                    outdir << "Processor: ";
                    outdir << setw(10) << it->second.node_addr[1];
                    outdir << "Addr: ";
                    outdir << setw(10) << it->second.res_pos_at_mem << endl;
                }
            }
            if (assign_pairs.find(i->first) != assign_pairs.end())
            {
                string tmp = assign_pairs[i->first];
                for (map<int, LutType>::iterator it = luts.begin(); it != luts.end(); it++)
                {
                    if (it->second.out_ports == tmp)
                    {
                        outdir << "Pin: ";
                        outdir.setf(ios::left);
                        outdir << setw(30) << i->first;
                        outdir << "LUT: ";
                        outdir << setw(10) << it->first;
                        outdir << "Clutser: ";
                        outdir << setw(10) << it->second.node_addr[0];
                        outdir << "Processor: ";
                        outdir << setw(10) << it->second.node_addr[1];
                        outdir << "Addr: ";
                        outdir << setw(10) << it->second.res_pos_at_mem << endl;
                    }
                }
                for (auto it = dffs.begin(); it != dffs.end(); it++)
                {
                    if (it->second.dff_out == tmp)
                    {
                        outdir << "Pin: ";
                        outdir.setf(ios::left);
                        outdir << setw(30) << i->first;
                        outdir << "DFF: ";
                        outdir << setw(10) << it->first;
                        outdir << "Clutser: ";
                        outdir << setw(10) << it->second.node_addr[0];
                        outdir << "Processor: ";
                        outdir << setw(10) << it->second.node_addr[1];
                        outdir << "Addr: ";
                        outdir << setw(10) << it->second.res_pos_at_mem << endl;
                    }
                }
            }
        }
    }
    outdir.close();

    auto end_out = std::chrono::steady_clock::now();
    long duration_out = std::chrono::duration_cast<std::chrono::milliseconds>(end_out - start_out).count();
    std::cout << "Successfully finished output files generation. (" << duration_out << "ms)" << endl;
    std::cout << endl;

    auto end_total = std::chrono::steady_clock::now();
    long duration_total = std::chrono::duration_cast<std::chrono::milliseconds>(end_total - start_total).count();
    std::cout << "COMPILING DONE!    TIME:" << duration_total << "ms" << endl;
    std::cout << endl;
}


vector<idx_t> part_func(vector<idx_t> &xadj, vector<idx_t> &adjncy, /*vector<idx_t> &adjwgt, */ decltype(METIS_PartGraphKway) *METIS_PartGraphFunc)
{
    idx_t nVertices = xadj.size() - 1;
    idx_t nEdges = adjncy.size() / 2;
    idx_t nWeights = 1;
    idx_t nParts = (nVertices + N_PROCESSORS_PER_CLUSTER * N_LUTS_PER_PROCESSOR - 1) / (N_PROCESSORS_PER_CLUSTER * N_LUTS_PER_PROCESSOR);
    assert(nParts <= MAX_CLUSTERS);
    idx_t objval;
    vector<idx_t> part(nVertices, 0);
    int ret = METIS_PartGraphFunc(&nVertices, &nWeights, xadj.data(), adjncy.data(),
                                  NULL, NULL, NULL, &nParts, NULL,
                                  NULL, NULL, &objval, part.data());
    if (ret != rstatus_et::METIS_OK)
    {
        std::cout << "METIS ERROR!" << endl;
    }
    std::cout << "METIS OK!" << endl;
    std::cout << "parts size: " << nParts << endl;
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
            case 0:
                BinDigit += "0000";
                break;
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
    int cur_node_addr = instr_1.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_1.Node_Addr[1];
    int filling_num = 4 - instr_1.Operand_Addr.size();
    for (int i = 0; i < filling_num; i++)
    {
        instr_1.Operand_Addr.insert(instr_1.Operand_Addr.begin(), MEM_DEPTH - 1);
    }
    string cat_instr_1;
    stringstream ss;
    ss << "0000" << instr_1.PC_Jump << instr_1.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << "0" << bitset<16>(LUT_Value) << bitset<4>(instr_1.Data_Mem_Select)
       << bitset<9>(toBinary(instr_1.Operand_Addr[0])) << bitset<9>(toBinary(instr_1.Operand_Addr[1])) << bitset<9>(toBinary(instr_1.Operand_Addr[2])) << bitset<9>(toBinary(instr_1.Operand_Addr[3]));
    cat_instr_1 = ss.str();
    return cat_instr_1;
}

string cat_instr_2(Instr_2 &instr_2)
{
    int cur_node_addr = instr_2.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_2.Node_Addr[1];
    string cat_instr_2;
    stringstream ss;
    ss << "0001" << instr_2.PC_Jump << instr_2.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(20, '0') << instr_2.Data_Mem_Select << string(27, '0') << bitset<9>(toBinary(instr_2.Operand_Addr));
    cat_instr_2 = ss.str();
    return cat_instr_2;
}

string cat_instr_3(Instr_3 &instr_3)
{
    int cur_node_addr = instr_3.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_3.Node_Addr[1];
    string cat_instr_3;
    stringstream ss;
    ss << "0010" << instr_3.PC_Jump << instr_3.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << instr_3.Static_Binary_Value << string(56, '0');
    cat_instr_3 = ss.str();
    return cat_instr_3;
}

string cat_instr_4(Instr_4 &instr_4)
{
    int cur_node_addr = instr_4.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_4.Node_Addr[1];
    string cat_instr_4;
    stringstream ss;
    ss << "0011" << instr_4.PC_Jump << instr_4.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(57, '0');
    cat_instr_4 = ss.str();
    return cat_instr_4;
}

string cat_instr_5(Instr_5 &instr_5)
{
    int cur_node_addr = instr_5.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_5.Node_Addr[1];
    string cat_instr_5;
    stringstream ss;
    ss << "0100" << instr_5.PC_Jump << instr_5.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(13, '0') << "00" << instr_5.data_mem_select << "0" << instr_5.value << "0" << instr_5.F << instr_5.T << string(18, '0') << bitset<9>(toBinary(instr_5.Operand_Addr)) << string(9, '0');
    cat_instr_5 = ss.str();
    return cat_instr_5;
}

string cat_instr_6(Instr_6 &instr_6)
{
    int cur_node_addr = instr_6.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_6.Node_Addr[1];
    string cat_instr_6;
    stringstream ss;
    ss << "0101" << instr_6.PC_Jump << instr_6.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(13, '0') << "0" << instr_6.data_mem_select2 << instr_6.data_mem_select1 << "0" << instr_6.value << "00" << instr_6.T << string(9, '0') << bitset<9>(toBinary(instr_6.F_Addr)) << bitset<9>(toBinary(instr_6.Operand_Addr)) << string(9, '0');
    cat_instr_6 = ss.str();
    return cat_instr_6;
}

string cat_instr_7(Instr_7 &instr_7)
{
    int cur_node_addr = instr_7.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_7.Node_Addr[1];
    string cat_instr_7;
    stringstream ss;
    ss << "0110" << instr_7.PC_Jump << instr_7.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(13, '0') << "0" << instr_7.data_mem_select3 << instr_7.data_mem_select1 << instr_7.data_mem_select2 << instr_7.value << "000" << string(9, '0') << bitset<9>(toBinary(instr_7.F_Addr)) << bitset<9>(toBinary(instr_7.Operand_Addr)) << bitset<9>(toBinary(instr_7.T_Addr));
    cat_instr_7 = ss.str();
    return cat_instr_7;
}

string cat_instr_8(Instr_8 &instr_8)
{
    int cur_node_addr = instr_8.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_8.Node_Addr[1];
    string cat_instr_8;
    stringstream ss;
    ss << "0111" << instr_8.PC_Jump << instr_8.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(13, '0') << "00" << instr_8.data_mem_select1 << instr_8.data_mem_select2 << instr_8.value << "0" << instr_8.F << "0" << string(18, '0') << bitset<9>(toBinary(instr_8.Operand_Addr)) << bitset<9>(toBinary(instr_8.T_Addr));
    cat_instr_8 = ss.str();
    return cat_instr_8;
}

string cat_instr_9(Instr_9 &instr_9)
{
    int cur_node_addr = instr_9.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_9.Node_Addr[1];
    string cat_instr_9;
    stringstream ss;
    ss << "1000" << instr_9.PC_Jump << instr_9.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(20, '0') << instr_9.im << string(36, '0');
    cat_instr_9 = ss.str();
    return cat_instr_9;
}

string cat_instr_10(Instr_10 &instr_10)
{
    int cur_node_addr = instr_10.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_10.Node_Addr[1];
    string cat_instr_10;
    stringstream ss;
    ss << "1001" << instr_10.PC_Jump << instr_10.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(13, '0') << "000" << instr_10.data_mem_select << string(31, '0') << bitset<9>(toBinary(instr_10.Addr));
    cat_instr_10 = ss.str();
    return cat_instr_10;
}

string cat_instr_11(Instr_11 &instr_11)
{
    int cur_node_addr = instr_11.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_11.Node_Addr[1];
    string cat_instr_11;
    stringstream ss;
    ss << "1010" << instr_11.PC_Jump << instr_11.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(13, '0') << "00" << instr_11.data_mem_select << "0" << instr_11.value << "00" << instr_11.T << string(18, '0') << bitset<9>(toBinary(instr_11.Operand_Addr)) << string(9, '0');
    cat_instr_11 = ss.str();
    return cat_instr_11;
}

string cat_instr_12(Instr_12 &instr_12)
{
    int cur_node_addr = instr_12.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_12.Node_Addr[1];
    string cat_instr_12;
    stringstream ss;
    ss << "1011" << instr_12.PC_Jump << instr_12.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(13, '0') << "00" << instr_12.data_mem_select1 << instr_12.data_mem_select2 << instr_12.value << "000" << string(18, '0') << bitset<9>(toBinary(instr_12.Operand_Addr)) << bitset<9>(toBinary(instr_12.T_Addr));
    cat_instr_12 = ss.str();
    return cat_instr_12;
}

string cat_instr_13(Instr_13 &instr_13)
{
    int cur_node_addr = instr_13.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_13.Node_Addr[1];
    string cat_instr_13;
    stringstream ss;
    ss << "1100" << instr_13.PC_Jump << instr_13.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(13, '0') << instr_13.data_mem_select2 << "0" << instr_13.data_mem_select1 << "0" << instr_13.value1 << instr_13.value2 << instr_13.T2 << instr_13.T1 << bitset<9>(toBinary(instr_13.Operand_Addr2)) << string(9, '0') << bitset<9>(toBinary(instr_13.Operand_Addr1)) << string(9, '0');
    cat_instr_13 = ss.str();
    return cat_instr_13;
}

string cat_instr_14(Instr_14 &instr_14)
{
    int cur_node_addr = instr_14.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_14.Node_Addr[1];
    string cat_instr_14;
    stringstream ss;
    ss << "1101" << instr_14.PC_Jump << instr_14.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(13, '0') << instr_14.data_mem_select2 << instr_14.data_mem_select3 << instr_14.data_mem_select1 << "0" << instr_14.value1 << instr_14.value2 << "0" << instr_14.T1 << bitset<9>(toBinary(instr_14.Operand_Addr2)) << bitset<9>(toBinary(instr_14.T2_Addr)) << bitset<9>(toBinary(instr_14.Operand_Addr1)) << string(9, '0');
    cat_instr_14 = ss.str();
    return cat_instr_14;
}

string cat_instr_15(Instr_15 &instr_15)
{
    int cur_node_addr = instr_15.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_15.Node_Addr[1];
    string cat_instr_15;
    stringstream ss;
    ss << "1110" << instr_15.PC_Jump << instr_15.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(13, '0') << instr_15.data_mem_select3 << "0" << instr_15.data_mem_select1 << instr_15.data_mem_select2 << instr_15.value1 << instr_15.value2 << instr_15.T2 << "0" << bitset<9>(toBinary(instr_15.Operand_Addr2)) << string(9, '0') << bitset<9>(toBinary(instr_15.Operand_Addr1)) << bitset<9>(toBinary(instr_15.T1_Addr));
    cat_instr_15 = ss.str();
    return cat_instr_15;
}

string cat_instr_16(Instr_16 &instr_16)
{
    int cur_node_addr = instr_16.Node_Addr[0] * N_PROCESSORS_PER_CLUSTER + instr_16.Node_Addr[1];
    string cat_instr_16;
    stringstream ss;
    ss << "1111" << instr_16.PC_Jump << instr_16.BM_Jump << string(9, '0') << bitset<8>(toBinary(cur_node_addr)) << string(13, '0') << instr_16.data_mem_select3 << instr_16.data_mem_select4 << instr_16.data_mem_select1 << instr_16.data_mem_select2 << instr_16.value1 << instr_16.value2 << "00" << bitset<9>(toBinary(instr_16.Operand_Addr2)) << bitset<9>(toBinary(instr_16.T2_Addr)) << bitset<9>(toBinary(instr_16.Operand_Addr1)) << bitset<9>(toBinary(instr_16.T1_Addr));
    cat_instr_16 = ss.str();
    return cat_instr_16;
}