#include <wfst/rule.h>
#include <util/splitter.h>
#include <jsgf/ckd_alloc.h>
#include <simple_log.h>

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include <functional>
#include <sstream>
#include <iterator>
#include <fstream>
#include <streambuf>
#include <regex>

namespace sogou
{

void setImportRuleSearchPath(const std::string &searchPath)
{
    if (-1 == setenv("JSGF_PATH", searchPath.c_str(), 1))
    {
        LOG_ERROR("setenv JSGF_PATH=%s failed", searchPath.c_str());
    }
    else
    {
        LOG_INFO("setenv JSGF_PATH=%s successfully", searchPath.c_str());
    }
}

RuleMgr::RuleMgr(const std::string &grammarFilePath,bool debug)
    : m_grammarFilePath(grammarFilePath), m_grammarName(""), m_ruleGeneratedIndex(0), 
    m_nillString(""),m_isWildcard(false),m_debug(debug)
{
    m_nomeanRuleNames.insert("<USER.nomean_begin>");
    m_nomeanRuleNames.insert("<USER.nomean_end>");
}

RuleMgr::~RuleMgr()
{
    jsgf_grammar_free(m_grammar);
    std::unordered_map<std::string,DictMatcher*>::iterator it = m_innerDicts.begin();
    for (; it != m_innerDicts.end(); ++it)
    {
        DictMatcher *matcher = it->second;
        delete matcher;
    }
}

bool RuleMgr::isNillSymbol(SymbolType type)
{
    return type == NillSymbol;
}

bool RuleMgr::isNillSymbolSet(const SymbolTypeSet &symbolTypeSet)
{
    return symbolTypeSet.empty();
}

bool RuleMgr::isWildcard()
{
    return m_isWildcard;
}

const std::string RuleMgr::getGrammarName()
{
    std::string domainName = m_grammarName;
    std::size_t foundPos = domainName.rfind(".");
    if(foundPos!=std::string::npos){
        domainName = domainName.substr(foundPos+1);
    }
    return domainName;
}

void RuleMgr::markDictTag(const std::string &dictName, bool force)
{
    bool mark = true;
    if (!force)
    {
        std::size_t foundPos1 = dictName.find("<USER.");
        std::size_t foundPos2 = dictName.find("<REG.");
        if (foundPos1 == std::string::npos && foundPos2 == std::string::npos)
        {
            mark = false;
        }
    }
    if (mark)
    {
        m_usedDictName.insert(dictName);
    }
}

bool RuleMgr::isDictTagUsed(const std::string &dictTagName)
{
    std::string fulleName = fullRuleName(dictTagName);
    if (m_usedDictName.count(fulleName))
    {
        return true;
    }
    return false;
}

SymbolType RuleMgr::transStringToInt(const std::string &str, bool saveReverse)
{
    std::size_t str_hash = std::hash<std::string>{}(str);
    if (saveReverse)
    {
        m_typeToName.insert(std::make_pair(str_hash, str));
    }
    return str_hash;
}

const std::string& RuleMgr::queryNameBySymbolType(SymbolType type)
{
    std::unordered_map<SymbolType, std::string>::iterator it;
    it = m_typeToName.find(type);
    if (it != m_typeToName.end())
    {
        return it->second;
    }
    return m_nillString;
}

int RuleMgr::handleOptional(const std::string &ruleName)
{
    std::unordered_map<std::string, bool>::iterator it = m_isOptionalFlags.find(ruleName);
    if(it!=m_isOptionalFlags.end()){
        return 0;
    }
    m_isOptionalFlags[ruleName] = true;
    if(m_debug){
        LOG_DEBUG("handleOptional : %s is Optional ", ruleName.c_str() );
    }
    return 1;
}

void RuleMgr::handleTerminal(const std::string &ruleName, const std::string &terminalValue)
{
    SplitResult splitResult;
    Splitter::SplitByChar(terminalValue, splitResult);
    if (splitResult.result.size() == 1)
    {
        saveSingleTerminal(ruleName, splitResult.result[0].word);
    }
    else
    {
        std::vector<StringSet> rightRuleNames;
        std::vector<sogou::SplitWord>::iterator it;
        for (it = splitResult.result.begin(); it != splitResult.result.end(); ++it)
        {
            SplitWord &item = *it;
            const StringSet &subRuleNameSet = queryTerminal(item.word);
            if (subRuleNameSet.empty())
            {
                std::string subRuleName = generateRuleName('w');
                saveSingleTerminal(subRuleName, item.word);
                StringSet set;
                set.insert(subRuleName);
                rightRuleNames.push_back(set);
            }
            else
            {
                rightRuleNames.push_back(subRuleNameSet);
            }
        }
        alignExpression(ruleName, rightRuleNames);
    }
}

void RuleMgr::handleExpression(const std::string &ruleName, std::vector<ExpUnit> &expUnits)
{
    std::vector<StringSet> rightRuleNames;
    std::vector<ExpUnit>::iterator it;
    for (it = expUnits.begin(); it != expUnits.end(); ++it)
    {
        ExpUnit &expUnit = *it;
        if (expUnit.isRule)
        {
            StringSet set;
            set.insert(expUnit.name);
            rightRuleNames.push_back(set);
            saveTag(expUnit.name, expUnit.tag); //保存tag
        }
        else
        {
            SplitResult splitResult;
            Splitter::SplitByChar(expUnit.name, splitResult);
            std::vector<sogou::SplitWord>::iterator iter;
            for (iter = splitResult.result.begin(); iter != splitResult.result.end(); ++iter)
            {
                SplitWord &item = *iter;
                const StringSet &subRuleNameSet = queryTerminal(item.word);
                if (subRuleNameSet.empty())
                {
                    std::string subRuleName = generateRuleName('w');
                    saveSingleTerminal(subRuleName, item.word);
                    StringSet set;
                    set.insert(subRuleName);
                    rightRuleNames.push_back(set);
                }
                else
                {
                    rightRuleNames.push_back(subRuleNameSet);
                }
            }
        }
    }
    alignExpression(ruleName, rightRuleNames);
}

/**
 * 将表达式格式化为A=BC的格式
 * Convert A=BCD to A=XD, X=BC
**/
void RuleMgr::alignExpression(const std::string &ruleName, std::vector<StringSet> &rightRuleNames)
{
    if ("" == ruleName || rightRuleNames.empty())
    {
        return;
    }
    if (rightRuleNames.size() == 1)
    {
        StringSet &rightRuleSet = rightRuleNames[0];
        StringSet::iterator rightRuleNamesIter = rightRuleSet.begin();
        for (; rightRuleNamesIter != rightRuleSet.end(); ++rightRuleNamesIter)
        {
            const std::string &rightRuleName = *rightRuleNamesIter;
            saveSimpleRule(ruleName, rightRuleName);
        }
    }
    else if (rightRuleNames.size() == 2)
    {
        StringSet &rightRuleSet1 = rightRuleNames[0]; //右一位置的规则集合
        StringSet &rightRuleSet2 = rightRuleNames[1]; //右二位置的规则集合
        arrangeExpression(ruleName, rightRuleSet1, rightRuleSet2);
    }
    else
    {
        StringSet rightRuleSet2 = rightRuleNames[rightRuleNames.size() - 1];
        rightRuleNames.pop_back();
        StringSet rightRuleSet1 = mergeExpression(rightRuleNames);
        arrangeExpression(ruleName, rightRuleSet1, rightRuleSet2);
    }
}

void RuleMgr::arrangeExpression(const std::string &ruleName, const StringSet &rightRuleSet1, const StringSet &rightRuleSet2)
{
    StringSet::const_iterator rightRuleSetIter1 = rightRuleSet1.cbegin();
    for (; rightRuleSetIter1 != rightRuleSet1.cend(); ++rightRuleSetIter1)
    {
        const std::string &rightRuleName1 = *rightRuleSetIter1;
        StringSet::const_iterator rightRuleSetIter2 = rightRuleSet2.cbegin();
        for (; rightRuleSetIter2 != rightRuleSet2.cend(); ++rightRuleSetIter2)
        {
            const std::string &rightRuleName2 = *rightRuleSetIter2;
            saveComplexRule(ruleName, std::make_pair(rightRuleName1, rightRuleName2));
        }
    }
}

StringSet RuleMgr::arrangeExpression(const StringSet &rightRuleSet1, const StringSet &rightRuleSet2)
{
    StringSet result;
    StringSet::const_iterator rightRuleSetIter1 = rightRuleSet1.cbegin();
    for (; rightRuleSetIter1 != rightRuleSet1.cend(); ++rightRuleSetIter1)
    {
        const std::string &rightRuleName1 = *rightRuleSetIter1;
        StringSet::const_iterator rightRuleSetIter2 = rightRuleSet2.cbegin();
        for (; rightRuleSetIter2 != rightRuleSet2.cend(); ++rightRuleSetIter2)
        {
            const std::string &rightRuleName2 = *rightRuleSetIter2;
            StringSet &targetRuleSet = saveComplexRule(std::make_pair(rightRuleName1, rightRuleName2));
            result.insert(targetRuleSet.begin(), targetRuleSet.end());
        }
    }
    return result;
}

/**
 * 合并表达式的多项
**/
StringSet RuleMgr::mergeExpression(std::vector<StringSet> &rightRules)
{
    if (rightRules.size() == 2)
    {
        StringSet &rightRuleSet1 = rightRules[0]; //右一位置的规则集合
        StringSet &rightRuleSet2 = rightRules[1]; //右二位置的规则集合
        StringSet result = arrangeExpression(rightRuleSet1, rightRuleSet2);
        return result;
    }
    else
    {
        StringSet rightRuleSet2 = rightRules[rightRules.size() - 1];
        rightRules.pop_back();
        StringSet rightRuleSet1 = mergeExpression(rightRules);
        StringSet result = arrangeExpression(rightRuleSet1, rightRuleSet2);
        return result;
    }
}

void RuleMgr::saveTag(const std::string &ruleName, const std::string &tag)
{
    if (ruleName == "" || tag == "")
    {
        return;
    }
    m_tags[ruleName] = tag;
}

void RuleMgr::deleteTag(const std::string &ruleName)
{
    m_tags.erase(ruleName);
}

std::string RuleMgr::generateRuleName(char prefix, bool fullname)
{
    m_ruleGeneratedIndex++;
    if(fullname){
        char temp[40];
        snprintf(temp, 40, "<%s.%c%06d>", m_grammarName.c_str(), prefix, m_ruleGeneratedIndex);
        return std::string(temp);
    }else{
        char temp[40];
        snprintf(temp, 40, "<%c%06d>", prefix, m_ruleGeneratedIndex);
        return std::string(temp);
    }
}

const StringSet &RuleMgr::queryLeftRulesBySimpleRule(const std::string &value)
{
    std::unordered_map<std::string, StringSet>::iterator it;
    it = m_simpleRules.find(value);
    if (it != m_simpleRules.end())
    {
        return it->second;
    }
    return m_nillStringSet;
}

const StringSet &RuleMgr::queryTerminal(const std::string &value)
{
    std::unordered_map<std::string, StringSet>::iterator it;
    it = m_terminals.find(value);
    if (it != m_terminals.end())
    {
        return it->second;
    }
    return m_nillStringSet;
}

int RuleMgr::saveSingleTerminal(const std::string &ruleName, const std::string &value)
{
    int count = 0;
    std::unordered_map<std::string, StringSet>::iterator it;
    it = m_terminals.find(value);
    if (it != m_terminals.end())
    {
        std::pair<StringSet::iterator,bool> ret;
        ret = it->second.insert(ruleName);
        if(ret.second == true){
            count = 1;
        }
    }
    else
    {
        std::unordered_set<std::string> set;
        set.insert(ruleName);
        m_terminals.insert(std::make_pair(value, set));
        count = 1;
    }
    if(count){
        if(m_debug){
            LOG_DEBUG("saveSingleTerminal : %s = %s ", ruleName.c_str(), value.c_str());
        }
    }
    return count;
}

int RuleMgr::saveSimpleRule(const std::string &leftRuleName, const std::string &rightRuleName)
{
    int count = 0;
    if (leftRuleName == "" || rightRuleName == "")
    {
        LOG_WARN("saveSimpleRule : empty rule : %s = %s", leftRuleName.c_str(), rightRuleName.c_str());
        return 0;
    }
    if (leftRuleName == rightRuleName)
    {
        //LOG_WARN("saveSimpleRule : invalid rule : %s = %s", leftRuleName.c_str(),rightRuleName.c_str());
        return 0;
    }
    std::unordered_map<std::string, StringSet>::iterator it;
    it = m_simpleRules.find(rightRuleName);
    if (it != m_simpleRules.end())
    {
        std::pair<StringSet::iterator,bool> ret;
        ret = it->second.insert(leftRuleName);
        if(ret.second == true){
            count = 1;
        }
    }
    else
    {
        std::unordered_set<std::string> set;
        set.insert(leftRuleName);
        m_simpleRules.insert(std::make_pair(rightRuleName, set));
        count = 1;
    }
    if(count){
        if(m_debug){
            LOG_DEBUG("saveSimpleRule : %s = %s ", leftRuleName.c_str(), rightRuleName.c_str());
        }
        if (!m_isWildcard && rightRuleName == "<WILDCARD>")
        {
            m_isWildcard = true;
        }
        markDictTag(rightRuleName);
    }
    return count;
}

int RuleMgr::saveComplexRule(const std::string &leftRuleName, const std::pair<std::string, std::string> &rightRules)
{
    int count = 0;
    if (leftRuleName == "" || rightRules.first == "" || rightRules.second == "")
    {
        LOG_WARN("saveSimpleRule : empty rule : %s = %s %s", leftRuleName.c_str(), rightRules.first.c_str(), rightRules.second.c_str());
        return 0;
    }
    /*if(rightRules.first == rightRules.second){
        LOG_WARN("saveSimpleRule : invalid rule : %s = %s %s", leftRuleName.c_str(),rightRules.first.c_str(),rightRules.second.c_str());
        return;
    }*/
    /*if(leftRuleName == rightRules.first && leftRuleName == rightRules.second){
        LOG_WARN("saveSimpleRule : invalid rule : %s = %s %s", leftRuleName.c_str(),rightRules.first.c_str(),rightRules.second.c_str());
        return;
    }*/
    std::unordered_map<std::pair<std::string, std::string>, StringSet, pair_hash>::iterator it;
    it = m_complexRules.find(rightRules);
    if (it != m_complexRules.end())
    {
        std::pair<StringSet::iterator,bool> ret;
        ret = it->second.insert(leftRuleName);
        if(ret.second == true){
            count = 1;
        }
    }
    else
    {
        std::unordered_set<std::string> set;
        set.insert(leftRuleName);
        m_complexRules.insert(std::make_pair(rightRules, set));
        count = 1;
    }
    if(count){
        if(m_debug){
            LOG_DEBUG("saveComplexRule : %s = %s %s ", leftRuleName.c_str(), rightRules.first.c_str(), rightRules.second.c_str());
        }
        if (rightRules.first == leftRuleName || rightRules.second == leftRuleName)
        { //记录递归标记
            m_isRecursionFlags[leftRuleName] = true;
        }
        if (!m_isWildcard && (rightRules.first == "<WILDCARD>" || rightRules.second == "<WILDCARD>"))
        {
            m_isWildcard = true;
        }
        markDictTag(rightRules.first);
        markDictTag(rightRules.second);
    }
    return count;
}

StringSet &RuleMgr::saveComplexRule(const std::pair<std::string, std::string> &rightRules)
{
    std::unordered_map<std::pair<std::string, std::string>, StringSet, pair_hash>::iterator it;
    it = m_complexRules.find(rightRules);
    if (it != m_complexRules.end())
    { //如果发现已经存在的推导，就直接使用
        return it->second;
    }
    std::string subRuleName = generateRuleName('r');
    saveComplexRule(subRuleName, rightRules);
    it = m_complexRules.find(rightRules);
    if (it != m_complexRules.end())
    {
        return it->second;
    }
    LOG_ERROR("saveComplexRule : some error occured!");
    return m_nillStringSet;
}

bool RuleMgr::deleteComplexRule(const std::string& leftRuleName, const std::pair<std::string,std::string>& rightRules)
{
    std::unordered_map<std::pair<std::string, std::string>, StringSet, pair_hash>::iterator it;
    it = m_complexRules.find(rightRules);
    if (it != m_complexRules.end())
    {
        StringSet & stringSet = it->second;
        StringSet::iterator it2 = stringSet.find(leftRuleName);
        if(it2 != stringSet.end())
        {
            stringSet.erase(it2);
            if(stringSet.size()==0){ //如果没有任何值留下，则删除整个规则记录
                m_complexRules.erase(it);
            }
            return true;
        }
    }
    return false;
}

bool RuleMgr::deleteSimpleRule(const std::string& leftRuleName, const std::string& rightRuleName)
{
    std::unordered_map<std::string, StringSet>::iterator it;
    it = m_simpleRules.find(rightRuleName);
    if (it != m_simpleRules.end())
    {
        StringSet & stringSet = it->second;
        StringSet::iterator it2 = stringSet.find(leftRuleName);
        if(it2 != stringSet.end())
        {
            stringSet.erase(it2);
            if(stringSet.size()==0){ //如果没有任何值留下，则删除整个规则记录
                m_simpleRules.erase(it);
            }
            return true;
        }
    }
    return false;
}

void RuleMgr::saveSingleTerminal(SymbolType type, const std::string &value)
{
    std::unordered_map<std::string, SymbolTypeSet>::iterator it;
    it = m_terminalsRunning.find(value);
    if (it != m_terminalsRunning.end())
    {
        it->second.insert(type);
    }
    else
    {
        std::unordered_set<SymbolType> set;
        set.insert(type);
        m_terminalsRunning.insert(std::make_pair(value, set));
    }
}

void RuleMgr::saveSimpleRule(SymbolType leftRuleType, SymbolType rightRuleType)
{
    std::unordered_map<SymbolType, SymbolTypeSet>::iterator it;
    it = m_simpleRulesRunning.find(rightRuleType);
    if (it != m_simpleRulesRunning.end())
    {
        it->second.insert(leftRuleType);
    }
    else
    {
        std::unordered_set<SymbolType> set;
        set.insert(leftRuleType);
        m_simpleRulesRunning.insert(std::make_pair(rightRuleType, set));
    }
}

void RuleMgr::saveComplexRule(SymbolType leftRuleType, const std::pair<SymbolType, SymbolType> &rightRuleTypes)
{
    std::unordered_map<std::pair<SymbolType, SymbolType>, SymbolTypeSet, pair_hash>::iterator it;
    it = m_complexRulesRunning.find(rightRuleTypes);
    if (it != m_complexRulesRunning.end())
    {
        it->second.insert(leftRuleType);
    }
    else
    {
        std::unordered_set<SymbolType> set;
        set.insert(leftRuleType);
        m_complexRulesRunning.insert(std::make_pair(rightRuleTypes, set));
    }
}

bool RuleMgr::checkValid()
{
    //检查是否简单规则的右侧是否包含可以为空的规则
    std::unordered_map<std::string, bool>::iterator it5;
    for (it5 = m_isOptionalFlags.begin(); it5 != m_isOptionalFlags.end(); ++it5)
    {
        const std::string &ruleName = it5->first;
        std::unordered_map<std::string, StringSet>::iterator it;
        it = m_simpleRules.find(ruleName);
        if (it != m_simpleRules.end())
        {
            StringSet &leftRuleSet = it->second;
            StringSet::iterator leftRuleIter = leftRuleSet.begin();
            for (; leftRuleIter != leftRuleSet.end(); ++leftRuleIter)
            {
                const std::string &leftRuleName = *leftRuleIter;
                //LOG_WARN("!!!! Rule is Optional and used to simple rule : %s = %s ", leftRuleName.c_str(), ruleName.c_str());
            }
        }
    }
    return true;
}

bool RuleMgr::isOptional(const std::string &ruleName)
{
    std::unordered_map<std::string, bool>::iterator it;
    it = m_isOptionalFlags.find(ruleName);
    if (it != m_isOptionalFlags.end())
    {
        return true;
    }
    return false;
}

bool RuleMgr::isRecursion(const std::string &ruleName)
{
    std::unordered_map<std::string, bool>::iterator it;
    it = m_isRecursionFlags.find(ruleName);
    if (it != m_isRecursionFlags.end())
    {
        return true;
    }
    return false;
}

void RuleMgr::displayJsgfRules()
{
    LOG_DEBUG("======================================Jsgf Grammar : %s======================================", m_grammarName.c_str());
    hash_table_t *rules = m_grammar->rules;
    jsgf_rule_iter_t *itor = NULL;
    for (itor = jsgf_rule_iter(m_grammar); itor; itor = jsgf_rule_iter_next(itor))
    {
        const char *key = itor->ent->key;
        jsgf_rule_t *rule = jsgf_rule_iter_rule(itor);
        jsgf_rhs_t *rhs;
        std::ostringstream rule_ostr;
        rule_ostr << " " << rule->name << " = ";
        for (rhs = rule->rhs; rhs; rhs = rhs->alt)
        {
            if (rhs != rule->rhs)
            {
                rule_ostr << " | ";
            }
            gnode_t *gn;
            for (gn = rhs->atoms; gn; gn = gnode_next(gn))
            {
                jsgf_atom_t *atom = (jsgf_atom_t *)gnode_ptr(gn);
                if (gn != rhs->atoms)
                {
                    rule_ostr << " , ";
                }
                if (jsgf_atom_is_rule(atom))
                {
                    rule_ostr << " $" << fullRuleName(atom->name).c_str();
                }
                else
                {
                    rule_ostr << " " << atom->name;
                }
                if (glist_count(atom->tags) > 0)
                {
                    rule_ostr << " @ ";
                }
                gnode_t *gn_tag;
                for (gn_tag = atom->tags; gn_tag; gn_tag = gnode_next(gn_tag))
                {
                    const char *tag_str = static_cast<const char *>(gn_tag->data.ptr);
                    if (gn_tag != atom->tags)
                    {
                        rule_ostr << " $ ";
                    }
                    rule_ostr << tag_str;
                }
            }
        }
        LOG_DEBUG("%s", rule_ostr.str().c_str());
    }
    LOG_DEBUG("======================================================================");
}

void RuleMgr::displayRules()
{
    LOG_DEBUG("==============CYK Grammar : %s==============", m_grammarName.c_str());
    LOG_DEBUG("Complex Rules: ");
    std::unordered_map<std::pair<std::string, std::string>, StringSet, pair_hash>::iterator complexRulesIter;
    for (complexRulesIter = m_complexRules.begin(); complexRulesIter != m_complexRules.end(); ++complexRulesIter)
    {
        std::unordered_set<std::string> &stringSet = complexRulesIter->second;
        for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
        {
            LOG_DEBUG("%s = %s %s", (*it2).c_str(), complexRulesIter->first.first.c_str(), complexRulesIter->first.second.c_str());
        }
    }
    LOG_DEBUG("Simple Rules: ");
    std::unordered_map<std::string, StringSet>::iterator simpleRulesIter;
    for (simpleRulesIter = m_simpleRules.begin(); simpleRulesIter != m_simpleRules.end(); ++simpleRulesIter)
    {
        std::unordered_set<std::string> &stringSet = simpleRulesIter->second;
        for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
        {
            LOG_DEBUG("%s = %s", (*it2).c_str(), simpleRulesIter->first.c_str());
        }
    }
    LOG_DEBUG("Optional Rules: ");
    std::unordered_map<std::string, bool>::iterator it5;
    for (it5 = m_isOptionalFlags.begin(); it5 != m_isOptionalFlags.end(); ++it5)
    {
        LOG_DEBUG("%s", it5->first.c_str());
    }
    LOG_DEBUG("Terminals Rules: ");
    std::unordered_map<std::string, StringSet>::iterator terminalsIter;
    for (terminalsIter = m_terminals.begin(); terminalsIter != m_terminals.end(); ++terminalsIter)
    {
        std::unordered_set<std::string> &stringSet = terminalsIter->second;
        for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
        {
            LOG_DEBUG("%s = %s", (*it2).c_str(), terminalsIter->first.c_str());
        }
    }    
    LOG_DEBUG("Tags: ");
    std::unordered_map<std::string, std::string>::iterator it4;
    for (it4 = m_tags.begin(); it4 != m_tags.end(); ++it4)
    {
        LOG_DEBUG("%s = %s", it4->first.c_str(), it4->second.c_str());
    }
    LOG_DEBUG("Recursion Rules: ");
    std::unordered_map<std::string, bool>::iterator it6;
    for (it6 = m_isRecursionFlags.begin(); it6 != m_isRecursionFlags.end(); ++it6)
    {
        LOG_DEBUG("%s", it6->first.c_str());
    }
    LOG_DEBUG("==============================================");
    LOG_DEBUG("Statistic Data: ");
    LOG_DEBUG("Complex Rules: %d", m_complexRules.size());
    LOG_DEBUG("Simple Rules: %d", m_simpleRules.size());
    LOG_DEBUG("Terminals Rules: %d", m_terminals.size());
    LOG_DEBUG("Tags : %d", m_tags.size());
    LOG_DEBUG("Optional : %d", m_isOptionalFlags.size());
    LOG_DEBUG("Recursion : %d", m_isRecursionFlags.size());
    LOG_DEBUG("==============================================");
    LOG_DEBUG("Running Statistic Data: ");
    LOG_DEBUG("Complex Rules: %d", m_complexRulesRunning.size());
    LOG_DEBUG("Simple Rules: %d", m_simpleRulesRunning.size());
    LOG_DEBUG("Terminals Rules: %d", m_terminalsRunning.size());
    LOG_DEBUG("Tags : %d", m_tagsRunning.size());
    LOG_DEBUG("==============================================");
}

/**
 * 将规则转换到运行时状态
**/
void RuleMgr::transToRunningState()
{
    LOG_DEBUG("###############begin to transToRunningState ");
    //m_terminalsRunning
    std::unordered_map<std::string, StringSet>::iterator terminalsIter;
    for (terminalsIter = m_terminals.begin(); terminalsIter != m_terminals.end(); ++terminalsIter)
    {
        std::unordered_set<std::string> &stringSet = terminalsIter->second;
        for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
        {
            saveSingleTerminal(transStringToInt(*it2, true), terminalsIter->first);
        }
    }
    //m_complexRulesRunning
    std::unordered_map<std::pair<std::string, std::string>, StringSet, pair_hash>::iterator complexRulesIter;
    for (complexRulesIter = m_complexRules.begin(); complexRulesIter != m_complexRules.end(); ++complexRulesIter)
    {
        std::unordered_set<std::string> &stringSet = complexRulesIter->second;
        for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
        {
            saveComplexRule(
                transStringToInt(*it2, true),
                std::make_pair(transStringToInt(complexRulesIter->first.first, true), transStringToInt(complexRulesIter->first.second, true)));
        }
    }
    //m_simpleRulesRunning
    std::unordered_map<std::string, StringSet>::iterator simpleRulesIter;
    for (simpleRulesIter = m_simpleRules.begin(); simpleRulesIter != m_simpleRules.end(); ++simpleRulesIter)
    {
        std::unordered_set<std::string> &stringSet = simpleRulesIter->second;
        for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
        {
            saveSimpleRule(transStringToInt(*it2, true), transStringToInt(simpleRulesIter->first, true));
        }
    }
    //m_tagsRunning
    std::unordered_map<std::string, std::string>::iterator it4;
    for (it4 = m_tags.begin(); it4 != m_tags.end(); ++it4)
    {
        m_tagsRunning.insert(std::make_pair(transStringToInt(it4->first, true), it4->second));
    }
}

const SymbolTypeSet &RuleMgr::queryTerminalTypeByValue(const std::string &value)
{
    std::unordered_map<std::string, SymbolTypeSet>::iterator it;
    it = m_terminalsRunning.find(value);
    if (it != m_terminalsRunning.end())
    {
        return it->second;
    }
    return m_nillSymbolTypeSet;
}

const SymbolTypeSet &RuleMgr::queryTargetTypeByPair(SymbolType type1, SymbolType type2)
{
    std::unordered_map<std::pair<SymbolType, SymbolType>, SymbolTypeSet, pair_hash>::iterator it;
    it = m_complexRulesRunning.find(std::make_pair(type1, type2));
    if (it != m_complexRulesRunning.end())
    {
        return it->second;
    }
    return m_nillSymbolTypeSet;
}

const SymbolTypeSet &RuleMgr::queryTargetLink(SymbolType type1)
{
    std::unordered_map<SymbolType, SymbolTypeSet>::iterator it;
    it = m_simpleRulesRunning.find(type1);
    if (it != m_simpleRulesRunning.end())
    {
        return it->second;
    }
    return m_nillSymbolTypeSet;
}

const std::string *RuleMgr::queryTag(SymbolType type1)
{
    std::unordered_map<SymbolType, std::string>::iterator it;
    it = m_tagsRunning.find(type1);
    if (it != m_tagsRunning.end())
    {
        return &(it->second);
    }
    return NULL;
}

std::string RuleMgr::fullRuleName(const std::string &name)
{
    if (strcmp(name.c_str(), "<NULL>") == 0)
    {
        return name;
    }
    if (strcmp(name.c_str(), "<VOID>") == 0)
    {
        return name;
    }
    if (strcmp(name.c_str(), "<WILDCARD>") == 0)
    {
        return name;
    }
    if (strncmp(name.c_str(), "<USER.", 6) == 0)
    {
        return name;
    }
    if (strncmp(name.c_str(), "<REG.", 5) == 0)
    {
        return name;
    }
    if (strncmp(name.c_str(), "USER.", 5) == 0)
    {
        std::string ruleName = "<";
        ruleName += name;
        ruleName += ">";
        return ruleName;
    }
    if (strncmp(name.c_str(), "REG.", 4) == 0)
    {
        std::string ruleName = "<";
        ruleName += name;
        ruleName += ">";
        return ruleName;
    }
    char * ret_ptr = jsgf_fullname(m_grammar, name.c_str());
    std::string rName(ret_ptr);
    ckd_free(ret_ptr);
    return rName;
}

void RuleMgr::matchInnerDict(const SplitResult &splitResult, std::vector<MatchUnit> &result)
{
    std::unordered_map<std::string,DictMatcher*>::iterator it = m_innerDicts.begin();
    for (; it != m_innerDicts.end(); ++it)
    {
        DictMatcher *dictMatcher = it->second;
        std::vector<MatchUnit> myset;
        dictMatcher->match(splitResult, myset);
        for (std::vector<MatchUnit>::iterator iter = myset.begin(); iter != myset.end(); iter++)
        {
            result.push_back(*iter);
        }
    }
}


/**
 * 将<a> = <NULL> | <b> <a> 的情况找出来，增加一个 <a> = <b>的规则
**/
void RuleMgr::postHandleKleene()
{
    LOG_DEBUG("###############begin to postHandleKleene ");
    std::unordered_map<std::string, bool>::iterator it5;
    for (it5 = m_isOptionalFlags.begin(); it5 != m_isOptionalFlags.end(); ++it5)
    { //遍历所有的可以为空的规则名
        const std::string &ruleName = it5->first;
        bool isRecur = isRecursion(ruleName); //该规则也是递归的
        if (isRecur)
        {
            bool found = false;
            std::pair<std::string, std::string> rightExpr;
            std::unordered_map<std::pair<std::string, std::string>, StringSet, pair_hash>::iterator it;
            for (it = m_complexRules.begin(); it != m_complexRules.end(); ++it)
            { //遍历所有的复杂规则
                std::unordered_set<std::string> &stringSet = it->second;
                if (stringSet.count(ruleName))
                { //如果有复杂规则可以推导出该递归
                    rightExpr = it->first;
                    if (rightExpr.first == ruleName && rightExpr.second != ruleName)
                    {
                        saveSimpleRule(ruleName, rightExpr.second);
                    }
                    if (rightExpr.first != ruleName && rightExpr.second == ruleName)
                    {
                        saveSimpleRule(ruleName, rightExpr.first);
                    }
                }
            }
        }
    }
}

void RuleMgr::printOptionalRuleNames(){
    std::ostringstream rulenames_ostr;
    std::unordered_map<std::string, bool>::iterator it5;
    for (it5 = m_isOptionalFlags.begin(); it5 != m_isOptionalFlags.end(); ++it5)
    {   
        rulenames_ostr << it5->first << " ";
    }
    LOG_DEBUG("printOptionalRuleNames : %s",rulenames_ostr.str().c_str());
}

int RuleMgr::markOptional(const std::string &ruleName)
{
    int count = 0;
    count += handleOptional(ruleName);
    const StringSet &targetRuleStringSet = queryLeftRulesBySimpleRule(ruleName);
    StringSet::const_iterator it = targetRuleStringSet.cbegin();
    for (; it != targetRuleStringSet.cend(); ++it)
    {
        const std::string& leftRuleName = *it;
        if(m_debug){
            LOG_DEBUG("find up link : %s <= %s",leftRuleName.c_str(),ruleName.c_str());
        }
        count += handleOptional(leftRuleName);
        count += markOptional(leftRuleName);
    }
    return count;
}

/**
 * 处理所有新增的可以为NULL的规则
 * 返回本次新标记的可以NULL的规则数量
**/
int RuleMgr::postHandleNull(){
    LOG_DEBUG("########begin to postHandleNull ########");
    if(m_debug){
        printOptionalRuleNames();
    }
    int count = 0;
    //将<a> = <NULL> && <b> = <a> 的情况找出来，增加一个 <b> = <NULL> 的规则
    std::unordered_map<std::string, StringSet>::iterator simpleRulesIter;
    for (simpleRulesIter = m_simpleRules.begin(); simpleRulesIter != m_simpleRules.end(); ++simpleRulesIter)
    {
        std::unordered_set<std::string> &stringSet = simpleRulesIter->second;
        for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
        {
            std::string leftRuleName = *it2;
            std::string rightRuleName = simpleRulesIter->first;
            if(isOptional(rightRuleName)){
                count += markOptional(leftRuleName);
            }
        }
    }
    //将<a> = <NULL> && <b> = <NULL> && <c> = <a> <b> 的情况找出来，增加一个 <c> = <NULL> 的规则
    std::unordered_map<std::pair<std::string, std::string>, StringSet, pair_hash>::iterator complexRulesIter;
    for (complexRulesIter = m_complexRules.begin(); complexRulesIter != m_complexRules.end(); ++complexRulesIter)
    {
        std::unordered_set<std::string> &stringSet = complexRulesIter->second;
        for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
        {
            std::string leftRuleName = *it2;
            std::string rightRuleName1 = complexRulesIter->first.first;
            std::string rightRuleName2 = complexRulesIter->first.second;
            if(isOptional(rightRuleName1) && isOptional(rightRuleName2)){
                count += markOptional(leftRuleName);
            }
        }
    }
    return count;
}

/**
 * 处理所有新增的Link
 * 返回本次新增的Link的数量
**/
int RuleMgr::postHandleCreatedLink(){
    LOG_DEBUG("########begin to postHandleCreatedLink ########");
    int count = 0;
    //将<a> = <b> <c> && <c> = <NULL> 的情况找出来，增加一个 <a> = <b>的规则
    std::unordered_map<std::pair<std::string, std::string>, StringSet, pair_hash>::iterator complexRulesIter;
    for (complexRulesIter = m_complexRules.begin(); complexRulesIter != m_complexRules.end(); ++complexRulesIter)
    {
        std::unordered_set<std::string> &stringSet = complexRulesIter->second;
        for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
        {
            std::string leftRuleName = *it2;
            std::string rightRuleName1 = complexRulesIter->first.first;
            std::string rightRuleName2 = complexRulesIter->first.second;
            if(isOptional(rightRuleName1)){
                count += saveSimpleRule(leftRuleName, rightRuleName2);
            }
            if(isOptional(rightRuleName2)){
                count += saveSimpleRule(leftRuleName, rightRuleName1);
            }
        }
    }
    return count;
}

void RuleMgr::printDiagramOfRules(const std::string& outputFilePath){
    std::ofstream out(outputFilePath);
    out << "digraph " << getGrammarName() << "{ " << std::endl;
    out << "graph [splines=true overlap=false];" << std::endl;
    out << "node [shape=none];" << std::endl;
    std::unordered_map<std::pair<std::string, std::string>, StringSet, pair_hash>::iterator complexRulesIter;
    for (complexRulesIter = m_complexRules.begin(); complexRulesIter != m_complexRules.end(); ++complexRulesIter)
    {
        std::unordered_set<std::string> &stringSet = complexRulesIter->second;
        for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
        {
            out << *it2 << "->" << complexRulesIter->first.first << " [color=blue]" << ";" << std::endl;
            out << *it2 << "->" << complexRulesIter->first.second << " [color=blue]" << ";" << std::endl;
        }
    }
    std::unordered_map<std::string, StringSet>::iterator simpleRulesIter;
    for (simpleRulesIter = m_simpleRules.begin(); simpleRulesIter != m_simpleRules.end(); ++simpleRulesIter)
    {
        std::unordered_set<std::string> &stringSet = simpleRulesIter->second;
        for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
        {
            out << *it2 << "->" << simpleRulesIter->first << " [color=red]" << ";" << std::endl;
        }
    }
    out << "} " << std::endl;
}

bool RuleMgr::saveRuleNameChangeMapping(const std::string& oldRuleName, const std::string& newRuleName)
{
    std::unordered_map<std::string,std::string>::iterator it = m_ruleNameMappingTable.find(oldRuleName);
    if(it != m_ruleNameMappingTable.end()){
        if( it->second != newRuleName ){
            LOG_ERROR("saveRuleNameChangeMapping failed %s->%s : existed %s->%s",
                oldRuleName.c_str(), newRuleName.c_str(),
                oldRuleName.c_str(), it->second.c_str()
            );
            return false;
        }else{
            return true;
        }
    }else{
        m_ruleNameMappingTable.insert(std::make_pair(oldRuleName,newRuleName));
        return true;
    }
}

const std::string& RuleMgr::getChangedRuleName(const std::string& oldRuleName)
{
    std::unordered_map<std::string,std::string>::iterator it = m_ruleNameMappingTable.find(oldRuleName);
    if(it != m_ruleNameMappingTable.end()){
        return it->second;
    }
    return oldRuleName;
}

void RuleMgr::searchWidthLinkNodes(std::vector<std::pair<std::string,StringSet> >& problemNodes)
{
    if(m_debug){
        LOG_DEBUG("######## searchWidthLinkNodes ########");
    }
    int maxWidth = 3;
    std::unordered_map<std::string, StringSet>::iterator simpleRulesIter;
    for (simpleRulesIter = m_simpleRules.begin(); simpleRulesIter != m_simpleRules.end(); ++simpleRulesIter)
    {
        const std::string& rightRuleName = simpleRulesIter->first;
        std::unordered_set<std::string> &leftRuleNamesSet = simpleRulesIter->second;
        if(leftRuleNamesSet.size()>maxWidth){
            problemNodes.push_back(std::make_pair(rightRuleName,leftRuleNamesSet));
        }
    }
    std::vector<std::pair<std::string,StringSet> >::iterator problemNodesIter = problemNodes.begin();
    for(;problemNodesIter!=problemNodes.end();++problemNodesIter){
        std::pair<std::string,StringSet> & item = *problemNodesIter;
        if(m_debug){
            LOG_DEBUG("problemNode : %s = %d ", item.first.c_str(), item.second.size() );
        }
    }
}

void RuleMgr::postReduceWidthOfLinkChians(){
    LOG_DEBUG("########begin to postReduceWidthOfLinkChians ########");
    //1,找出宽的那些LINK节点
    std::vector<std::pair<std::string,StringSet> > problemNodes;
    searchWidthLinkNodes(problemNodes);
    //2,再进行缩减宽度
    //拷贝两个集合，用于操作元素
    std::unordered_map<std::pair<std::string,std::string>, StringSet, pair_hash> copied_complexRules;
    copied_complexRules.insert(m_complexRules.begin(),m_complexRules.end());
    std::unordered_map<std::string, StringSet> copied_simpleRules;
    copied_simpleRules.insert(m_simpleRules.begin(),m_simpleRules.end());
    std::vector<std::pair<std::string,StringSet> >::iterator problemNodesIter = problemNodes.begin();
    for(;problemNodesIter!=problemNodes.end();++problemNodesIter){
        std::pair<std::string,StringSet> & item = *problemNodesIter; //一个问题规则处理一轮
        std::string rightRuleName = getChangedRuleName(item.first);
        StringSet& needChangedRuleNamesSet = item.second;
        std::string newChangedRuleName = generateRuleName('e');
        //处理简单LINK的规则名替换
        std::unordered_map<std::string, StringSet>::iterator simpleRulesIter;
        for (simpleRulesIter = copied_simpleRules.begin(); simpleRulesIter != copied_simpleRules.end(); ++simpleRulesIter)
        {
            std::unordered_set<std::string> &stringSet = simpleRulesIter->second;
            for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
            {
                std::string targetLeftRuleName = *it2;
                std::string targetRightRuleName = simpleRulesIter->first;
                //遍历需要被替换的元素
                StringSet::iterator needChangedRuleNamesIter = needChangedRuleNamesSet.begin();
                for(;needChangedRuleNamesIter!=needChangedRuleNamesSet.end();++needChangedRuleNamesIter){
                    std::string needChangedRuleName = getChangedRuleName(*needChangedRuleNamesIter);
                    if(targetLeftRuleName == needChangedRuleName){  //换左边
                        deleteSimpleRule(targetLeftRuleName,targetRightRuleName);
                        saveSimpleRule(newChangedRuleName,targetRightRuleName);
                    }
                    if(targetRightRuleName == needChangedRuleName){ //换右边
                        deleteSimpleRule(targetLeftRuleName,targetRightRuleName);
                        saveSimpleRule(targetLeftRuleName,newChangedRuleName);
                    }
                }
            }
        }
        //处理复杂规则的规则名替换
        std::unordered_map<std::pair<std::string, std::string>, StringSet, pair_hash>::iterator complexRulesIter;
        for (complexRulesIter = copied_complexRules.begin(); complexRulesIter != copied_complexRules.end(); ++complexRulesIter)
        {
            std::unordered_set<std::string> &stringSet = complexRulesIter->second;
            for (std::unordered_set<std::string>::iterator it2 = stringSet.begin(); it2 != stringSet.end(); ++it2)
            {
                std::string targetLeftRuleName = *it2;
                std::string targetRightRuleName1 = complexRulesIter->first.first;
                std::string targetRightRuleName2 = complexRulesIter->first.second;
                //遍历需要被替换的元素
                StringSet::iterator needChangedRuleNamesIter = needChangedRuleNamesSet.begin();
                for(;needChangedRuleNamesIter!=needChangedRuleNamesSet.end();++needChangedRuleNamesIter){
                    std::string needChangedRuleName = getChangedRuleName(*needChangedRuleNamesIter);
                    if(targetLeftRuleName == needChangedRuleName){  //换左边
                        deleteComplexRule(targetLeftRuleName,std::make_pair(targetRightRuleName1,targetRightRuleName2));
                        saveComplexRule(newChangedRuleName,std::make_pair(targetRightRuleName1,targetRightRuleName2));
                    }
                    if(targetRightRuleName1 == needChangedRuleName){ //换右边1
                        deleteComplexRule(targetLeftRuleName,std::make_pair(targetRightRuleName1,targetRightRuleName2));
                        saveComplexRule(targetLeftRuleName,std::make_pair(newChangedRuleName,targetRightRuleName2));
                    }
                    if(targetRightRuleName2 == needChangedRuleName){ //换右边2
                        deleteComplexRule(targetLeftRuleName,std::make_pair(targetRightRuleName1,targetRightRuleName2));
                        saveComplexRule(targetLeftRuleName,std::make_pair(targetRightRuleName1,newChangedRuleName));
                    }
                }
            }
        }
        //保存新的规则名
        StringSet::iterator needChangedRuleNamesIter2 = needChangedRuleNamesSet.begin();
        for(;needChangedRuleNamesIter2!=needChangedRuleNamesSet.end();++needChangedRuleNamesIter2){
            std::string needChangedRuleName = getChangedRuleName(*needChangedRuleNamesIter2);
            saveRuleNameChangeMapping(needChangedRuleName,newChangedRuleName);
        }
        ////
    }
    //3,找出宽的那些LINK节点
    std::vector<std::pair<std::string,StringSet> > problemNodes2;
    searchWidthLinkNodes(problemNodes2);
}

void RuleMgr::postReduceHeightOfLinkChians(){
    LOG_DEBUG("########begin to postReduceHeightOfLinkChians ########");
    //1,先找出最基础的LINK节点
    std::vector<std::string> baseNodes;
    //2,再进行缩减高度

}

static char *randStr(char *str, const int len)
{
    srand((unsigned)time(0));
    int i;
    for(i=0;i<len-1;++i)
        str[i]='A'+rand()%25;
    str[i]='\0';
    return str;
}

static std::string& replaceAll(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos))!= std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();//Handles case where 'to' is a substring of 'from'
    }
    return str;
}

