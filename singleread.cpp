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
    a.add<string>("dest", 'd', "destination type", false, "mfem", cmdline::oneof<string>("abauqs", "mfem"));
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
    ofstream fout;
    fin.open(inputFileName);
    fout.open(outputFileName);

    // structure to store data
    vector<Node *> nodeList;
    vector<Element *> elementList;
    vector<Element *> boundaryList;

    double start, end; // used to record the time cost

    memset(rows, 1, sizeof(rows)); // 填充为最大

    // read part
    start = MPI_Wtime();

    if (!sourceType.compare("abaqus")) {
        if (SIZE > 1 && RANK != 0 && RANK < sizeof(rows) / sizeof(int)) { // 子进程先等待0号进程发送读取指标，存在se中
            MPI_Recv(rows, sizeof(rows)/sizeof(int), MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            cout<<"rank "<<RANK<<"\t"<<"startLine: "<<rows[RANK-1]<<"\t"<<"endLine: "<<rows[RANK]<<endl;
            readAbaqus(fin, nodeList, elementList, rows[RANK-1], rows[RANK]);
        } else { // 0号进程读取整个文件
            readAbaqus(fin, nodeList, elementList);
            for(int i=1;i<SIZE;i++)
                MPI_Send(rows,sizeof(rows)/sizeof(int),MPI_INT,i,0,MPI_COMM_WORLD);
//            MPI_Bcast(rows,sizeof(rows)/sizeof(int),MPI_INT,0,MPI_COMM_WORLD);
        }
    } else if (!sourceType.compare("mfem")) {
        readMfem(fin, nodeList, elementList, boundaryList);
    } else {
        cerr << left << setw(20) << "no such source type " << sourceType << endl;
    }

    end = MPI_Wtime();
    ctable("read cost:");
    cout<<"rank "<<RANK<<" : " << end - start << "s" << endl;

    // write part
    start = MPI_Wtime();

    if (!destType.compare("oofem")) {
        writeOofem(fout, nodeList, elementList);
    } else if (!destType.compare("mfem")) {
        writeMfem(fout, nodeList, elementList, boundaryList);
    } else {
        cerr << left << setw(20) << "no such destination type " << destType << endl;
    }


    end = MPI_Wtime();
    ctable("write cost:");
    cout<<"rank "<<RANK<<" : " << end - start << "s" << endl;

    MPI_Barrier(MPI_COMM_WORLD);

    fin.close();
    fout.close();

    MPI_Finalize();
    ctable("rank:");cout<<RANK<<" process over!"<< endl;
    return 0;
}
