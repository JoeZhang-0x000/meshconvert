#include<stdio.h>
#include<iostream>
#include "structure.h"
#include "cmdline.h"
#include "mfem.h"
#include "abaqus.h"

using namespace std;

int main(int argc, char **argv) {

    // 初始化MPI
    MPI_Init(0, 0);
    MPI_Comm_rank(MPI_COMM_WORLD, &RANK);
    MPI_Comm_size(MPI_COMM_WORLD, &SIZE);

    // command decipher
    cmdline::parser a; // initialize a cmd decipher
    a.add<string>("input", 'i', "input file name", true);
    a.add<string>("output", 'o', "output file name", true);
    a.add<string>("source", 's', "source type", false, "abaqus", cmdline::oneof<string>("abaqus", "mfem"));
    a.add<string>("dest", 'd', "destination type", false, "debug", cmdline::oneof<string>("abauqs", "mfem", "debug"));
    a.add<bool>("print", 'p', "print details", false, true);
    a.add<int>("width", 'w', "write span wdith", false, 10, cmdline::range(1, 9999));
    a.parse_check(argc, argv);

    string inputFileName = a.get<string>("input").c_str();
    string outputFileName = a.get<string>("output").c_str();
    string sourceType = a.get<string>("source");
    string destType = a.get<string>("dest");
    WID = a.get<int>("width");

    if (RANK == 0) {
        cout << "**********************************" << endl;
        ctable("input:");
        cout << inputFileName << endl;
        ctable("output:");
        cout << outputFileName << endl;
        ctable("source type:");
        cout << sourceType << endl;
        ctable("destination type:");
        cout << destType << endl;
        ISPRINT = a.get<bool>("print"); // if not specify, default isprint = true
        ctable("print details:");
        cout << ISPRINT << endl;
    }

    ifstream fin;
    fstream fout;
    fin.open(inputFileName);
    auto op = (RANK == 0) ? ios::out | ios::trunc : ios::out;
    fout.open(outputFileName, op);

    // structure to store data
    AbaqusData *data;
//    vector<Node *> nodeList;
//    vector<Element *> elementList;
//    vector<Element *> boundaryList;

    double start, end; // used to record the time cost


    // read part
    start = MPI_Wtime();

    if (!sourceType.compare("abaqus")) {
        data = new AbaqusData();
        int end;
        if (RANK == 0)
            end = scanAbaqus(fin);
        MPI_Bcast(&end, 1, MPI_INT, 0, MPI_COMM_WORLD);
        STARTLINE = end / SIZE * RANK;
        ENDLINE = STARTLINE + end / SIZE;
        if (RANK == SIZE - 1) ENDLINE = end + 1; // 保证最后一个进程读完剩余的所有行
        readAbaqus(fin, data, STARTLINE, ENDLINE);
        if (ISPRINT)
            cout << "rank:" << RANK << " " << "start: " << STARTLINE << " end: " << ENDLINE << endl;

    } else if (!sourceType.compare("mfem")) {
//        readMfem(fin, nodeList, elementList, boundaryList);
    } else {
        cerr << left << setw(20) << "no such source type " << sourceType << endl;
    }

    end = MPI_Wtime();
    ctable("read cost:");
    cout << "rank " << RANK << " : " << end - start << "s" << endl;

    // write part
    start = MPI_Wtime();

    if (!destType.compare("debug")) {
        int ok;
        MPI_Status status;
        int left = MPI_PROC_NULL;
        int right = MPI_PROC_NULL;
        if (RANK != 0)
            left = RANK - 1;
        if (RANK != SIZE - 1)
            right = RANK + 1;
        MPI_Recv(&ok, 1, MPI_INT, left, 0, MPI_COMM_WORLD, &status);

        writeDebug(fout, data);

        MPI_Send(&ok, 1, MPI_INT, right, 0, MPI_COMM_WORLD);

    } else if (!destType.compare("mfem")) {
//        writeMfem(fout, nodeList, elementList, boundaryList);
    } else {
        cerr << left << setw(20) << "no such destination type " << destType << endl;
    }


    end = MPI_Wtime();
    ctable("write cost:");
    cout << "rank " << RANK << " : " << end - start << "s" << endl;

    fin.close();
    fout.close();

    MPI_Finalize();
    ctable("rank:");
    cout << RANK << " process over!" << endl;
    return 0;
}
