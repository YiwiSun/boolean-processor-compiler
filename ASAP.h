#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "VCDTypes.h"

#ifndef ASAP_H
#define ASAP_H

using namespace std;

/**
 * @brief ASAP Levelization
 **/
class ASAP {
public:
    //! Instance a new Pre-processing Class
    ASAP();

    //! Destructor
    ~ASAP();

    void GenAdjList(map<int, LutType> &luts, map<string, vector<int>> &net_for_id, map<int, vector<int>> &adjList);
    void SetInDegrees(map<int, vector<int>> &adjList, vector<int> &indegrees);
    void TopologyOrder(map<int, vector<int>> &adjList, vector<int> &indegrees, int &n, vector<vector<int>> &levels);

    void ASAP_Levelization(map<int, LutType> &luts, map<string, vector<int>> &net_for_id, int &n);

    /* data */
    std::vector<std::vector<int>> levels;
};

#endif