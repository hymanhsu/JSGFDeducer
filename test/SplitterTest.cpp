#include <util/splitter.h>
#include <util/timer.h>

#include <iostream>
#include <vector>
#include <string>

int main(int argc, char **argv)
{
    if(argc<2){
        return 0;
    }
    std::string query = argv[1];

    sogou::Timer timer;

    sogou::SplitResult splitResult;
    sogou::Splitter::SplitByChar(query, splitResult);
    std::vector<sogou::SplitWord>::iterator it;
    for (it = splitResult.result.begin(); it != splitResult.result.end(); ++it)
    {
        timer.start();
        sogou::SplitWord &item = *it;
        std::cout << "[" << item.word << "], offset=" << item.offset << ", len=" << item.len
                  << ", start=" << item.start << ", end=" << item.end << std::endl;
        timer.stop();
        //std::cout << "cost time = " << timer.costTime() << std::endl;
    }
    //std::cout << "total cost time = " << timer.totalCostTime() << std::endl;

    return 0;
}
