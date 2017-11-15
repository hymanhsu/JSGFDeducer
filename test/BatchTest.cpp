#include <util/splitter.h>
#include <util/timer.h>
#include <dict/matcher.h>
#include <wfst/rule.h>
#include <wfst/parser.h>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <utility>

typedef std::vector<std::pair<std::string,int> > StaticsResult;

void doOne(sogou::DictMatcherMgr *dictMatcherMgr,
    sogou::RegexMatcher *regexMatcher,
    sogou::RuleMgr *ruleMgr,
    const std::string &query, StaticsResult& staticsResult)
{
    //初始化计时器
    sogou::Timer timer;

    //分词
    timer.start();
    sogou::SplitResult splitResult;
    sogou::Splitter::SplitByChar(query, splitResult);
    timer.stop();
    staticsResult.push_back(std::make_pair("split",timer.costTime()));

    //词典匹配
    std::vector<sogou::MatchUnit> dictResult;
    //dict tag
    timer.start();
    dictMatcherMgr->match(splitResult, dictResult);
    timer.stop();
    staticsResult.push_back(std::make_pair("dict",timer.costTime()));

    //regex tag
    timer.start();
    regexMatcher->match(splitResult, dictResult);
    timer.stop();
    staticsResult.push_back(std::make_pair("regex",timer.costTime()));
    
    //regex tag
    timer.start();
    ruleMgr->matchInnerDict(splitResult, dictResult);
    timer.stop();
    staticsResult.push_back(std::make_pair("innnerdict",timer.costTime()));

    //过滤重复
    timer.start();
    sogou::deduplicateMatchUnits(dictResult);
    timer.stop();
    staticsResult.push_back(std::make_pair("deduplicate",timer.costTime()));

    //推导
    bool suucess = false;
    timer.start();
    sogou::WfstParser *wfstParser = new sogou::WfstParser(ruleMgr, &splitResult, &dictResult);
    if (!wfstParser->deduce())
    {
        timer.stop();
        staticsResult.push_back(std::make_pair("deduce",timer.costTime()));
    }
    else
    {
        timer.stop();
        staticsResult.push_back(std::make_pair("deduce",timer.costTime()));
        suucess = true;
    }

    if (suucess)
    {
        //分析推导结果
        timer.start();
        std::vector<sogou::DeduceResult> deduceResults;
        wfstParser->analyzeDeduceResult(deduceResults);
        timer.stop();
        staticsResult.push_back(std::make_pair("analyze",timer.costTime()));
    }

    staticsResult.push_back(std::make_pair("total",timer.totalCostTime()));
}

void doBatch(sogou::DictMatcherMgr *dictMatcherMgr,
             sogou::RegexMatcher *regexMatcher,
             sogou::RuleMgr *ruleMgr,
             const std::string &intputFilePath,
             const std::string &outputFilePath)
{
    std::ifstream in(intputFilePath);
    std::ofstream out(outputFilePath);
    std::string query;


    // S表示平均数，可以推导出 S{n} = ( a{n} + S{n-1} * (n-1) ) / n
    int averageCostTime = 0;
    int maxCostTime = 0;
    int n = 0;

    if(in){
        while( getline(in,query) ){
            if(query == ""){
                continue;
            }

            StaticsResult staticsResult;
            doOne(dictMatcherMgr,regexMatcher,ruleMgr,query,staticsResult);

            int curCost = staticsResult[staticsResult.size()-1].second;
            ++n;

            if(averageCostTime == 0){
                averageCostTime = curCost;
            }else{
                averageCostTime = (averageCostTime * (n-1) + curCost) / n;
            }
            
            if(maxCostTime == 0){
                maxCostTime = curCost;
            }else{
                if(curCost > maxCostTime){
                    maxCostTime = curCost;
                }
            }

            out << "query: [" << query << "], ";
            StaticsResult::iterator it = staticsResult.begin();
            for(;it!=staticsResult.end();++it){
                std::pair<std::string,int> & item = *it;
                out << item.first << ":" << item.second << ",";
            }
            out << std::endl;
        }
    }

    out << " total : average cost =" << averageCostTime << ", max cost = " << maxCostTime << std::endl;

}

int main(int argc, char **argv)
{
    std::string dictDirPath;
    std::string grammarFilePath;
    std::string intputFilePath;
    std::string outputFilePath;
    if (argc > 4)
    {
        dictDirPath = argv[1];
        grammarFilePath = argv[2];
        intputFilePath = argv[3];
        outputFilePath = argv[4];

        //初始化规则管理器
        sogou::RuleMgr *ruleMgr = new sogou::RuleMgr(grammarFilePath);
        if (!ruleMgr->init())
        {
            return 0;
        }

        //初始化词典
        sogou::DictMatcherMgr *dictMatcherMgr = new sogou::DictMatcherMgr();
        dictMatcherMgr->init(dictDirPath);
        sogou::RegexMatcher *regexMatcher = new sogou::RegexMatcher();

        //do it
        doBatch(dictMatcherMgr, regexMatcher, ruleMgr, intputFilePath, outputFilePath);
    }
    else
    {
        std::cout << "Missing command parameters !" << std::endl;
    }

    return 0;
}
