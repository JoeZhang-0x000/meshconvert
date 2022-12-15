//
// Created by zhangx on 2022/12/14.
//

#ifndef ABAQUS_H
#define ABAQUS_H

#include "structure.h"

/**
 * 串行、并行读取abaqus .inp
 * 若只有一个进程，则串行读
 * 弱进程数大于1，则0号通读文件，找出定位点，指导其他进程读取的起始与结束行
 * 除0号进程外，只需要读取[starLine,endLine)之间的数据即可
 * @param fin
 * @param nodeList
 * @param elementList
 * @param startLine 开始读取的行
 * @param endLine 终止读取的行
 */

int rows[8];// rows 存储子进程读取的起止行，进程1 读取 [rows[0],rows[1]),进程2 读取 [rows[1],rows[2]) ...
int rl = 0; // 进程0 用到的一个变量，用来索引rows元素，rows[rl]
int loc = 0; // the number of lines have been read

void readAbaqus(ifstream &fin, vector<Node *> &nodeList, vector<Element *> &elementList,
                int startLine = 0, int endLine = INT_MAX) {
    char data[1024]; // buffer, is used when read something

    int flag = -1; // default -1 represents reading plain text, otherwise corresponds to flags
    const string flags[] = {
            "Node",
            "Element",
            "Nset",
            "Elset",
    }; // the key words list, e.g. flag = 1 means read "Node" type content


    while (!fin.eof() && loc < endLine) {
        // each time read only one line, if it contains keywords, do something special
        fin.getline(data, 1024);
        loc++;
        if (loc < startLine)
            continue;

        if (strlen(data) == 0) //empty
            continue;
        auto[s, len] = split(data);

            if ((s[0][0] != '*') && ((SIZE > 1 && RANK != 0) || (SIZE == 1 && RANK == 0)))
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
                    case 2:{ // read Nset


                        break;
                    }
                    case 3:{ // read Elset

                        break;
                    }
                    default:
                        break;
                }
            else {
                flag = -1; // reset flag to -1
                for (int i = 0; i < sizeof(flags) / sizeof(flags[0]); i++) {
                    if (find(s[0], (char *) (flags[i].c_str())) != -1) {
                        if (ISPRINT)
                            printf("at %d,read %s\n", loc, flags[i].c_str());
                        rows[rl++] = loc;
                        flag = i; // change flag
                    }
                }
            }
    }
}

void writeOofem(ofstream &fout, vector<Node *> &nodeList, vector<Element *> &elementList) {
    for (auto i: nodeList) {
        fout << i->to_write_oofem();
    }
    fout << endl;
    for (auto i: elementList) {
        fout << i->to_write_oofem();
    }
}


#endif //ABAQUS_H
