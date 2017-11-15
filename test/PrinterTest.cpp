#include <util/splitter.h>
#include <util/printer.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

int main(int argc, char **argv)
{

    sogou::TTable table;

    sogou::TRow row1;
    row1.push_back(sogou::TCell({"a","b","c"}));
    row1.push_back(sogou::TCell({"c","d"}));
    table.push_back(row1);

    sogou::TRow row2;
    row2.push_back(sogou::TCell({"m","w"}));
    row2.push_back(sogou::TCell());
    table.push_back(row2);

    //std::stringstream strout;
    sogou::Printer::printTable(table,std::cout,20);

    //std::cout << strout.str() << std::endl;

    return 0;
}
