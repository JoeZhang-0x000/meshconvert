## 安装
下载源码，使用cmake编译

## 使用
基本命令格式

Meshconvert -i filename1 -o filename2 [options] # filename1 -> filename2
[options]

-i, –input, input file name # 输入文件的名称

-o, –output, output file name  # 输出文件的名称

-s, –source, source type,[“abaqus”,”mfem”,”oofem”,...] # 默认abaqus

-d, –destination, destination type,[同上] # 默认 mfem

-p, –print, print details,[True,False] # 默认Fasle

-w, –width, write span width # 输出文件的间距

…

## 用例
例如在../data目录下有如下两个文件

| 文件名      | 类型     |
|----------|--------|
| test.inp | abaqus |
| test2.in | mfem   |

我们使用meshconvert将test.inp转换为mfem可使用如下命令

``./meshconvert -i test.inp -o test.out -s abaqus -d mfem``

将test2.in转换为mfem类型(测试程序的正确性)可以使用如下命令

``./meshconvert -i test2.in -s mfem -o test.out -d mfem``
