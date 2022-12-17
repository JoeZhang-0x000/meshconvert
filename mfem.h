//
// Created by zhangx on 2022/12/14.
//

#ifndef MFEM_H
#define MFEM_H
#include "structure.h"

void readMfem(ifstream &fin, vector<Node *> &nodeList, vector<Element *> &elementList, vector<Element *> &boundaryList) {
    nodeList.clear(), elementList.clear();
    char data[1024]; // buffer, is used when read something

    int flag = -1; // default -1 represents reading plain text, otherwise corresponds to flags

    int loc = 0; // the number of lines have been read

    while (!fin.eof()) {
        // each time read only one line, if it contains keywords, do something special
        loc++;
        fin.getline(data, 1024);
        if (strlen(data) == 0) //empty
            continue;
        auto[s, len] = split(data);

        if (s[0][0] == 'd') { // dimension
            flag = 0;
            continue;
        } else if (s[0][0] == 'e') {// elements
            flag = 1;
            fin.getline(data, 1024); // read elements number
            continue;
        } else if (s[0][0] == 'b') { // boundaries
            flag = 2;
            fin.getline(data, 1024); // read boundaries number
            continue;
        } else if (s[0][0] == 'v') { // vertices
            flag = 3;
            fin.getline(data, 1024); // read vertices number
            fin.getline(data, 1024); // read vertices dim
            NDIM = stoi(data);
            continue;
        }
        switch (flag) {
            case -1: {
                break;
            }
            case 0: {
                // read dimension
                // just read next line
                DIM = stoi(data);
                break;
            }
            case 1: { // read Element
                // data type:
                // attribute,geometry,point 1,point 2,...
                Element *element = new Element(EID++); //EID is the global indices of elements
                for (int i = 2; i < len; i++) { // just read points
                    element->add(stoi(s[i]));
                }
                elementList.push_back(element);
                break;
            }
            case 2: { // read boundary
                // data type:
                // attribute,geometry,point 1,point 2,...
                Element *element = new Element(EID++); //EID is the global indices of elements
                for (int i = 2; i < len; i++) { // just read points
                    element->add(stoi(s[i]));
                }
                boundaryList.push_back(element);
                break;
            }
            case 3: { // read vertices
                // data type:
                // x,y,z
                Node *node = new Node(NID++, stod(s[0]), stod(s[1]), NDIM == 2 ? 0.f : stod(s[2]));
                nodeList.push_back(node);
                break;
            }
            default:
                break;
        }
    }
}

void writeMfem(fstream &fout, vector<Node *> &nodeList, vector<Element *> &elementList, vector<Element *> &boundaryList) {
    fout << "MFEM mesh v1.x\n";
    fout << endl;
    fout << "dimension\n";
    fout << DIM << "\n";
    fout << endl;
    fout << "elements\n";
    fout << elementList.size() << endl;
    for (auto i: elementList) {
        fout << i->to_write_mfem();
    }
    fout << endl;

    fout << "boundary\n";
    DIM = 2;
    fout << boundaryList.size() << endl;
    for (auto i: boundaryList) {
        fout << i->to_write_mfem();
    }
    fout << endl;

    fout << endl;
    fout << "vertices\n";
    fout << nodeList.size() << endl;
    fout << NDIM << endl;
    for (auto i: nodeList) {
        fout << i->to_write_mfem();
    }
}

#endif //MFEM_H
