//
// Created by zhangx on 2022/12/14.
//

#ifndef ABAQUS_H
#define ABAQUS_H

#include "structure.h"



const string flags[] = {
        "*Node",
        "*Element",
        "*Nset",
        "*Elset",
}; // the key words list, e.g. flag = 1 means read "Node" type content

/**
 * 扫描整个文件，
 * @param fin
 */
int scanAbaqus(ifstream &fin) {
    string buff;
    int loc = 0; //从0行开始读
    while (getline(fin, buff)) {
        loc++;
    }
    return loc; // 返回总行数
}

/**
 * 读abaqus 文件指定行数
 * @param fin
 * @param nodeList
 * @param elementList
 * @param elsetList
 * @param nestList
 * @param startLine
 * @param endLine
 */
void readAbaqus(ifstream &fin,AbaqusData *abaqusData,
                int startLine = 0, int endLine = INT_MAX) {
    vector<Node *> *nodeList = &abaqusData->nodeList;
    vector<Element *> *elementList = &abaqusData->elementList;
    vector<Elset *> *elsetList = &abaqusData->elsetList;
    vector<Nset *> *nestList = &abaqusData->nsetList;
    string buff;
    int loc = 0;
    int flag = -1; // default -1 represents reading plain text, otherwise corresponds to flags
    int isdata = 0; // 1 表示读数据行，0 表示读的是文本行
    int op = -1; // 特殊操作例如 generate
    Nset *nset; // 指向vector中元素
    Elset *elset; // 指向vector中元素
    fin.clear();
    fin.seekg(0, ios::beg);
    while (getline(fin, buff)) {
        loc++;
        if (loc == endLine)
            break;
        if (buff.empty()) //empty
            continue;
        auto[s, len] = split(&buff[0]); // 分割字符串
        isdata = 1;
        if (buff[0] == '*') { // *后面是关键字，但是 **后面带的是注释
            flag = -1; // 默认flag，提高容错率
            for (int i = 0; i < sizeof(flags) / sizeof(flags[0]); i++) // 匹配关键字
                if (flags[i].compare(s[0]) == 0)
                    flag = i;
            isdata = 0; // 表示这一行不是数据行
        }
        if (loc < startLine)
            continue;
        switch (flag) {
            case -1: {
                break;
            }
            case 0: { // read Node
                if (isdata) {
                    // data type:
                    // id,x,y,z
                    NDIM = len - 1;
                    Node *node = new Node(stoi(s[0]), stod(s[1]), stod(s[2]), NDIM == 2 ? 0.f : stod(s[3]));
                    nodeList->push_back(node);
                } else {

                }
                break;
            }
            case 1: { // read Element
                if (isdata) {
                    // data type:
                    // id,point 1,point 2,...
                    Element *element = new Element(stoi(s[0]));
                    for (int i = 1; i < len; i++) {
                        element->add(stoi(s[i]));
                    }
                    elementList->push_back(element);
                } else {

                }
                break;
            }
            case 2: { // read Nset
                if (isdata) {
                    if (op == 0) { // 默认读法，
                        for (int i = 0; i < len; i++) { // 整行数据都读入
                            nset->ids.push_back(stoi(s[i]));
                        }
                    } else if (op == 1) { // generate 操作，读入等差数列
                        int start = stoi(s[0]);
                        int end = stoi(s[1]);
                        int step = stoi(s[2]);
                        for (int i = start; i < end; i += step) {
                            nset->ids.push_back(i);
                        }
                    }
                    if (nset != NULL)
                        nestList->push_back(nset);
                } else {
                    // *Nset Nset=... generate
                    auto[ts, tlen] =split(s[1], "=");
                    nset = new Nset(ts[1]);
                    op = 0; // 默认读法
                    for (int i = 2; i < len; i++)
                        if (s[i][0] == 'g') // generate
                            op = 1;
                }
                break;
            }
            case 3: { // read Elset
                if (isdata) {
                    if (op == 0) { // 默认读法，
                        for (int i = 0; i < len; i++) { // 整行数据都读入
                            elset->ids.push_back(stoi(s[i]));
                        }
                    } else if (op == 1) { // generate 操作，读入等差数列
                        int start = stoi(s[0]);
                        int end = stoi(s[1]);
                        int step = stoi(s[2]);
                        for (int i = start; i < end; i += step) {
                            elset->ids.push_back(i);
                        }
                    }
                    if (nset != NULL)
                        elsetList->push_back(elset);
                } else {
                    // *Elset Elset=... generate
                    auto[ts, tlen] =split(s[1], "=");
                    elset = new Elset(ts[1]);
                    op = 0; // 默认读法
                    for (int i = 2; i < len; i++)
                        if (s[i][0] == 'g') // generate
                            op = 1;
                }
                break;
            }
            default:
                break;
        }
    }
}


/**
 * debug输出
 * @param fout
 * @param nodeList
 * @param elementList
 */
void writeDebug(string filename, AbaqusData* abaqusData) {
    auto op = (RANK==0)? ios::trunc | ios::out : ios::app;
    ofstream fout(filename,op);
    vector<Node *> *nodeList = &abaqusData->nodeList;
    vector<Element *> *elementList = &abaqusData->elementList;
    vector<Elset *> *elsetList = &abaqusData->elsetList;
    vector<Nset *> *nsetList = &abaqusData->nsetList;
    string buf;
    fout<<"**rank: "<< RANK<<" start: "<<STARTLINE<<" end: "<<ENDLINE<< endl;
    fout<<"*Node"<<endl;
    for (auto i: *nodeList) {
        fout << i->to_write_debug();
    }
    fout << endl;
    fout <<"*Element"<<endl;
    for (auto i: *elementList) {
        fout << i->to_write_debug();
    }
    fout<<endl;
    fout<<"**Nset"<<endl;
    for(auto i:*nsetList){
        fout <<i->to_write_debug();
    }
    fout<<endl;
    fout<<"**Elset"<<endl;
    for(auto i:*elsetList){
        fout<<i->to_write_debug();
    }
    fout<<endl;
}


#endif //ABAQUS_H