void RuleMgr::searchMatch(const std::string& content, const std::string& regexExpr, std::unordered_map<std::string, int>& matchResult){
    std::regex square_bracket_regex(regexExpr);
    auto words_begin = std::sregex_iterator(content.begin(), content.end(), square_bracket_regex);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string match_str = match.str();
        std::unordered_map<std::string, int>::iterator it = matchResult.find(match_str);
        if(it!=matchResult.end()){
            ++it->second;
        }else{
            matchResult.insert(std::make_pair(match_str,1));
        }
    }
}

bool RuleMgr::preHandleGramFile(const std::string& outputFilePath){
    std::locale old;
    std::locale::global(std::locale("en_US.UTF-8"));
    LOG_DEBUG("########begin to preHandleGramFile ########");
    //把整个语法文件读入内存
    std::ifstream in(m_grammarFilePath);
    std::string gramContent((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
    //定义需要替换的表达式
    std::vector<std::string> regexExprs;
    regexExprs.push_back("(\\[<[a-zA-Z0-9\\._]*>\\])");
    std::vector<std::string> needAppendLines;
    //
    std::vector<std::string>::iterator exprIter = regexExprs.begin();
    for(;exprIter!=regexExprs.end();++exprIter){
        std::string & expr = *exprIter;
        //1,寻找出所有的可能存在问题的规则名
        std::unordered_map<std::string, int> matchResult;
        searchMatch(gramContent,expr,matchResult);
        //2,对规则名地方进行替换
        std::unordered_map<std::string, int>::iterator matchedIter = matchResult.begin();
        for(;matchedIter!=matchResult.end();++matchedIter){
            LOG_DEBUG("%s = %d", matchedIter->first.c_str(), matchedIter->second);
            if(matchedIter->second>1){  //出现两次以上才替换
                std::string oldRuleExpr =  matchedIter->first;
                std::string newRuleName = generateRuleName('p',false);
                needAppendLines.push_back( newRuleName + " = " + oldRuleExpr + ";" );
                replaceAll(gramContent,oldRuleExpr,newRuleName);
            }
        }
    }

    //创建输出文件
    std::ofstream out(outputFilePath);
    out << gramContent << std::endl;
    out << std::endl << std::endl << std::endl << std::endl;
    std::vector<std::string>::iterator it = needAppendLines.begin();
    for(;it!=needAppendLines.end();++it){
        std::string & line = *it;
        out << line << std::endl;
    }
    std::locale::global(old);
    return true;
}

/**
* 将jsgf语法对象转换为CYK规则
**/
bool RuleMgr::init()
{
    //对于*.gram文件进行预处理
    int pos = m_grammarFilePath.find_last_of('/');
    std::string fileName(m_grammarFilePath.substr(pos + 1) );
    char randomFileName[10];
    randStr(randomFileName,10);
    std::string replacedGramFilePath = "./";
    replacedGramFilePath += randomFileName;
    replacedGramFilePath += "_";
    replacedGramFilePath += fileName;
    preHandleGramFile(replacedGramFilePath);
    //使用替换后的gram文件
    m_grammar = jsgf_parse_file(replacedGramFilePath.c_str(), nullptr);
    if (NULL == m_grammar)
    {
        LOG_ERROR("grammar file %s jsgf_parse_file failed", m_grammarFilePath.c_str());
        return false;
    }
    if(!m_debug){
        remove(replacedGramFilePath.c_str());
    }
    this->m_grammarName = m_grammar->name;
    hash_table_t *rules = m_grammar->rules;
    jsgf_rule_iter_t *itor = NULL;
    for (itor = jsgf_rule_iter(m_grammar); itor; itor = jsgf_rule_iter_next(itor))
    {
        const char *key = itor->ent->key;
        jsgf_rule_t *rule = jsgf_rule_iter_rule(itor);
        jsgf_rhs_t *rhs;
        int rhsIndex = 0;
        std::string ruleName = rule->name;
        std::unordered_set<std::string> patternList;
        for (rhs = rule->rhs; rhs; rhs = rhs->alt)
        {
            rhsIndex++;
            int atomIndex = 0, atomCount = glist_count(rhs->atoms);
            std::vector<ExpUnit> expUnits;
            gnode_t *gn;
            for (gn = rhs->atoms; gn; gn = gnode_next(gn))
            {
                atomIndex++;
                jsgf_atom_t *atom = (jsgf_atom_t *)gnode_ptr(gn);
                ExpUnit expUnit;
                if (atomCount == 1)
                {
                    if (jsgf_atom_is_rule(atom))
                    {
                        if (strcmp(atom->name, "<NULL>") == 0)
                        { //右侧是一个单个<NULL>
                            handleOptional(ruleName);
                            continue;
                        }
                        else
                        { //右侧是一个单个普通规则,可能还包括<WILDCARD>,<VOID>等
                            expUnit.name = fullRuleName(atom->name);
                            expUnit.isRule = true;
                        }
                    }
                    else
                    { //右侧是一个单个字符串
                        patternList.insert(atom->name);
                        //handleTerminal(ruleName,atom->name);
                        continue;
                    }
                }
                else
                {
                    if (jsgf_atom_is_rule(atom))
                    { //右侧遇到一个普通规则,可能还包括<WILDCARD>,<VOID>等
                        expUnit.name = fullRuleName(atom->name);
                        expUnit.isRule = true;
                    }
                    else
                    { //右侧遇到一个字符串
                        expUnit.name = atom->name;
                        expUnit.isRule = false;
                    }
                }
                gnode_t *gn_tag;
                for (gn_tag = atom->tags; gn_tag; gn_tag = gnode_next(gn_tag))
                {
                    const char *tag_str = static_cast<const char *>(gn_tag->data.ptr);
                    expUnit.tag = tag_str;
                    break;
                }
                expUnits.push_back(expUnit);
            }
            if (!expUnits.empty())
            {
                handleExpression(ruleName, expUnits);
            }
        }
        //检查是否可以创建内部词典
        if (!patternList.empty())
        {
            //创建内部词典
            DictMatcher *matcher = new DictMatcher(ruleName, patternList);
            if(matcher->init()){
                std::unordered_map<std::string,DictMatcher*>::iterator it = m_innerDicts.find(ruleName);
                if(it!=m_innerDicts.end()){
                    LOG_ERROR("Dupliated inner dict : %s", ruleName.c_str());
                    delete matcher;
                }else{
                    m_innerDicts.insert(std::make_pair(ruleName,matcher));
                    markDictTag(ruleName,true);
                }
            }else{
                delete matcher;
            }
        }
    }

    //处理*问题
    postHandleKleene();

    //后处理逻辑
    //因为“标记可以为空”和“产生了新的LINK”之间存在相互影响，所以必须让这两个处理都没有可处理的内容（状体稳定）之后，才表示处理完毕
    bool loopFlag = true;
    do{
        int count1 = postHandleNull();
        int count2 = postHandleCreatedLink();
        LOG_DEBUG("########postHandleNull result : %d, postHandleCreatedLink result : %d########", count1, count2);
        if(count1 == 0 && count2 == 0){
            loopFlag = false;
        }
    }while(loopFlag);
    
    //postReduceWidthOfLinkChians();
    //postReduceHeightOfLinkChians();

    //画规则的关系图
    if(m_debug){
        printDiagramOfRules("./"+getGrammarName()+".dot");
    }

    //把所有的规则转换为运行时数据
    transToRunningState();
    return true;
}

int RuleMgr::queryWeight(SymbolType sType){
    const std::string& ruleName = queryNameBySymbolType(sType);
    if (m_nomeanRuleNames.count(ruleName))
    {
        return 1;
    }
    return 10;
}

int RuleMgr::thresholdWeight(){
    return 1;
}

}
