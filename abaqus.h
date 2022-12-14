//
// Created by zhangx on 2022/12/14.
//

#ifndef ABAQUS_H
#define ABAQUS_H
#include "structure.h"

void readAbaqus(ifstream &fin, vector<Node *> &nodeList, vector<Element *> &elementList) {
    char data[1024]; // buffer, is used when read something

    int flag = -1; // default -1 represents reading plain text, otherwise corresponds to flags
    const string flags[] = {
            "Node",
            "Element",
    }; // the key words list, e.g. flag = 1 means read "Node" type content

    int loc = 0; // the number of lines have been read

    while (!fin.eof()) {
        // each time read only one line, if it contains keywords, do something special
        loc++;
        fin.getline(data, 1024);
        if (strlen(data) == 0) //empty
            continue;
        auto[s, len] = split(data);

        if (s[0][0] != '*')
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
                default:
                    break;
            }
        else {
            flag = -1; // reset flag to -1
            for (int i = 0; i < sizeof(flags) / sizeof(flags[0]); i++) {
                if (find(s[0], (char *) (flags[i].c_str())) != -1) {
                    if (ISPRINT)
                        printf("at %d,read %s\n", loc, flags[i].c_str());
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
