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

#define MAX_CLUSTERS 8
#define N_PROCESSORS_PER_CLUSTER 64
#define N_INS_PER_PROCESSOR 512
#define MEM_DEPTH 512
#define N_LUTS_PER_PROCESSOR 14
#define INTER_CLUSTER_CLK 3
#define ACR_CLUSTER_CLK 3
#define MEMORY_INITIALIZATION_RADIX 2

typedef struct
{
    // string opcode;
    string LUT_Value;
    vector<int> Node_Addr;
    string Data_Mem_Select;
    vector<int> Operand_Addr;
    int PC_Jump;
    int BM_Jump;    // benchmark initial input jump
} Instr_1; // logic mapping instruction

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    string Data_Mem_Select;
    int Operand_Addr;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_2; // data mem access instruction

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    string Static_Binary_Value;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_3; // static configuration instruction

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_4; // idle mode instruction

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int Operand_Addr;
    int data_mem_select;
    int value;
    int T;
    int F;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_5;
// DFF instruction(if...else...)
// Judgment success: Assign an immediate number
// Judgment failed : Assign an immediate number

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int Operand_Addr;
    int data_mem_select1;
    int value;
    int T;
    int F_Addr;
    int data_mem_select2;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_6;
// DFF instruction(if...else...)
// Judgment success: Assign an immediate number
// Judgment failed : Assign data mem address

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int Operand_Addr;
    int data_mem_select1;
    int value;
    int T_Addr;
    int data_mem_select2;
    int F_Addr;
    int data_mem_select3;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_7;
// DFF instruction(if...else...)
// Judgment success: Assign an data mem address
// Judgment failed : Assign an data mem address

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int Operand_Addr;
    int data_mem_select1;
    int value;
    int T_Addr;
    int data_mem_select2;
    int F;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_8;
// DFF instruction(if...else...)
// Judgment success: Assign data mem address
// Judgment failed : Assign an immediate number

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int im;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_9;
// DFF instruction(Direct assignment)
// Assign an immediate number

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int Addr;
    int data_mem_select;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_10;
// DFF instruction(Direct assignment)
// Assign data mem address

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int Operand_Addr;
    int data_mem_select;
    int value;
    int T;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_11;
// DFF instruction(if...)
// Judgment success: Assign an immediate number
// Judgment failed : Keep the last state

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int Operand_Addr;
    int data_mem_select1;
    int value;
    int T_Addr;
    int data_mem_select2;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_12;
// DFF instruction(if...)
// Judgment success: Assign data mem address
// Judgment failed : Keep the last state

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int Operand_Addr1;
    int data_mem_select1;
    int value1;
    int T1;
    int Operand_Addr2;
    int data_mem_select2;
    int value2;
    int T2;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_13;
// DFF instruction(if...else if...)
// Judgment success1: Assign an immediate number
// Judgment success2: Assign an immediate number

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int Operand_Addr1;
    int data_mem_select1;
    int value1;
    int T1;
    int Operand_Addr2;
    int data_mem_select2;
    int value2;
    int T2_Addr;
    int data_mem_select3;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_14;
// DFF instruction(if...else if...)
// Judgment success1: Assign an immediate number
// Judgment success2: Assign data mem address

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int Operand_Addr1;
    int data_mem_select1;
    int value1;
    int T1_Addr;
    int data_mem_select2;
    int Operand_Addr2;
    int data_mem_select3;
    int value2;
    int T2;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_15;
// DFF instruction(if...else if...)
// Judgment success1: Assign data mem address
// Judgment success2: Assign an data mem address

typedef struct
{
    // string opcode;
    vector<int> Node_Addr;
    int Operand_Addr1;
    int data_mem_select1;
    int value1;
    int T1_Addr;
    int data_mem_select2;
    int Operand_Addr2;
    int data_mem_select3;
    int value2;
    int T2_Addr;
    int data_mem_select4;
    int PC_Jump;
    int BM_Jump; // benchmark initial input jump
} Instr_16;
// DFF instruction(if...else if...)
// Judgment success1: Assign data mem address
// Judgment success2: Assign data mem address

typedef struct
{
    int dff_num;
    vector<int> LUT_Node_Addr;
    int LUT_Instr_Addr;
    pair<int, int> Replaced_Instr_Range;
} blank_addr; // input from the output of DFFs

typedef struct
{
    // vector<int> inter_mem;
    // vector<int> exter_mem;
    vector<string> instr_mem;
    pair<int, int> id_outaddr; // lut_id : output_instr_addr
} Processor;

// map<int, vector<string>> instr;
map<int, Processor> processors;             // processor_id : Processor
map<int, vector<blank_addr>> blank_addrs;   // <lut_num, vector<blank_addr>>

vector<idx_t> part_func(vector<idx_t> &xadj, vector<idx_t> &adjncy, /*vector<idx_t> &adjwgt, */ decltype(METIS_PartGraphKway) *METIS_PartGraphFunc);
string toBinary(int n);
string HextoBinary(string HexDigit);
bool cmp(pair<int, int> a, pair<int, int> b);
string cat_instr_1(Instr_1 &instr_1);
string cat_instr_2(Instr_2 &instr_2);
string cat_instr_3(Instr_3 &instr_3);
string cat_instr_4(Instr_4 &instr_4);
string cat_instr_5(Instr_5 &instr_5);
string cat_instr_6(Instr_6 &instr_6);
string cat_instr_7(Instr_7 &instr_7);
string cat_instr_8(Instr_8 &instr_8);
string cat_instr_9(Instr_9 &instr_9);
string cat_instr_10(Instr_10 &instr_10);
string cat_instr_11(Instr_11 &instr_11);
string cat_instr_12(Instr_12 &instr_12);
string cat_instr_13(Instr_13 &instr_13);
string cat_instr_14(Instr_14 &instr_14);
string cat_instr_15(Instr_15 &instr_15);
string cat_instr_16(Instr_16 &instr_16);

#endif





















