#include<stdio.h>
#include<mpi.h>
#include<fstream>
#include<cstring>
#include<iostream>
#include<tuple>
#include<vector>
#include"utils.h"
#include<map>
#include "cmdline.h"
using namespace std;
// # geometry      id   vertices
// # POINT       = 0    1
// # SEGMENT     = 1    2
// # TRIANGLE    = 2    3
// # SQUARE      = 3    4
// # TETRAHEDRON = 4    4
// # CUBE        = 5    8
// # PRISM       = 6    9
// # PYRAMID     = 7    5
map<string,int> GEOM = {{"POINT",0},{"SEGMENT",1},{"TRIANGLE",2},{"SQUARE",3},{"TETRAHEDRON",4},{"CUBE",5},{"PRISM",6},{"PYRAMID",7}}; // dictionary, k:the number of nodes, v:geom index in mfem
map<int,string> IGEOM = {{0,"POINT"},{1,"SEGMENT"},{2,"TRIANGLE"},{3,"SQUARE"},{4,"TETRAHEDRON"},{5,"CUBE"},{6,"PRISM"},{7,"PYRAMID"}}; // inverse GEOM
map<int,string> GEOM2D = {{1,"POINT"},{2,"SEGMENT"},{3,"TRIANGLE"},{4,"SQUARE"}}; // k:vertices number,v: geometry
map<string,int> IGEOM2D = {{"POINT",1},{"SEGMENT",2},{"TRIANGLE",3},{"SQUARE",4}}; // inverse GEOM2D
map<int,string> GEOM3D = {{4,"TETRAHEDRON"},{8,"CUBE"},{6,"PRISM"},{5,"PYRAMID"}};
map<string,int> IGEOM3D = {{"TETRAHEDRON",4},{"CUBE",8},{"PRISM",6},{"PYRAMID",5}}; // inverse GEOM3D

int DIM = 3; //default dimension
int NDIM = 3; //default nodes dimension
int NID = 1; // global node id
int EID = 1; // global element id

int WID = 10; //default span, used in setw(WID)

bool ISPRINT = true; // if true, print something,


/*find if str1 contains str2,
 * if yes return the position where first find str2 in str1
 * if no, return str1.npos,
 */
int find(char *s1,char *s2){
  string str1 = s1;
  string str2 = s2;
  int pos = str1.find(str2);
  pos = pos==str1.npos?-1:pos;
  return pos;
}

/*split a string with given delimiter
 */
tuple<char**,int> split(char* s){
  char *p;
  char **res;
  res = new char*[10];
  char delim[] = " ,";
  p = new char[1024];
  p = strtok(s,delim);
  int len = 0;
  while(p!=NULL){
    res[len++] = p;
    p = strtok(NULL,delim);
  }
  return tuple<char**,int>{res,len};
}

class Node{
  public:
    double id,x,y,z;
  public:
    Node(int=0,double=0.f,double=0.f,double=0.f);
    string to_write_oofem();
    string to_write_mfem();
};

Node::Node(int id,double x,double y,double z){
  this->id = id;
  this->x = x;
  this->y = y;
  this->z = z;
}


string Node::to_write_oofem(){
  stringstream tmp;
  if(DIM==3)
    tmp<<"node"<<" "<<int(id)<<" "<<"coords"<<" "<<"3"<<" "<<to_string(x)<<" "<<to_string(y)<<" "<<to_string(z)<<"\n";
  else
    tmp<<"node"<<" "<<int(id)<<" "<<"coords"<<" "<<"2"<<" "<<to_string(x)<<" "<<to_string(y)<<"\n";
  string res = tmp.str();
  return res;
}

string Node::to_write_mfem(){
  stringstream tmp;
  if(NDIM==3)
    tmp<<setw(WID)<<to_string(x)<<" "<<setw(WID)<<to_string(y)<<" "<<setw(WID)<<to_string(z)<<"\n";
  else
    tmp<<setw(WID)<<to_string(x)<<" "<<setw(WID)<<to_string(y)<<"\n";
  string res = tmp.str();
  return res;
}

