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

int STARTLINE = 0; // 默认开始读位置

int ENDLINE = INT_MAX; //默认结束读位置

int NSIZE; //总地节点数量

int ELSIZE; // 总地单元数量

int ELSETSIZE; //总地elset数量

int NSETSIZE; // 总地nset数量

int ATT; //用以作为偏移量，计算全局的编号




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

    string to_write_debug();

    string to_write_mfem();

};

Node::Node(int id, double x, double y, double z) {
    this->id = id;
    this->x = x;
    this->y = y;
    this->z = z;
}

string Node::to_write_debug() {
    stringstream tmp;
    if (DIM == 3)
        tmp << int(id)<< " " << to_string(x) << " " << to_string(y)
            << " " << to_string(z) << "\n";
    else
        tmp << int(id) <<" " << to_string(x) << " " << to_string(y)
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
    string type; //单员类型
    vector<int> v_node; // storage points index
public:
    Element(int);

    void add(int);

    string to_write_debug();

    string to_write_mfem();
};

Element::Element(int id) {
    this->id = id;
}

void Element::add(int n) {
    this->v_node.push_back(n);
}

string Element::to_write_debug() {
    stringstream tmp;
    tmp << int(id) << " " ;
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
    tmp <<" " << 1 << " " << gem << " ";
    for (auto i: v_node) {
        tmp << " "<< to_string(i-1) << " ";
    }
    tmp << endl;
    string res = tmp.str();
    return res;
}

class Nset{
public:
    string name;
    int att; //在每个进程内部的编号,
    vector<int> ids; // nodes ids
public:
    Nset(string name = "no name"){
        this->name = name;
    }
    string to_write_debug(){
        stringstream tmp;
        tmp<<"*Nset,Nset="<<name<<endl;
        for(int i:ids){
            tmp<<i<<" ";
        }
        tmp<<endl;
        return tmp.str();
    }
    string to_write_mfem(){
        stringstream  tmp;
        // 认为boundary是由abaqus Nset构成，其中abaqus一个nset的点连接起来就是边界
        for(int i=0;i<ids.size()-1;i++){
            tmp<<ATT<<" "<<"1"<<" "<<ids[i]-1<<" "<<ids[i+1]-1<<endl;
        }
//        tmp<<endl;
        return tmp.str();
    }
};

class Elset{
public:
    string name;
    vector<int> ids; // elements ids
public:
    Elset(string name){
        this->name = name;
    }

    string to_write_debug(){
        stringstream tmp;
        tmp<<"*Elset,Elset="<<name<<endl;
        for(int i:ids){
            tmp<<i<<" ";
        }
        tmp<<endl;
        return tmp.str();
    }

};

/**
 * 总的数据类型类，abaqusdata继承自它
 */
class Data{
public:
};

class AbaqusData : public Data {
public:
    vector<Node *> nodeList;
    vector<Element *> elementList;
    vector<Elset *> elsetList;
    vector<Nset *> nsetList;
};

#endif //STRUCTURE_H
