/*************************************************  
Author: Kevin.XU
Date:   2017-10-26
Desc:   输出格式化
**************************************************/ 

#ifndef PRINTER_H
#define PRINTER_H

#include <iostream> 
#include <vector>
#include <string>


namespace sogou
{

typedef std::vector<std::string>   TCell;
typedef std::vector<TCell>         TRow;
typedef std::vector<TRow>          TTable;


class Printer
{
  public:
    static void printTable(TTable& table, std::ostream& outstream, int width=20);

};


}

#endif


