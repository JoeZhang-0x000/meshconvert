#include<stdio.h>
#include<mpi.h>
#include<iostream>
#include "structure.h"
#include "cmdline.h"
#include "mfem.h"
#include "abaqus.h"

using namespace std;




int main(int argc, char **argv) {

    // command decipher
    cmdline::parser a; // initialize a cmd decipher
    a.add<string>("input", 'i', "input file name", true);
    a.add<string>("output", 'o', "output file name", true);
    a.add<string>("source", 's', "source type", false, "abaqus", cmdline::oneof<string>("abaqus", "mfem"));
    a.add<string>("dest", 'd', "destination type", false, "mfem", cmdline::oneof<string>("abauqs", "mfem"));
    a.add<bool>("print", 'p', "print details", false, true);
    a.add<int>("width", 'w', "write span wdith", false, 10, cmdline::range(1, 9999));
    a.parse_check(argc, argv);
    MPI_Init(0, 0);

    string inputFileName = a.get<string>("input").c_str();
    string outputFileName = a.get<string>("output").c_str();
    string sourceType = a.get<string>("source");
    string destType = a.get<string>("dest");
    WID = a.get<int>("width");

    cout << "**********************************" << endl;
    ctable("input:");
    cout << inputFileName << endl;
    ctable("output:");
    cout << outputFileName << endl;
    ctable("source type:");
    cout << sourceType << endl;
    ctable("destination type:");
    cout << destType << endl;

    ifstream fin;
    ofstream fout;
    fin.open(inputFileName);
    fout.open(outputFileName);

    ISPRINT = a.get<bool>("print"); // if not specify, default isprint = true
    ctable("print details:");
    cout << ISPRINT << endl;

    // structure to store data
    vector<Node *> nodeList;
    vector<Element *> elementList;
    vector<Element *> boundaryList;

    double start, end; // used to record the time cost

    // read part
    start = MPI_Wtime();

    if (!sourceType.compare("abaqus")) {
        readAbaqus(fin, nodeList, elementList);
    } else if (!sourceType.compare("mfem")) {
        readMfem(fin, nodeList, elementList, boundaryList);
    } else {
        cerr << left << setw(20) << "no such source type " << sourceType << endl;
    }

    end = MPI_Wtime();
    ctable("read cost:");
    cout << end - start << "s" << endl;

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
    ctable("read cost:");
    cout << end - start << "s" << endl;

    fin.close();
    fout.close();

    MPI_Finalize();
    cout << "over!" << endl;
    return 0;
}
