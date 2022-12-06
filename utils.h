//
// Created by zhangx on 2022/12/6.
//

#ifndef ABAQUS_INP_UTILS_H
#define ABAQUS_INP_UTILS_H
#include "string"
#include <iostream>
#include "iomanip"
#define CWD 20
void ctable(std::string s){
    std::cout<<std::left<<std::setw(CWD)<<s;
}

#endif //ABAQUS_INP_UTILS_H
