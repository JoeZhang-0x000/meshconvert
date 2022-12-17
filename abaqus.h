//
// Created by zhangx on 2022/12/14.
//

#ifndef ABAQUS_H
#define ABAQUS_H

#include "structure.h"

struct KEYS{
    int loc;
    int flag;
}keys[10024]; // 键值对存关键字在的行数与对应的标志,e.g. (100,2)代表从100行开始读入的是element
int ik = 0; // 索引KEYS的指针, keys[i]

const string flags[] = {
        "Node",
        "Element",
        "Nset",
        "Elset",
}; // the key words list, e.g. flag = 1 means read "Node" type content

/**
 * 扫描整个文件，找出关键字所在行
 * @param fin
 */
int scanAbaqus(ifstream &fin) {
    string buff;
    int loc = 0; //从0行开始读
    while (getline(fin, buff)) {
        loc++;
        if (buff.empty() || buff[0] != '*') // 改行为空，或者非关键字行，跳过
            continue;
        auto[s, len] = split(&buff[0]); // 分割字符串

        for (int i = 0; i < sizeof(flags) / sizeof(flags[0]); i++)
            if (find(s[0], (char *) (flags[i].c_str())) != -1)
                keys[ik++] = {loc,i};
    }
//    for(auto it : keys)
//        cout<<it.loc<<":"<<flags[it.flag]<<endl;
    return loc; // 返回总行数
}

/**
 * 读入abaqus文件指定行内容
 * @param fin
 * @param nodeList
 * @param elementList
 * @param startLine
 * @param endLine
 */
void readAbaqus(ifstream &fin, vector<Node *> &nodeList, vector<Element *> &elementList,
                int startLine = 0, int endLine = INT_MAX) {
    string buff;
    int loc = 0;
    int flag = -1; // default -1 represents reading plain text, otherwise corresponds to flags
    int p = 0; // 索引 keys[p]
    fin.clear();
    fin.seekg(0,ios::beg);
    while (getline(fin,buff)) {
        loc++;
        if(loc>endLine)
            break;
        for(auto itr : keys){
            if((loc -1)== itr.loc){
                flag = itr.flag;
                break;
            }
        }
        if (loc < startLine)
            continue;
        if (buff.empty()) //empty
            continue;
        if(buff[0]=='*')
            continue;
        auto[s, len] = split(&buff[0]);
        switch (flag) {
                case -1: {
                    break;
                }
                case 0: { // read Node
                    // data type:
                    // id,x,y,z
                    NDIM = len - 1;
                    Node *node = new Node(stoi(s[0]), stod(s[1]), stod(s[2]), NDIM == 2 ? 0.f : stod(s[3]));
                    nodeList.push_back(node);
                    break;
                }
                case 1: { // read Element
                    // data type:
                    // id,point 1,point 2,...
                    Element *element = new Element(stoi(s[0]));
                    for (int i = 1; i < len; i++) {
                        element->add(stoi(s[i]));
                    }
                    elementList.push_back(element);
                    break;
                }
                case 2: { // read Nset

                    break;
                }
                case 3: { // read Elset

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
void writeDebug(fstream &fout, vector<Node *> &nodeList, vector<Element *> &elementList,int RANK) {
    fout.clear();
    fout.seekp(0,ios::end);
    string buf;
    fout<<RANK<<endl;
    for (auto i: nodeList) {
        fout << i->to_write_oofem();
    }
    fout << endl;
    for (auto i: elementList) {
        fout << i->to_write_oofem();
    }
}


#endif //ABAQUS_H
