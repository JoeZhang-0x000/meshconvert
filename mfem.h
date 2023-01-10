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

void writeMfemElement(string filename,AbaqusData* abaqusData){
    auto op = ios::app;
    ofstream fout(filename,op);
    vector<Element*> elementList = abaqusData->elementList;
    if(RANK==0){
        fout<<"elements\n";
        fout<<ELSIZE<<endl;
    }
//    fout<<"#e"<<RANK<<endl;
    for(auto itr : elementList){
        fout<<itr->to_write_mfem();
    }
    fout.close();
}

void writeMfemBoundary(string filename,AbaqusData* abaqusData){
    auto op = ios::app;
    ofstream fout(filename,op);
    vector<Nset*> nsetList = abaqusData->nsetList;
    if(RANK==0){
        fout<<"boundary\n";
        fout<<NSETSIZE<<endl;
    }
//    fout<<"#b"<<RANK<<endl;
    for(int i=0;i<nsetList.size();i++){
        fout<<nsetList[i]->to_write_mfem();
        ATT ++;
    }
    fout.close();
}

void writeMfemVertices(string filename,AbaqusData* abaqusData){
    auto op = ios::app;
    ofstream fout(filename,op);
    vector<Node *> nodeList = abaqusData->nodeList;
    if(RANK==0){
        fout<<"vertices\n";
        fout<<NSIZE<<endl;
        fout<<DIM<<endl;
    }
//    fout<<"#v"<<RANK<<endl;
    for(auto itr : nodeList){
        fout<<itr->to_write_mfem();
    }
    fout.close();
}
void writeMfem(string filename,AbaqusData *abaqusData) {
    NSIZE = abaqusData->nodeList.size();
    ELSIZE = abaqusData->elementList.size();
    for(int i=0;i<abaqusData->nsetList.size();i++){ // 因为所有边界都用线段表示了，所以每个进程存的边界就是线段的数量，线段的数量应等于点集数量-1，
        NSETSIZE += abaqusData->nsetList[i]->ids.size()-1;

    }
    int tn,te,tns; // 临时变量，MPI 归约用
    MPI_Reduce(&NSIZE,&tn,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&ELSIZE,&te,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&NSETSIZE,&tns,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    NSIZE = tn;
    ELSIZE = te;
    NSETSIZE = tns;
    if(RANK==0){
        auto op = ios::trunc;
        ofstream fout(filename,op);
        fout << "MFEM mesh v1.0\n";
        fout << endl;
        fout<<"#\n"
              "# MFEM Geometry Types (see mesh/geom.hpp):\n"
              "#\n"
              "# POINT       = 0\n"
              "# SEGMENT     = 1\n"
              "# TRIANGLE    = 2\n"
              "# SQUARE      = 3\n"
              "# TETRAHEDRON = 4\n"
              "# CUBE        = 5\n"
              "#"<<endl;
        fout << "dimension\n";
        fout << DIM << "\n";
        fout << endl;
        fout.close();
    }
    int ok;
    MPI_Status status;
    int left = MPI_PROC_NULL;
    int right = MPI_PROC_NULL;
    if (RANK != 0)
        left = RANK - 1;
    if (RANK != SIZE - 1)
        right = RANK + 1;

    MPI_Recv(&ok, 1, MPI_INT, left, 0, MPI_COMM_WORLD, &status);
    writeMfemElement(filename,abaqusData);
    MPI_Send(&ok, 1, MPI_INT, right, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Recv(&ATT, 1, MPI_INT, left, 1, MPI_COMM_WORLD, &status); // 这里交换了ATT信息给下一个进程
    writeMfemBoundary(filename,abaqusData);
    MPI_Send(&ATT, 1, MPI_INT, right, 1, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Recv(&ok, 1, MPI_INT, left, 2, MPI_COMM_WORLD, &status);
    writeMfemVertices(filename,abaqusData);
    MPI_Send(&ok, 1, MPI_INT, right, 2, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);


}

#endif //MFEM_H
