#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <metis.h>


/**
 * @brief A file for common types and data structures used by compiler.
 **/

#ifndef Compile_H
#define Compile_H

using namespace std;

#define MAX_CLUSTERS 48
#define N_PROCESSORS_PER_CLUSTER 64
#define N_INS_PER_PROCESSOR 512
#define MEM_DEPTH 512
#define N_LUTS_PER_PROCESSOR 100

typedef struct
{
    // string opcode;
    string LUT_Value;
    vector<int> Node_Addr;
    string Data_Mem_Select;
    vector<int> Operand_Addr;
} Instr_1; // logic mapping instruction

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    string Data_Mem_Select;
    int Operand_Addr;
} Instr_2; // data mem access instruction

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    string Static_Binary_Value;
} Instr_3; // static configuration instruction

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
} Instr_4; // idle mode instruction

typedef struct
{
    vector<int> inter_mem;
    vector<int> exter_mem;
    vector<string> instr_mem;
} Processor;

map<int, vector<string>> instr;
map<int, Processor> processors;
// string par_out_path = "../test/test_2/partition.txt";
// string instr_out_path = "../test/test_2/instr_out.txt";
// string out_dir_path = "../test/test_2/out_dir.txt";

vector<idx_t> part_func(vector<idx_t> &xadj, vector<idx_t> &adjncy, /*vector<idx_t> &adjwgt, */ decltype(METIS_PartGraphKway) *METIS_PartGraphFunc);
string toBinary(int n);
string HextoBinary(string HexDigit);
bool cmp(pair<int, int> a, pair<int, int> b);
string cat_instr_1(Instr_1 &instr_1);
string cat_instr_2(Instr_2 &instr_2);
string cat_instr_3(Instr_3 &instr_3);
string cat_instr_4(Instr_4 &instr_4);

#endif