#include <iostream>
#include <fstream> 
#include <cstring>
#include <chrono>
#include <algorithm>
#include <thread>
#include <limits>
#include <cstddef>
#include <queue>
// #define NDEBUG
#include <cassert>

#include "ASAP.h"

using namespace std;

/**
 * @brief ASAP Levelization
 **/

//! Instance a new Pre-processing Class
ASAP::ASAP() {

}

//! Destructor
ASAP::~ASAP() {

}

void ASAP::ASAP_Levelization(map<int, LutType> &luts, map<string, vector<int>> &net_for_id, int &n)
{
    map<int, vector<int>> adjList; // vertex : list of edges
    vector<int> indegrees(n);
    GenAdjList(luts, net_for_id, adjList);
    SetInDegrees(adjList, indegrees);
    TopologyOrder(adjList, indegrees, n, levels);
    cout << "levels size: " << levels.size() << endl;
    // debug
    for (auto l = levels.begin(); l != levels.end(); l++)
    {
        cout << "level" << distance(levels.begin(), l) << ": " << endl;
        for (auto i = l->begin(); i != l->end(); i++)
        {
            cout << *i << " ";
        }
        cout << endl;
    }
}

void ASAP::GenAdjList(map<int, LutType> &luts, map<string, vector<int>> &net_for_id, map<int, vector<int>> &adjList)
{
    int n = luts.size();
    for (unsigned i = 0; i < n; i++)
    {
        LutType cur_lut = luts.find(i)->second;
        string cur_out = cur_lut.out_ports;
        if (net_for_id.find(cur_out) != net_for_id.end())
        {
            adjList[i] = net_for_id[cur_out];
        }
        sort(adjList[i].begin(), adjList[i].end());
        adjList[i].erase(unique(adjList[i].begin(), adjList[i].end()), adjList[i].end());
    }
    // debug
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

void ASAP::SetInDegrees(map<int, vector<int>> &adjList, vector<int> &indegrees)
{
    for (map<int, vector<int>>::iterator i = adjList.begin(); i != adjList.end(); i++)
    {
        for (int j = 0; j < i->second.size(); j++)
        {
            indegrees[i->second[j]] += 1;
        }
    }
}

void ASAP::TopologyOrder(map<int, vector<int>> &adjList, vector<int> &indegrees, int &n,
                         vector<vector<int>> &levels)
{
    vector<int> Q;
    vector<int> tmp_Q;
    for (int i = 0; i < n; i++)
    {
        if (indegrees[i] == 0)
        {
            Q.push_back(i);
        }
    }
    while (!Q.empty())
    {
        levels.push_back(Q);
        for (int i = 0; i < Q.size(); i++)
        {
            n--;
            int tmp = Q[i];
            for (int j = 0; j < adjList[tmp].size(); j++)
            {
                int v = adjList[tmp][j];
                indegrees[v] -= 1;
                if (indegrees[v] == 0)
                {
                    tmp_Q.push_back(v);
                }
            }
        }
        Q = tmp_Q;
        tmp_Q.clear();
    }
    assert(n <= 0);
}