/*************************************************  
Author: Kevin.XU
Date:   2017-10-26
Desc:   规则
**************************************************/ 

#ifndef RULE_H
#define RULE_H


#include <dict/matcher.h>
#include <jsgf/jsgf_internal.h>
#include <jsgf/jsgf.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>


namespace sogou
{

typedef size_t SymbolType;  //符号类的类型
typedef std::unordered_set<std::string> StringSet;
typedef std::unordered_set<SymbolType>  SymbolTypeSet;

const SymbolType NillSymbol = 0; //空值

struct pair_hash
{
    template<class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& p) const
    {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

/**
 * 设置import的搜索路径，多个路径用;分割
**/
void setImportRuleSearchPath(const std::string& searchPath);


class ExpUnit{
public:
    ExpUnit():name(""),tag(""),isRule(true){}
    ~ExpUnit(){}
    std::string name;
    std::string tag;
    bool isRule;
};


class RuleMgr{
public:
    RuleMgr(const std::string& grammarFilePath,bool debug = false);
    ~RuleMgr();
    /**
     * 将jsgf语法对象转换为CYK规则
     **/
    bool init();
    /**
     * 打印jsgf规则
     **/
    void displayJsgfRules();
    /**
     * 打印规则
     **/
    void displayRules();
    /**
     * 判断规则是否可选
     **/
    bool isOptional(const std::string& ruleName);
    /**
     * 判断规则是否属于递归性质的
     **/
    bool isRecursion(const std::string& ruleName);
    /**
     * 检查有效性
     **/
    bool checkValid();
    /**
     * 将规则数据转换为运行状态数据
     **/
    void transToRunningState();
    /**
     * 返回规则全名
     **/
    std::string fullRuleName(const std::string& name);
    /**
     * 根据单词返回其类型
     * For 运行时
     **/
    const SymbolTypeSet& queryTerminalTypeByValue(const std::string& value);
    /**
     * 如果存在A=BC
     * 根据BC，查询出A
     * For 运行时
     **/
    const SymbolTypeSet& queryTargetTypeByPair(SymbolType type1, SymbolType type2);
    /**
     * 如果存在A=B
     * 根据B，查询出A
     * For 运行时
     **/
    const SymbolTypeSet& queryTargetLink(SymbolType type1);
    /**
     * 根据类型返回其关联的tag
     **/
    const std::string* queryTag(SymbolType type1);
    /**
     * 把规则名转换为SymbolType
     **/
    SymbolType transStringToInt(const std::string& str, bool saveReverse = false);
    /**
     * 根据SymbolType反查规则名
     **/
    const std::string& queryNameBySymbolType(SymbolType type);
    /**
     * 类型是否为空
     **/
    bool isNillSymbol(SymbolType type);
    /**
     * 类型集合是否为空
     **/
    bool isNillSymbolSet(const SymbolTypeSet& symbolTypeSet);
    /**
     * 规则是否包含<WILDCARD>
     **/
    bool isWildcard();
    /**
     * 返回语法名，其实也是Domain值
     **/
    const std::string getGrammarName();
    /**
     * 判断某一个词典是否被grammar所使用
     **/
    bool isDictTagUsed(const std::string& dictTagName);
    /**
     * 匹配内部词典
     **/
    void matchInnerDict(const SplitResult &splitResult, std::vector<MatchUnit> &result);
    /**
     * 查询某一个类型的权重
     **/
    int queryWeight(SymbolType sType);
    /**
     * 查询权重阈值(小于等于这个值，则表示没有意义)
     **/
    int thresholdWeight();
protected:
    void searchMatch(const std::string& content, const std::string& regexExpr, std::unordered_map<std::string, int>& matchResult);
    /**
     * 对*.gram文件进行预处理
     **/
    bool preHandleGramFile(const std::string& outputFilePath);
    /**
     * 标识规则为可选
     **/
    int handleOptional(const std::string& ruleName);
    /**
     * 处理A=a
     **/
    void handleTerminal(const std::string& ruleName, const std::string& terminalValue);
    /**
     * 处理A=B C D E
     **/
    void handleExpression(const std::string& ruleName, std::vector<ExpUnit>& expUnits);
    /**
     * 将表达式格式化为A=BC的格式
     * Convert A=BCD to A=XD, X=BC
     **/
    void alignExpression(const std::string& ruleName, std::vector<StringSet>& rightRules);
    /**
     * 再处理A=B C，其中B和C都是集合
     **/
    void arrangeExpression(const std::string& ruleName,const StringSet& rightRuleSet1,const StringSet& rightRuleSet2);
    /**
     * B和C都是集合，返回A
     * A = B C
     **/
    StringSet arrangeExpression(const StringSet& rightRuleSet1,const StringSet& rightRuleSet2);
    /**
     * A = B C D E
     * 合并B C D, 返回 X, X = B C D
     **/
    StringSet mergeExpression(std::vector<StringSet>& rightRules);
    /**
     * 保存规则的tag
     **/
    void saveTag(const std::string& ruleName, const std::string& tag);
    /**
     * 删除规则的tag
     **/
    void deleteTag(const std::string& ruleName);
    /**
     * 产生新的规则名
     **/
    std::string generateRuleName(char prefix = 'w', bool fullname = true);
    /**
     * 打印可以为空的规则名字
     **/
    void printOptionalRuleNames();
    /**
     * A=B
     * 根据B，查出A
     **/
    const StringSet& queryLeftRulesBySimpleRule(const std::string& value);
    /**
     * A=a
     * 根据a，查出A
     **/
    const StringSet& queryTerminal(const std::string& value);
    /**
     * 保存 A=a
     * 返回保存成功的个数
     **/
    int saveSingleTerminal(const std::string& ruleName, const std::string& value);
    /**
     * 保存 A=B
     * 返回保存成功的个数
     **/
    int saveSimpleRule(const std::string& leftRuleName, const std::string& rightRuleName);
    /**
     * 保存 A=BC
     * 返回保存成功的个数
     **/
    int saveComplexRule(const std::string& leftRuleName, const std::pair<std::string,std::string>& rightRules);
    /**
     * 保存 A=BC
     * 返回保存成功的A的集合
     **/
    StringSet& saveComplexRule(const std::pair<std::string,std::string>& rightRules);
    /**
     * 删除A=BC
     * 返回是否删除成功
     **/
    bool deleteComplexRule(const std::string& leftRuleName, const std::pair<std::string,std::string>& rightRules);
    /**
     * 删除A=B
     * 返回是否删除成功
     **/
    bool deleteSimpleRule(const std::string& leftRuleName, const std::string& rightRuleName);
    /**
     * 保存 A=a
     * For 运行时数据
     **/
    void saveSingleTerminal(SymbolType type, const std::string& value);
    /**
     * 保存 A=B
     * For 运行时数据
     **/
    void saveSimpleRule(SymbolType leftRuleType, SymbolType rightRuleType);
    /**
     * 保存 A=BC
     * For 运行时数据
     **/
    void saveComplexRule(SymbolType leftRuleType, const std::pair<SymbolType,SymbolType>& rightRuleTypes);
    /**
     * 将<a> = <NULL> | <b> <a> 的情况找出来，增加一个 <a> = <b>的规则
     **/
    void postHandleKleene();
    /**
     * 标记可以为空的规则名
     **/
    int markOptional(const std::string& ruleName);
    /**
     * 后处理，推断出两种规则为空的情况
     * 1, B = NULL, A = B             ==> A = NULL
     * 2, B = NULL, C = NULL, A = B C ==> A = NULL
     **/
    int postHandleNull();
    /**
     * 后处理，推断出新产生的LINK
     * 1, A = B C, B = NULL ==> A = C
     * 2, A = B C, C = NULL ==> A = B 
     **/
    int postHandleCreatedLink();
    /**
     * 将使用到的词典名字记录下来
     **/
    void markDictTag(const std::string& dictName, bool force = false);
    /**
     * 输出规则的关系图
     **/
    void printDiagramOfRules(const std::string& outputFilePath);
    /**
     * 保存规则名的修改关系
     **/
    bool saveRuleNameChangeMapping(const std::string& oldRuleName, const std::string& newRuleName);
    /**
     * 返回规则名的新名字
     **/
    const std::string& getChangedRuleName(const std::string& oldRuleName);
    /**
     * 查找比较宽的LINK链节点
     * 节点分叉数量大于3个
     **/
    void searchWidthLinkNodes(std::vector<std::pair<std::string,StringSet> >& problemNodes);
    /**
     * 后处理，从宽度方面减少LINK
     * 注意，不能影响原来的类似A=BC的规则
     * 1, A = B, A = C, A = D    ==>  A = A'
     **/
    void postReduceWidthOfLinkChians();
    /**
     * 后处理，从高度方面减少LINK
     * 注意，不能影响原来的类似A=BC的规则
     * 1, A = B, B = C, C = D    ==>  A = D
     **/
    void postReduceHeightOfLinkChians();
private:
    bool                                        m_debug;  //是否调试模式
    std::unordered_map<std::string,DictMatcher*>   m_innerDicts;   //内置词典
    StringSet                                   m_usedDictName;  //本规则使用到的词典集合
    std::string                                 m_grammarFilePath;  //*.gram路径
    jsgf_t*                                     m_grammar;  //旧语法对象
    int                                         m_ruleGeneratedIndex;
    std::string                                 m_grammarName; //语法名
    bool                                        m_isWildcard;  //是否支持WILDCARD
    std::unordered_map<std::pair<std::string,std::string>, StringSet, pair_hash>    m_complexRules;  //例如A=BC, key:BC, value:A
    std::unordered_map<std::string, StringSet>                                      m_simpleRules;   //例如A=B, key:B， value:A
    std::unordered_map<std::string, StringSet>                                      m_terminals;     //例如A=a, key:a，value:A
    std::unordered_map<std::string, std::string>                       m_tags;          //key:规则名，value: 标签值
    std::unordered_map<std::string, bool>       m_isOptionalFlags; //key:规则名，value: 该规则是否可选
    std::unordered_map<std::string, bool>       m_isRecursionFlags; //key:规则名，value: 该规则是否是递归
    std::unordered_map<std::string,std::string>           m_ruleNameMappingTable;  //规则名新旧名映射表
    /*****下面是用于运行时实际解析时候调用的****/
    std::unordered_map<std::pair<SymbolType,SymbolType>, SymbolTypeSet, pair_hash>  m_complexRulesRunning;
    std::unordered_map<SymbolType, SymbolTypeSet>                                   m_simpleRulesRunning;
    std::unordered_map<std::string, SymbolTypeSet>                                  m_terminalsRunning;
    std::unordered_map<SymbolType, std::string>                        m_tagsRunning;
    std::unordered_map<SymbolType, std::string>                        m_typeToName;
    std::string                                 m_nillString;        //空字符串
    SymbolTypeSet                               m_nillSymbolTypeSet; //空集合，用于返回空值
    StringSet                                   m_nillStringSet;     //空集合，用于返回空值
    StringSet                                   m_nomeanRuleNames;   //无意义规则名字集合
};



}

#endif