class Element{
  public:
    int id;
    vector<int> v_node; // storage points index
  public:
    Element(int);
    void add(int);
    string to_write_oofem();
    string to_write_mfem();
};

Element::Element(int id){
  this->id = id;
}

void Element::add(int n){
  this->v_node.push_back(n);
}

string Element::to_write_oofem(){
  stringstream tmp;
  tmp<<"lspace"<<" "<<int(id)<<" "<<"nodes"<<" "<<v_node.size()<<" ";
  for(auto i:v_node){
    tmp <<to_string(i)<<" ";
  }
  tmp<<endl;
  string res =tmp.str();
  return res;
}

string Element::to_write_mfem(){
  stringstream tmp;
  int gem = -1;
  if(DIM == 3)
    gem = GEOM[GEOM3D[int(v_node.size())]];
  else
    gem = GEOM[GEOM2D[int(v_node.size())]];
  // tmp<<int(id)<<" ";
  tmp<<setw(WID)<<1<<" "<<setw(WID)<<gem<<" ";
  for(auto i:v_node){
    tmp <<setw(WID)<<to_string(i)<<" ";
  }
  tmp<<endl;
  string res =tmp.str();
  return res;
}

//void readMfem(ifstream &fin,vector<Node*> &nodeList,vector<Element*> &elementList){
//  char data[1024]; // buffer, is used when read something
//  fin.getline(data,1024);
//  auto [s,len] = split(data);
//  string version = s[len-1];
//  if(len!=3 || version!="v1.0"){
////    stderr<<"not mfem mesh!\n";
//    return;
//  }
//  // skip #
//  while(!fin.eof()){
//    fin.getline(data,1024);
//    if(strlen(data)!=0 && data[0]!='#') //not empty
//      break;
//  } // dimension
//  int dim;
//  fin>>dim;
//  if(dim!=2 && dim!=3){
////    stderr<<"dimension error!\n";
//    return;
//  }
//  DIM = dim;
//  while(!fin.eof()){
//    fin.getline(data,1024);
//    if(data[0]=='e')
//        break;
//  }// elements
//  int nE; // the number of elements to be read below
//  fin>>nE;
//  for(int i=0;i<nE;i++){
//    fin.getline(data,1024);
//    if(strlen(data)==0)
//        continue;
//    auto [s,len] = split(data);
//    int att = stoi(s[0]);
//    int geomType = stoi(s[1]); // geometry type
//    string geom = IGEOM[geomType];
//    int nd = DIM==3?IGEOM3D[geom]:IGEOM2D[geom]; // nodes number
//    Element *element = new Element(EID++);
//    for(int i=2;i<len;i++){
//        element->add(stoi(s[i]));
//    }
//    elementList.push_back(element);
//  }
//
//    while(!fin.eof()){
//        fin.getline(data,1024);
//        if(data[0]=='b')
//            break;
//    } // boundary seems to element
//    cout<<data<<endl;
//    fin>>nE;
//    for(int i=0;i<nE;i++){
//        fin.getline(data,1024);
//        if(strlen(data)==0)
//            continue;
//        auto [s,len] = split(data);
//        int att = stoi(s[0]);
//        int geomType = stoi(s[1]); // geometry type
//        string geom = IGEOM[geomType];
//        int nd = DIM==3?IGEOM3D[geom]:IGEOM2D[geom]; // nodes number
//        Element *element = new Element(EID++);
//        for(int i=2;i<len;i++){
//            element->add(stoi(s[i]));
//        }
//        elementList.push_back(element);
//    }
//
//
//    while(!fin.eof()){
//        fin.getline(data,1024);
//        if(data[0]=='v')
//            break;
//    } // vertices
//  int nN; // the number of nodes to be read below
//  fin>>nN;
//  fin>>dim; // vertices dim
//  for(int i=0;i<nN;i++){
//      int x,y,z;
//      fin>>x>>y;
//      if(dim==3)
//          fin>>z;
//      Node *node = new Node(x,y,z);
//      nodeList.push_back(node);
//  }
//
//}

