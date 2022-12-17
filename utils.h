//
// Created by zhangx on 2022/12/6.
//

#ifndef M_UTILS_H
#define M_UTILS_H
#include "string"
#include <iostream>
#include "iomanip"
#include "tuple"
using namespace  std;
#define CWD 20
void ctable(std::string s){
    std::cout<<std::left<<std::setw(CWD)<<s;
}

/*find if str1 contains str2,
 * if yes return the position where first find str2 in str1
 * if no, return str1.npos,
 */
int find(char *s1, char *s2) {
    string str1 = s1;
    string str2 = s2;
    int pos = str1.find(str2);
    pos = pos == str1.npos ? -1 : pos;
    return pos;
}

/*split a string with given delimiter
 */
pair<char **, int> split(char *s) {
    char *p;
    char **res;
    res = new char *[10];
    char delim[] = " ,";
    p = new char[1024];
    p = strtok(s, delim);
    int len = 0;
    while (p != NULL) {
        res[len++] = p;
        p = strtok(NULL, delim);
    }
    return {res, len};
}

#endif //M_UTILS_H
