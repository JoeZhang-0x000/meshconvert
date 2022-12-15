//
// Created by zhangx on 2022/12/14.
//

#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <iomanip>
#include "string"
#include "sstream"
#include "map"
#include<vector>
#include "fstream"
#include<cstring>
#include"utils.h"
#include<mpi.h>

using namespace std;

int DIM = 3; //default dimension

int NDIM = 3; //default nodes dimension

int NID = 1; // global node id

int EID = 1; // global element id

int WID = 10; //default span, used in setw(WID)

bool ISPRINT = false; // if true, print something,

int RANK = 0; // 默认当前进程编号

int SIZE = 1; // 默认进程数


MPI_Status status;

// # geometry      id   vertices
// # POINT       = 0    1
// # SEGMENT     = 1    2
// # TRIANGLE    = 2    3
// # SQUARE      = 3    4
// # TETRAHEDRON = 4    4
// # CUBE        = 5    8
// # PRISM       = 6    9
// # PYRAMID     = 7    5
map<string, int> GEOM = {{"POINT",       0},
                         {"SEGMENT",     1},
                         {"TRIANGLE",    2},
                         {"SQUARE",      3},
                         {"TETRAHEDRON", 4},
                         {"CUBE",        5},
                         {"PRISM",       6},
                         {"PYRAMID",     7}}; // dictionary, k:the number of nodes, v:geom index in mfem
map<int, string> IGEOM = {{0, "POINT"},
                          {1, "SEGMENT"},
                          {2, "TRIANGLE"},
                          {3, "SQUARE"},
                          {4, "TETRAHEDRON"},
                          {5, "CUBE"},
                          {6, "PRISM"},
                          {7, "PYRAMID"}}; // inverse GEOM
map<int, string> GEOM2D = {{1, "POINT"},
                           {2, "SEGMENT"},
                           {3, "TRIANGLE"},
                           {4, "SQUARE"}}; // k:vertices number,v: geometry
map<string, int> IGEOM2D = {{"POINT",    1},
                            {"SEGMENT",  2},
                            {"TRIANGLE", 3},
                            {"SQUARE",   4}}; // inverse GEOM2D
map<int, string> GEOM3D = {{4, "TETRAHEDRON"},
                           {8, "CUBE"},
                           {6, "PRISM"},
                           {5, "PYRAMID"}};
map<string, int> IGEOM3D = {{"TETRAHEDRON", 4},
                            {"CUBE",        8},
                            {"PRISM",       6},
                            {"PYRAMID",     5}}; // inverse GEOM3D

class Node {
public:
    double id, x, y, z;
public:
    Node(int= 0, double= 0.f, double= 0.f, double= 0.f);

    string to_write_oofem();

    string to_write_mfem();
};

Node::Node(int id, double x, double y, double z) {
    this->id = id;
    this->x = x;
    this->y = y;
    this->z = z;
}


string Node::to_write_oofem() {
    stringstream tmp;
    if (DIM == 3)
        tmp << "node" << " " << int(id) << " " << "coords" << " " << "3" << " " << to_string(x) << " " << to_string(y)
            << " " << to_string(z) << "\n";
    else
        tmp << "node" << " " << int(id) << " " << "coords" << " " << "2" << " " << to_string(x) << " " << to_string(y)
            << "\n";
    string res = tmp.str();
    return res;
}

string Node::to_write_mfem() {
    stringstream tmp;
    if (NDIM == 3)
        tmp << setw(WID) << to_string(x) << " " << setw(WID) << to_string(y) << " " << setw(WID) << to_string(z)
            << "\n";
    else
        tmp << setw(WID) << to_string(x) << " " << setw(WID) << to_string(y) << "\n";
    string res = tmp.str();
    return res;
}


class Element {
public:
    int id;
    vector<int> v_node; // storage points index
public:
    Element(int);

    void add(int);

    string to_write_oofem();

    string to_write_mfem();
};

Element::Element(int id) {
    this->id = id;
}

void Element::add(int n) {
    this->v_node.push_back(n);
}

string Element::to_write_oofem() {
    stringstream tmp;
    tmp << "lspace" << " " << int(id) << " " << "nodes" << " " << v_node.size() << " ";
    for (auto i: v_node) {
        tmp << to_string(i) << " ";
    }
    tmp << endl;
    string res = tmp.str();
    return res;
}

string Element::to_write_mfem() {
    stringstream tmp;
    int gem = -1;
    if (DIM == 3)
        gem = GEOM[GEOM3D[int(v_node.size())]];
    else
        gem = GEOM[GEOM2D[int(v_node.size())]];
    // tmp<<int(id)<<" ";
    tmp << setw(WID) << 1 << " " << setw(WID) << gem << " ";
    for (auto i: v_node) {
        tmp << setw(WID) << to_string(i) << " ";
    }
    tmp << endl;
    string res = tmp.str();
    return res;
}

#endif //STRUCTURE_H