void readMfem(ifstream &fin,vector<Node*> &nodeList,vector<Element*> &elementList,vector<Element*> &boundaryList){
    nodeList.clear(),elementList.clear();
    char data[1024]; // buffer, is used when read something

    int flag = -1; // default -1 represents reading plain text, otherwise corresponds to flags
    const string flags[] = {
            "dimension",
            "elements",
            "boundary",
            "vertices",
    }; // the key words list,

    int loc = 0; // the number of lines have been read

    while(!fin.eof()){
        // each time read only one line, if it contains keywords, do something special
        loc ++;
        fin.getline(data,1024);
        if(strlen(data)==0) //empty
            continue;
        auto [s,len] = split(data);

        if(s[0][0]=='d') { // dimension
            flag = 0;
            continue;
        }
        else if(s[0][0]=='e') {// elements
            flag = 1;
            fin.getline(data,1024); // read elements number
            continue;
        }
        else if(s[0][0]=='b') { // boundaries
            flag = 2;
            fin.getline(data,1024); // read boundaries number
            continue;
        }
        else if(s[0][0]=='v') { // vertices
            flag = 3;
            fin.getline(data,1024); // read vertices number
            fin.getline(data,1024); // read vertices dim
            NDIM = stoi(data);
            continue;
        }
        switch(flag){
            case -1:{
                break;
            }
            case 0:{
                // read dimension
                // just read next line
                DIM = stoi(data);
                break;
            }
            case 1:{ // read Element
                // data type:
                // attribute,geometry,point 1,point 2,...
                Element *element = new Element(EID++); //EID is the global indices of elements
                for(int i=2;i<len;i++){ // just read points
                    element->add(stoi(s[i]));
                }
                elementList.push_back(element);
                break;
            }
            case 2:{ // read boundary
                // data type:
                // attribute,geometry,point 1,point 2,...
                Element *element = new Element(EID++); //EID is the global indices of elements
                for(int i=2;i<len;i++){ // just read points
                    element->add(stoi(s[i]));
                }
                boundaryList.push_back(element);
                break;
            }
            case 3:{ // read vertices
                // data type:
                // x,y,z
                Node *node = new Node(NID++,stod(s[0]),stod(s[1]),NDIM==2?0.f:stod(s[2]));
                nodeList.push_back(node);
                break;
            }
            default:
                break;
        }
    }
}


void readAbaqus(ifstream &fin,vector<Node*> &nodeList,vector<Element*> &elementList){
  char data[1024]; // buffer, is used when read something

  int flag = -1; // default -1 represents reading plain text, otherwise corresponds to flags
  const string flags[] = {
    "Node",
    "Element",
  }; // the key words list, e.g. flag = 1 means read "Node" type content

  int loc = 0; // the number of lines have been read

  while(!fin.eof()){
    // each time read only one line, if it contains keywords, do something special
    loc ++;
    fin.getline(data,1024);
    if(strlen(data)==0) //empty
      continue;
    auto [s,len] = split(data);

    if(s[0][0]!='*')
      switch(flag){
        case -1:{
          break;
        }
        case 0:{ // read Node
          // data type:
          // id,x,y,z
          DIM = len-1;
          Node *node = new Node(stoi(s[0]),stod(s[1]),stod(s[2]),DIM==2?0.f:stod(s[3]));
          nodeList.push_back(node);
          break;
        }
        case 1:{ // read Element
        // data type:
        // id,point 1,point 2,...
          Element *element = new Element(stoi(s[0]));
          for(int i=1;i<len;i++){
            element->add(stoi(s[i]));
          }
          elementList.push_back(element);
          break;
        }
        default:
          break;
      }
    else{
      flag = -1; // reset flag to -1
      for(int i=0;i<sizeof(flags)/sizeof(flags[0]);i++){
        if(find(s[0],(char*)(flags[i].c_str()))!=-1){
          if(ISPRINT)
              printf("at %d,read %s\n",loc,flags[i].c_str());
          flag = i; // change flag
        }
      }
    }
  }
}

