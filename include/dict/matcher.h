/*************************************************  
Author: Kevin.XU
Date:   2017-10-26
Desc:   词典
**************************************************/ 

#ifndef MATCHER_H
#define MATCHER_H

#include <util/splitter.h>

#include <re2/stringpiece.h>
#include <re2/re2.h>

#include <string>
#include <vector>
#include <unordered_set>
#include <map>
#include <string>
#include <list>
#include <set>
#include <tuple>


namespace sogou
{
    struct MatchUnit {
        std::string text_;
        std::string norm_;
        std::string tag_;
        int offset_;
        int count_;
        std::string source_;
    };

    /*class TagUnit {
    public:
        TagUnit():offset_(0), count_(0), score_(0.0){}
        virtual ~TagUnit(){}
    public:
        std::string text_;  // 五道口
        int offset_;    // 0
        int count_; // 9
        std::string tag_; // GEO
        std::string norm_; // 五道口
        float score_; // norm and tag's score
        std::string source_; //regex, rank, rule, dict
    };*/

    class DictMatcher {
    public:
        DictMatcher(const std::string & dict_name, std::unordered_set<std::string> & patterns);
        ~DictMatcher();
        bool init();
        int match(const SplitResult &splitResult, std::vector<MatchUnit>& result);
    public:
        std::string           dict_name_;
        size_t                max_size_;
        std::unordered_set<std::string> patterns_;
    };

    class DictMatcherMgr {
    public:
        DictMatcherMgr();
        ~DictMatcherMgr();
        bool init(const std::string & dictDir);
        bool isRegularFile(const char* filename);
        int readFileList(const std::string& basePath, const std::string& terminal, std::vector<std::string> &files);
        void match(const SplitResult &splitResult, std::vector<MatchUnit>& result);
    private:
        std::map<std::string, DictMatcher*> matchers_; // key 为 TAG
    };

    class RegexMatcher {
    public:
        RegexMatcher();
        virtual ~RegexMatcher();
        bool re2_full_match(const std::string & pattern, const std::string & patternTag, const std::string & patternName, const std::string & str, std::vector<std::string> & results);
        void matchSinglePattern(std::list<MatchUnit> &sents, const std::string & pattern, const std::string & patternTag, const std::string & patternName);
        void saveCompiledPattern(const std::string& pattern, RE2* re2);
        RE2* searchCompiledPattern(const std::string& pattern);
        void match(const SplitResult &splitResult, std::vector<MatchUnit>& result);
    private:
        //1,tag, 2,regex exp name, 3,regex exp
        std::vector<std::tuple<std::string, std::string, std::string> > regs_;
        // std::map<std::string,  RE2*> regs_;
        std::map<std::string, RE2* > compiledPatterns_;
    };

    /**
     * 过滤重复的匹配结果
     **/
    void deduplicateMatchUnits(std::vector<MatchUnit>& result);

}

#endif