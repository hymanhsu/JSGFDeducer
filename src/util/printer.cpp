#include <util/printer.h>
#include <iomanip>


namespace sogou
{

void Printer::printTable(TTable &table, std::ostream &outstream, int width)
{
    if (table.size() == 0)
    {
        return;
    }
    size_t row = table.size();
    size_t col = table[0].size();

    //画表头
    //outstream << std::setiosflags(std::ios::left);
    outstream.setf(std::ios::right,std::ios::adjustfield);
    outstream << std::setfill('-') << std::setw(width * (col + 1) + col + 2) << "-" << std::endl;
    outstream << std::setfill(' ');
    outstream << "|" << std::setw(width) << " " << "|";
    for (size_t i = 0; i < col; ++i)
    {
        outstream << std::setw(width) << i << "|";
    }
    outstream << std::endl;
    outstream << std::setfill('-') << std::setw(width * (col + 1) + col + 2) << "-" << std::endl;

    //画内容
    for (size_t i = 0; i < row; ++i)
    {
        TRow & row = table[i];
        bool loopPrint = true;
        bool hasPrintNum = false;
        do{
            bool foundData = false;
            for (size_t j = 0; j < col; ++j){
                TCell & cell = table[i][j];
                if(!cell.empty()){
                    foundData = true;
                    break;
                }
            }
            if(!foundData){
                loopPrint = false;
                outstream << std::setfill('-') << std::setw(width * (col + 1) + col + 2) << "-" << std::endl;
            }else{
                outstream << std::setfill(' ');
                if(!hasPrintNum){
                    outstream << '|' << std::setw(width) << i << "|";
                    hasPrintNum = true;
                }else{
                    outstream << '|' << std::setw(width) << "" << "|";
                }
                for (size_t j = 0; j < col; ++j)
                {
                    TCell & cell = table[i][j];
                    if(cell.empty()){
                        outstream << std::setw(width) << "" << "|";
                    }else{
                        std::string & value = cell.front();
                        auto l = value.length();
                        if (l > width)
                        {
                            outstream << std::setw(width) << value.substr(l - width) << "|";
                        }
                        else
                        {
                            outstream << std::setw(width) << value << "|";
                        }
                        cell.erase(cell.begin());
                    }
                }
                outstream << std::endl;
            }
        }while(loopPrint);
    }
}




}
