#include <util/splitter.h>
#include <util/timer.h>
#include <dict/matcher.h>

#include <iostream>
#include <vector>
#include <string>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>


sogou::DictMatcherMgr * testInitDictMatcherMgr(std::string & dictBaseDir){
    sogou::DictMatcherMgr * dictMatcherMgr = new sogou::DictMatcherMgr();
    dictMatcherMgr->init(dictBaseDir);
    return dictMatcherMgr;
}


int main(int argc, char **argv)
{
    std::string dictBaseDir = "../test/dict/";
    std::string query;
    if(argc>=2){
        dictBaseDir = argv[1];
        query = argv[1];
    }

    std::vector<sogou::MatchUnit> result;

    sogou::SplitResult splitResult;
    sogou::Splitter::SplitByChar(query, splitResult);

    sogou::DictMatcherMgr * dictMatcherMgr = testInitDictMatcherMgr(dictBaseDir);

    sogou::RegexMatcher * regexMatcher = new sogou::RegexMatcher();

    //dict tag
    dictMatcherMgr->match(splitResult, result);

    //regex tag
    regexMatcher->match(splitResult, result);
    
    //过滤重复
    sogou::deduplicateMatchUnits(result);

    std::vector<sogou::MatchUnit>::iterator it;
    for (it = result.begin(); it != result.end(); ++it)
    {
        sogou::MatchUnit &item = *it;
        std::cout << "[" << item.text_ << "], offset=" << item.offset_ << ", len=" << item.count_
                  << ", tag=" << item.tag_ << ", source=" << item.source_ << std::endl;
    }

    return 0;
}