void writeOofem(ofstream &fout,vector<Node*> &nodeList,vector<Element*> &elementList){
  for(auto i:nodeList){
    fout<<i->to_write_oofem();
  }
  fout<<endl;
  for(auto i:elementList){
    fout<<i->to_write_oofem();
  }
}

void writeMfem(ofstream &fout,vector<Node*> &nodeList,vector<Element*> &elementList,vector<Element*> &boundaryList){
  fout<<"MFEM mesh v1.x\n";
  fout<<endl;
  fout<<"dimension\n";
  fout<<DIM<<"\n";
  fout<<endl;
  fout<<"elements\n";
  fout<<elementList.size()<<endl;
  for(auto i:elementList){
    fout<<i->to_write_mfem();
  }
  fout<<endl;

  fout<<"boundary\n";
  DIM = 2;
  fout<<boundaryList.size()<<endl;
    for(auto i:boundaryList){
        fout<<i->to_write_mfem();
    }
  fout<<endl;

  fout<<endl;
  fout<<"vertices\n";
  fout<<nodeList.size()<<endl;
  fout<<NDIM<<endl;
  for(auto i:nodeList){
    fout<<i->to_write_mfem();
  }
}


int main(int argc,char** argv){
  /**
   * read part
  */
  cmdline::parser a; // initialize a cmd decipher
  a.add<string>("input",'i',"input file name",true);
  a.add<string>("output",'o',"output file name",true);
  a.add<string>("source",'s',"source type",false,"abaqus",cmdline::oneof<string>("abaqus","mfem"));
  a.add<string>("dest",'d',"destination type", false,"mfem",cmdline::oneof<string>("abauqs","mfem"));
  a.add<bool>("print",'p',"print details",false,true);
  a.add<int>("width",'w',"write span wdith",false,10,cmdline::range(1,9999));
  a.parse_check(argc,argv);
  MPI_Init(0,0);

  string inputFileName = a.get<string>("input").c_str();
  string outputFileName = a.get<string>("output").c_str();
  string sourceType = a.get<string>("source");
  string destType = a.get<string>("dest");
  WID = a.get<int>("width");

  cout<<"**********************************"<<endl;
  ctable("input");cout<<inputFileName<<endl;
  ctable("output:");cout<<inputFileName<<endl;
  ctable("source type:");cout<<sourceType<<endl;
  ctable("destination type:");cout<<destType<<endl;

  ifstream fin;
  ofstream fout;
  fin.open(inputFileName);
  fout.open(outputFileName);

  ISPRINT = a.get<bool>("print"); // if not specify, default isprint = true
  ctable("print details:");cout<<ISPRINT<<endl;

  // structure to store data
  vector<Node*> nodeList;
  vector<Element*> elementList;
  vector<Element*> boundaryList;

  double start,end; // used to record the time cost

    // read part
  start = MPI_Wtime();

  if(!sourceType.compare("abaqus")){
      readAbaqus(fin,nodeList,elementList);
  }
  else if(!sourceType.compare("mfem")){
      readMfem(fin,nodeList,elementList,boundaryList);
  }
  else{
      cerr<<left<<setw(20)<<"no such source type "<<sourceType<<endl;
  }

  end = MPI_Wtime();
  ctable("read cost:");cout<<end-start<<"s"<<endl;

  // write part
  start = MPI_Wtime();

    if(!destType.compare("oofem")){
        writeOofem(fout,nodeList,elementList);
    }
    else if(!destType.compare("mfem")){
        writeMfem(fout,nodeList,elementList,boundaryList);
    }
    else{
        cerr<<left<<setw(20)<<"no such destination type "<<destType<<endl;
    }


    end = MPI_Wtime();
    ctable("read cost:");cout<<end-start<<"s"<<endl;

    fin.close();
    fout.close();

    MPI_Finalize();
    cout<<"over!"<<endl;
    return 0;
}
