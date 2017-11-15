#include <util/splitter.h>
#include <util/timer.h>
#include <dict/matcher.h>
#include <wfst/rule.h>
#include <wfst/parser.h>

#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <unordered_set>

int main(int argc, char **argv)
{
    bool debug = true;
    char* grammar_filename_c = NULL;
    char* query = NULL;
    if(argc<3){
        return 0;
    }
    if(argc == 3){
        grammar_filename_c = argv[1];
        query = argv[2];
    }else if(argc == 4){
        grammar_filename_c = argv[1];
        query = argv[2];
        if(0 == strcasecmp(argv[3],"true")){
            debug = true;
        }else if(0 == strcasecmp(argv[3],"false")){
            debug = false;
        }
    }

    sogou::setImportRuleSearchPath("../test/gram/");
    
    //初始化规则管理器
    sogou::RuleMgr * ruleMgr = new sogou::RuleMgr(grammar_filename_c,debug);
    if(!ruleMgr->init()){
        return 0;
    }
    ruleMgr->displayJsgfRules();
    ruleMgr->displayRules();

    //初始化词典
    std::vector<sogou::MatchUnit> dictResult;
    std::string dictBaseDir = "../test/dict/";
    sogou::DictMatcherMgr * dictMatcherMgr = new sogou::DictMatcherMgr();
    dictMatcherMgr->init(dictBaseDir);
    sogou::RegexMatcher * regexMatcher = new sogou::RegexMatcher();

    //初始化计时器
    sogou::Timer timer;

    //分词
    timer.start();
    sogou::SplitResult splitResult;
    sogou::Splitter::SplitByChar(query, splitResult);
    timer.stop();
    std::cout << "split query cost time = " << timer.costTime() << std::endl;

    //词典匹配
    //dict tag
    timer.start();
    dictMatcherMgr->match(splitResult, dictResult);
    timer.stop();
    std::cout << "dict match cost time = " << timer.costTime() << std::endl;
    //regex tag
    timer.start();
    regexMatcher->match(splitResult, dictResult);
    timer.stop();
    std::cout << "regex dict match cost time = " << timer.costTime() << std::endl;
    //内部词典处理
    timer.start();
    ruleMgr->matchInnerDict(splitResult, dictResult);
    timer.stop();
    std::cout << "inner dict match cost time = " << timer.costTime() << std::endl;
    //过滤重复
    timer.start();
    sogou::deduplicateMatchUnits(dictResult);
    timer.stop();
    std::cout << "deduplicate dict result cost time = " << timer.costTime() << std::endl;

    //推导
    bool suucess = false;
    timer.start();
    sogou::WfstParser * wfstParser = new sogou::WfstParser(ruleMgr,&splitResult,&dictResult, debug);
    if(!wfstParser->deduce()){
        timer.stop();
        std::cout << "Deduce failed " << std::endl;
        std::cout << "deduce cost time = " << timer.costTime() << std::endl;
    }else{
        timer.stop();
        std::cout << "Deduce successfully " << std::endl;
        std::cout << "deduce cost time = " << timer.costTime() << std::endl;

        suucess = true;
    }

    if(suucess){
        //分析推导结果
        timer.start();
        std::vector<sogou::DeduceResult> deduceResults;
        wfstParser->analyzeDeduceResult(deduceResults,!debug);
        timer.stop();
        std::cout << "analyze deduce result cost time = " << timer.costTime() << std::endl;

        std::vector<sogou::DeduceResult>::iterator it = deduceResults.begin();
        for(;it != deduceResults.end(); ++it){
            std::cout << " deduceResult >> " << std::endl;
            sogou::DeduceResult & deduceResult = *it;
            std::unordered_set<std::pair<std::string,std::string>,sogou::pair_hash> & slots = deduceResult.slots;
            std::unordered_set<std::pair<std::string,std::string>,sogou::pair_hash>::iterator slotIter = slots.begin();
            for(;slotIter!=slots.end();++slotIter){
                const std::pair<std::string,std::string> & slot = *slotIter;
                std::cout << "           [" << slot.first << "] = [" << slot.second << "]" << std::endl;
            }
        }
    }

    std::cout << "total cost time = " << timer.totalCostTime() << std::endl;

    delete wfstParser;
    delete ruleMgr;
    delete dictMatcherMgr;
    delete regexMatcher;

    return 0;
}

