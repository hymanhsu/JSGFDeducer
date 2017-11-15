#include <wfst/parser.h>
#include <util/printer.h>
#include <simple_log.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <functional>
#include <sstream>
#include <iterator>


namespace sogou
{

TableUnit::TableUnit() {}

TableUnit::~TableUnit()
{
    std::unordered_map<SymbolType, CopiedTagListPtr>::iterator it2;
    for (it2 = m_copiedTags.begin(); it2 != m_copiedTags.end(); ++it2)
    {
        std::unordered_set<std::pair<TUnitPtr,SymbolType>,pair_hash> *list1 = it2->second;
        delete list1;
    }
}

void TableUnit::addType(SymbolType type)
{
    m_matchedTypes.insert(type);
}

void TableUnit::addTag(SymbolType type, TagInfo *tag)
{
    m_tags.insert(std::make_pair(type, tag));
}

void TableUnit::copyTag(SymbolType type, TableUnit* sourceUnit, SymbolType sourceType){
    std::unordered_map<SymbolType, CopiedTagListPtr>::iterator it;
    it = m_copiedTags.find(type);
    if (it != m_copiedTags.end())
    {
        it->second->insert(std::make_pair(sourceUnit,sourceType));
    }
    else
    {
        std::unordered_set<std::pair<TUnitPtr,SymbolType>,pair_hash> *list1 = new std::unordered_set<std::pair<TUnitPtr,SymbolType>,pair_hash>();
        list1->insert(std::make_pair(sourceUnit,sourceType));
        m_copiedTags.insert(std::make_pair(type, list1));
    }
}

bool TableUnit::hasValidTag(SymbolType type){
    if(m_tags.count(type) || m_copiedTags.count(type)){
        return true;
    }
    return false;
}

WfstParser::WfstParser(RuleMgr *ruleMgr, SplitResult *splitResult, std::vector<MatchUnit> *dictResult, bool debug)
    : m_ruleMgr(ruleMgr), m_splitResult(splitResult), m_dictResult(dictResult), m_debug(debug)
{
    //创建WFST表格
    const SplitWord &lastItem = m_splitResult->result[m_splitResult->result.size() - 1];
    m_tableSize = lastItem.end + 1;
    m_table = new TUnitPtr *[m_tableSize];
    for (int i = 0; i < m_tableSize; ++i)
    {
        m_table[i] = new TUnitPtr[m_tableSize];
        for (int j = 0; j < m_tableSize; ++j)
        {
            m_table[i][j] = nullptr;
        }
    }
    //初始化染色板
    m_colorFlags = new int[m_tableSize-1];
    for (int i = 0; i < m_tableSize-1; ++i)
    {
        m_colorFlags[i] = 0;
    }
}

WfstParser::~WfstParser()
{
    //清理所有标签对象
    std::unordered_set<TagInfo *>::iterator it;
    for (it = m_tagInfos.begin(); it != m_tagInfos.end(); ++it)
    {
        TagInfo *tag = *it;
        delete tag;
    }
    //清理表格
    for (int i = 0; i < m_tableSize; ++i)
    {
        for (int j = 0; j < m_tableSize; ++j)
        {
            TableUnit *unit = m_table[i][j];
            if (unit != nullptr)
            {
                delete unit;
            }
        }
        delete[] m_table[i];
    }
    delete[] m_table;
    //清理染色板
    delete[] m_colorFlags;
}

void WfstParser::queryOffset(TableUnit* unit_, int& offsetX, int& offsetY){
    for (int i = 0; i < m_tableSize; ++i)
    {
        for (int j = 0; j < m_tableSize; ++j)
        {
            TableUnit *unit = m_table[i][j];
            if (unit == unit_)
            {
                offsetX = j;
                offsetY = i;
                return;
            }
        }
    }
}

TagInfo *WfstParser::generateTag(int offsetX_, int offsetY_, const std::string &info_)
{
    TagInfo *tag = new TagInfo(offsetX_, offsetY_, info_);
    m_tagInfos.insert(tag);
    return tag;
}

TableUnit *WfstParser::getUnit(int offsetX, int offsetY)
{
    return m_table[offsetY][offsetX];
}

TableUnit *WfstParser::accessUnit(int offsetX, int offsetY)
{
    TableUnit *unit = m_table[offsetY][offsetX];
    if (unit == nullptr)
    {
        unit = new TableUnit;
        m_table[offsetY][offsetX] = unit;
    }
    return unit;
}

TableUnit* WfstParser::fillUnit(SymbolType sType, int offsetX, int offsetY)
{
    if (!m_ruleMgr->isNillSymbol(sType))
    {
        TableUnit *unit = accessUnit(offsetX, offsetY);
        unit->addType(sType);
        const std::string* tagtext = m_ruleMgr->queryTag(sType);
        if (NULL != tagtext){  //如果包含tag，也把tag保存下来
            TagInfo *tag = generateTag(offsetX, offsetY, *tagtext);
            unit->addTag(sType, tag);
        }
        return unit;
    }
    return NULL;
}

void WfstParser::fillColor(int offsetX, int offsetY)
{
    int fillCount = offsetY - offsetX;
    for (int i = offsetX; i < offsetY; ++i)
    {
        m_colorFlags[i] = 1;
    }
}

bool WfstParser::isAllInputTransfed()
{
    for (int i = 0; i < m_tableSize-1; ++i)
    {
        if (m_colorFlags[i] == 0)
        {
            LOG_WARN("location %d is not filled",i);
            return false;
        }
    }
    return true;
}

/**
 * 查询词典匹配结果对应的表格坐标
**/
void WfstParser::queryOffset(MatchUnit& matchUnit, int& offsetX,int& offsetY){
    int distance = 0;
    offsetX = -1;
    offsetY = -1;
    std::vector<SplitWord>::iterator it;
    for (it = m_splitResult->result.begin(); it != m_splitResult->result.end(); ++it)
    {
        SplitWord &item = *it;
        if(item.offset == matchUnit.offset_){
            offsetX = item.start;
            distance += item.len;
            if(distance == matchUnit.count_){
                offsetY = item.end;
                break;
            }
        }else{
            if(offsetX != -1 && -1 == offsetY){
                if(distance + item.len < matchUnit.count_){
                    distance += item.len;
                }else if(distance + item.len == matchUnit.count_){
                    offsetY = item.end;
                    break;
                }
            }
        }
    }
}

bool WfstParser::copyTag(SymbolType sourceType, TableUnit *sourceUnit, SymbolType targetType, TableUnit *targetUnit){
    if(sourceType == targetType && sourceUnit == targetUnit){
        LOG_WARN("copyTag : cannot copy itself ");
        return false;
    }
    if(sourceUnit->hasValidTag(sourceType)){
        targetUnit->copyTag(targetType, sourceUnit, sourceType);
    }
    /*std::unordered_map<SymbolType, TagListPtr>::iterator it = sourceUnit->m_tags.find(sourceType);
    if (it != sourceUnit->m_tags.end()){ //有需要拷贝的tag
        std::unordered_set<TagInfo*> * tagListPtr = it->second;
        std::unordered_set<TagInfo*>::iterator tagIter = tagListPtr->begin();
        for(;tagIter!=tagListPtr->end();++tagIter){
            TagInfo* tag = *tagIter;
            targetUnit->addTag(targetType, tag);
        }
    }*/
    return true;
}

/**
 * 处理单个规则推导出另一个单个规则的情况，这里需要对所有的简单规则都推导到无法推导为止
 **/
int WfstParser::deduceLink(int offsetPos1, int offsetPos2, SymbolType symbolType, TableUnit * unit){
    int count = 0;
    const SymbolTypeSet& peerTargetSymbolTypeSet = m_ruleMgr->queryTargetLink(symbolType);
    if(!m_ruleMgr->isNillSymbolSet(peerTargetSymbolTypeSet)){ //有向上的连接存在
        SymbolTypeSet::const_iterator peerTargetSymbolTypeIter = peerTargetSymbolTypeSet.cbegin();
        for(;peerTargetSymbolTypeIter!=peerTargetSymbolTypeSet.cend();++peerTargetSymbolTypeIter){
            const SymbolType& peerTargetSymbolType = *peerTargetSymbolTypeIter;
            if(m_debug){
                LOG_DEBUG("deduceLink at %d,%d [%s] => [%s]",
                    offsetPos1,offsetPos2,
                    m_ruleMgr->queryNameBySymbolType(symbolType).c_str(),
                    m_ruleMgr->queryNameBySymbolType(peerTargetSymbolType).c_str());
            }
            fillUnit(peerTargetSymbolType, offsetPos1, offsetPos2);
            ++count;
            copyTag(symbolType,unit,peerTargetSymbolType,unit);
            count += deduceLink(offsetPos1,offsetPos2,peerTargetSymbolType,unit);
        }
    }
    return count;
}

bool WfstParser::deduce(int offsetPos1, int offsetPos2, int k){
    TableUnit * unit1 = getUnit(offsetPos1,k);
    TableUnit * unit2 = getUnit(k,offsetPos2);
    if(nullptr != unit1 && nullptr != unit2){  //这两部分都可能被匹配过
        std::unordered_set<SymbolType>::iterator leftIter  = unit1->m_matchedTypes.begin();
        for(;leftIter!=unit1->m_matchedTypes.end();++leftIter){
            std::unordered_set<SymbolType>::iterator rightIter = unit2->m_matchedTypes.begin();
            for(;rightIter!=unit2->m_matchedTypes.end();++rightIter){
                SymbolType leftSymbolType = *leftIter;
                SymbolType rightSymbolType = *rightIter;
                const SymbolTypeSet& targetSymbolTypeSet = m_ruleMgr->queryTargetTypeByPair(leftSymbolType,rightSymbolType);
                if(!m_ruleMgr->isNillSymbolSet(targetSymbolTypeSet)){ //A=BC 推导可以成立
                    SymbolTypeSet::const_iterator targetSymbolTypeIter = targetSymbolTypeSet.cbegin();
                    for(;targetSymbolTypeIter!=targetSymbolTypeSet.cend();++targetSymbolTypeIter){ //遍历所有的A
                        const SymbolType& targetSymbolType = *targetSymbolTypeIter;
                        if(m_debug){
                            LOG_DEBUG("deduce at %d,%d [%s] %d,%d [%s] => %d,%d [%s]",
                            offsetPos1,k,
                            m_ruleMgr->queryNameBySymbolType(leftSymbolType).c_str(),
                            k,offsetPos2,
                            m_ruleMgr->queryNameBySymbolType(rightSymbolType).c_str(),
                            offsetPos1,offsetPos2,
                            m_ruleMgr->queryNameBySymbolType(targetSymbolType).c_str());
                        }
                        TableUnit * unitTarget = fillUnit(targetSymbolType, offsetPos1,offsetPos2); 
                        copyTag(leftSymbolType,unit1,targetSymbolType,unitTarget);
                        copyTag(rightSymbolType,unit2,targetSymbolType,unitTarget);
                        deduceLink(offsetPos1,offsetPos2,targetSymbolType,unitTarget);  //处理向上的连接
                    }
                }
            }
        }
    }
    return true;
}

/**
 * 采用WFST表格，自底向上进行推导
**/
bool WfstParser::deduce()
{
    LOG_DEBUG("Query : [%s]",m_splitResult->rawQuery.c_str());
    if(m_splitResult->result.size()==0){
        return false;
    }
    int denominator = 0;
    double numerator = 0;
    //1,处理第一层，把基本的字符转换为类型SymbolType
    std::string wildcardRuleName = m_ruleMgr->fullRuleName("<WILDCARD>");
    SymbolType wildcardSType = m_ruleMgr->transStringToInt(wildcardRuleName);
    std::vector<SplitWord>::iterator it;
    for (it = m_splitResult->result.begin(); it != m_splitResult->result.end(); ++it)
    {
        SplitWord &item = *it;
        if(m_ruleMgr->isWildcard()){
            if(m_debug){
                LOG_DEBUG("fillUnit [%s][%s] at [%d,%d]",wildcardRuleName.c_str(),item.word.c_str(), item.start, item.end);
            }
            TableUnit* unit = fillUnit(wildcardSType, item.start, item.end);
            deduceLink(item.start, item.end, wildcardSType, unit);
            fillColor(item.start, item.end);
        }
        const SymbolTypeSet& symbolTypeSet = m_ruleMgr->queryTerminalTypeByValue(item.word);
        if(m_ruleMgr->isNillSymbolSet(symbolTypeSet)){
            if(m_debug){
                LOG_WARN("word not be located : %s",item.word.c_str());
            }
            continue;
        }
        SymbolTypeSet::const_iterator sTypeIter = symbolTypeSet.cbegin();
        for(;sTypeIter!=symbolTypeSet.cend();++sTypeIter){
            const SymbolType& sType = *sTypeIter;
            if(m_debug){
                LOG_DEBUG("fillUnit [%s][%s] at [%d,%d]",m_ruleMgr->queryNameBySymbolType(sType).c_str(),item.word.c_str(),item.start, item.end);
            }
            TableUnit* unit = fillUnit(sType, item.start, item.end);
            int count = deduceLink(item.start, item.end, sType, unit);
            if(m_debug){
                LOG_DEBUG("expand up link %d at [%d,%d]",count,item.start, item.end);
            }
            fillColor(item.start, item.end);
            //累加权重
            numerator += m_ruleMgr->queryWeight(sType) * (item.end-item.start);
            denominator += item.end-item.start;
        }
    }
    LOG_DEBUG("Word transform done!");
    //使用词典进行转换
    std::vector<MatchUnit>::iterator it2;
    for (it2 = m_dictResult->begin(); it2 != m_dictResult->end(); ++it2)
    {
        MatchUnit &item = *it2;
        if(!m_ruleMgr->isDictTagUsed(item.tag_)){
            if(m_debug){
                LOG_WARN("dict result not be used : %s, %s, offset=%d,count=%d",item.text_.c_str(), item.tag_.c_str(),item.offset_, item.count_);
            }
            continue;
        }
        int offsetX = -1, offsetY = -1;
        queryOffset(item,offsetX,offsetY);  //得把词典的匹配直接对应到格子坐标上
        if(-1 == offsetX || -1 == offsetY){
            if(m_debug){
                LOG_WARN("dict result not be located : %s, %s, offset=%d,count=%d",item.text_.c_str(), item.tag_.c_str(), item.offset_, item.count_);
            }
            continue;
        }
        TableUnit* unit = accessUnit(offsetX, offsetY);
        std::string ruleName = m_ruleMgr->fullRuleName(item.tag_);
        SymbolType sType = m_ruleMgr->transStringToInt(ruleName);
        //m_typeToName.insert(std::make_pair(sType,ruleName));
        if(m_debug){
            LOG_DEBUG("fillUnit [%s][%s] at [%d,%d]",ruleName.c_str(), item.text_.c_str(), offsetX, offsetY);
        }
        fillUnit(sType, offsetX, offsetY);
        deduceLink(offsetX, offsetY, sType, unit);
        fillColor(offsetX, offsetY);
        //累加权重
        numerator += m_ruleMgr->queryWeight(sType) * (offsetY-offsetX);
        denominator += offsetY-offsetX;
    }
    LOG_DEBUG("Dict match transform done!");
    if (!isAllInputTransfed())  //检查是否所有的输入串所在的格子都被转换过了，如果没有转换完，可以认为是证据不全，直接返回失败
    {
        LOG_ERROR("Not completely covered at Length 1");
        return false;
    }
    double totalWeight = numerator/denominator;
    int zoomSize = 1000000;
    if(m_debug){
        LOG_DEBUG("Total weight (%f)",totalWeight);
    }
    if( m_splitResult->result.size()>3 && int(totalWeight*zoomSize) <= int(m_ruleMgr->thresholdWeight()*zoomSize) ){ //如果大于3个单词以上且总意义权重低于阈值，则放弃继续处理
        if(m_debug){
            LOG_WARN("Total weight (%f) is lower than ThresholdWeight (%d) ",totalWeight,m_ruleMgr->thresholdWeight());
        }
        return false;
    }
    if(m_debug){
        displayTable(1);
    }
    //2,处理其他层
    for(int length=2; length<m_tableSize; ++length){  //定义匹配宽度
        int offsetPos1 = 0, offsetPos2 = 0;
        for( offsetPos1 = 0, offsetPos2 = offsetPos1 + length ; 
            offsetPos1 < m_tableSize && offsetPos2 < m_tableSize; 
            ++offsetPos1, offsetPos2 = offsetPos1 + length){  //定义符合匹配宽度的匹配范围
            for(int k = offsetPos1 + 1 ; k < offsetPos2; ++k ){ //在一个匹配范围内设定k分割为两部分
                deduce(offsetPos1,offsetPos2,k);
            }
        }
        if(m_debug){
            displayTable(length);
        }
    }
    //3,判断推导是否完成
    TableUnit * finalUnit = getUnit(0, m_tableSize-1);
    if(nullptr == finalUnit){
        LOG_ERROR("Not found deduce result");
        return false;
    }
    std::string mainRuleName = m_ruleMgr->fullRuleName("<main>");
    SymbolType mainRuleType = m_ruleMgr->transStringToInt(mainRuleName);
    bool foundMainRule = false;
    std::unordered_set<SymbolType>::iterator typeIter = finalUnit->m_matchedTypes.begin();
    for(;typeIter!=finalUnit->m_matchedTypes.end();++typeIter){
        SymbolType sType = *typeIter;
        if(sType == mainRuleType){
            foundMainRule = true;
        }
    }
    if(!foundMainRule){
        LOG_ERROR("Not deduce to <main>, deduce do not finish");
        return false;
    }
    return true;
}


std::string WfstParser::querySource(int offsetX,int offsetY){
    std::ostringstream slotValue; 
    std::vector<SplitWord>::iterator it;
    bool foundStart = false;
    for (it = m_splitResult->result.begin(); it != m_splitResult->result.end(); ++it)
    {
        SplitWord &item = *it;
        if(!foundStart && item.start == offsetX){
            foundStart = true;
            slotValue << item.word;
        }else if(foundStart && item.start < offsetY){
            slotValue << item.word;
        }
    }
    return slotValue.str();
}

void WfstParser::extractTags(SymbolType sType, TableUnit *finalUnit, DeduceResult& deduceResult){
    //处理自身tag
    std::unordered_map<SymbolType, TagInfoPtr>::iterator tagFoundIter = finalUnit->m_tags.find(sType);
    if(tagFoundIter!=finalUnit->m_tags.end()){ //发现了sType类型对应的tag
        TagInfo* tag = tagFoundIter->second;
        std::string tagInfo = tag->getInfo();
        std::vector<std::string> tokens;
        Splitter::Split(tagInfo,".",tokens);
        if(tokens.size() == 1){  //例如{musicName}
            std::string slotName = tokens[0];
            slotName = slotName.substr(1,slotName.size()-2);
            std::string slotValue = querySource(tag->getOffsetX(),tag->getOffsetY());
            deduceResult.slots.insert(std::make_pair(slotName,slotValue));
        }else if(tokens.size() == 2){ //例如{Intent.music}
            std::string slotName = tokens[0];
            slotName = slotName.substr(1,slotName.size()-1);
            std::string slotValue = tokens[1];
            slotValue = slotValue.substr(0,slotValue.size()-1);
            deduceResult.slots.insert(std::make_pair(slotName,slotValue));
        }else{  //为了支持不太符合tag规范的格式，例如{musician.attr1.attr2}
            std::string slotName = tokens[0];
            slotName = slotName.substr(1,slotName.size()-1);
            std::stringstream slotValueStream;
            for(int i=1; i<tokens.size(); ++i){
                slotValueStream << tokens[i];
            }
            std::string slotValue = slotValueStream.str();
            slotValue = slotValue.substr(0,slotValue.size()-1);
            deduceResult.slots.insert(std::make_pair(slotName,slotValue));
        }
    }
    //处理拷贝过来的tag
    std::unordered_map<SymbolType, CopiedTagListPtr>::iterator copyTagIter = finalUnit->m_copiedTags.find(sType);
    if(copyTagIter!=finalUnit->m_copiedTags.end()){
        std::unordered_set<std::pair<TUnitPtr,SymbolType>,pair_hash> * listPtr = copyTagIter->second;
        std::unordered_set<std::pair<TUnitPtr,SymbolType>,pair_hash>::iterator tagIter = listPtr->begin();
        for(;tagIter!=listPtr->end();++tagIter){
            const std::pair<TUnitPtr,SymbolType> & tagPair = *tagIter;
            TableUnit * downPeerUnit = tagPair.first;
            SymbolType  downPeerType = tagPair.second;
            extractTags(downPeerType,downPeerUnit,deduceResult);
        }
    }
}

void WfstParser::analyzeDeduceResult(std::vector<DeduceResult>& deduceResults, bool format){
    std::map<std::string,DeduceResult> resultMap;
    //把语法名作为domainName
    std::string domainName = m_ruleMgr->getGrammarName();
    //遍历所有单元格，寻找命中的规则名,包含'.rule_'或'.pattern_'
    for (int i = 0; i < m_tableSize; ++i)
    {
        for (int j = 0; j < m_tableSize; ++j)
        {
            TableUnit *unit = m_table[i][j];
            if(nullptr!=unit){
                std::unordered_set<SymbolType>::iterator typeIter = unit->m_matchedTypes.begin();
                for(;typeIter!=unit->m_matchedTypes.end();++typeIter){
                    SymbolType sType = *typeIter;
                    std::string ruleName = m_ruleMgr->queryNameBySymbolType(sType); 
                    std::size_t foundPos1 = ruleName.find(".rule_");
                    std::size_t foundPos2 = ruleName.find(".pattern_");
                    if(foundPos2==std::string::npos && foundPos1==std::string::npos){
                        continue;
                    }
                    LOG_DEBUG("analyzeDeduceResult %s at [%d,%d]", ruleName.c_str(), i, j);
                    //遇到一个命中的pattern
                    DeduceResult deduceResult;
                    deduceResult.slots.insert(std::make_pair("Rule",ruleName));
                    deduceResult.slots.insert(std::make_pair("Domain",domainName));
                    std::stringstream ss;
                    ss << j << "," << i;
                    deduceResult.slots.insert(std::make_pair("Pos",ss.str()) );
                    extractTags(sType,unit,deduceResult);
                    std::map<std::string,DeduceResult>::iterator foundIter = resultMap.find(ruleName);
                    if(foundIter!=resultMap.end()){
                        DeduceResult & existedResult = foundIter->second;
                        if(deduceResult.slots.size() > existedResult.slots.size()){
                            resultMap.erase(ruleName);
                            std::pair<std::map<std::string,DeduceResult>::iterator,bool> ret;
                            ret = resultMap.insert(std::make_pair(ruleName, deduceResult) );
                            if(ret.second){
                                if(m_debug){
                                    LOG_DEBUG("replace success : %s [%d,%d]", ruleName.c_str(), i, j);
                                }
                            }else{
                                if(m_debug){
                                    LOG_DEBUG("replace failed : %s [%d,%d]", ruleName.c_str(), i, j);
                                }
                            }
                        }
                    }else{
                        resultMap.insert(std::make_pair(ruleName, deduceResult) );
                        if(m_debug){
                            LOG_DEBUG("insert success : %s [%d,%d]", ruleName.c_str(), i, j);
                        }
                    }
                }
            }
        }
    }
    std::map<std::string,DeduceResult>::iterator viewIter = resultMap.begin();
    for(;viewIter!=resultMap.end();++viewIter){
        deduceResults.push_back(viewIter->second);
    }
}

// std::string WfstParser::queryNameBySymbolType(SymbolType type){
//     std::unordered_map<SymbolType, std::string>::iterator it;
//     it = m_typeToName.find(type);
//     if(it != m_typeToName.end()){
//         return it->second;
//     }
//     return "";
// }

void WfstParser::displayTableUnit(int i,int j){
    TableUnit *unit = m_table[i][j];
    LOG_DEBUG("=============POS:%d,%d rules:%d taglists:%d=============",i,j, 
        (nullptr!=unit)?unit->m_matchedTypes.size():0,
        (nullptr!=unit)?unit->m_tags.size():0 );
    if(nullptr!=unit){
        std::unordered_set<SymbolType>::iterator typeIter = unit->m_matchedTypes.begin();
        for(;typeIter!=unit->m_matchedTypes.end();++typeIter){
            SymbolType sType = *typeIter;
            const std::string& ruleName = m_ruleMgr->queryNameBySymbolType(sType);  //命中的规则名
            // if(ruleName == ""){
            //     ruleName = queryNameBySymbolType(sType); 
            // }
            std::unordered_map<SymbolType, CopiedTagListPtr>::iterator copyTagFoundIter = unit->m_copiedTags.find(sType);
            LOG_DEBUG("    rule : %s, tags:%d",ruleName.c_str(), unit->m_tags.count(sType),
                (copyTagFoundIter!=unit->m_copiedTags.end())?copyTagFoundIter->second->size():0
            );
            std::unordered_map<SymbolType, TagInfoPtr>::iterator tagFoundIter = unit->m_tags.find(sType);
            if(tagFoundIter!=unit->m_tags.end()){
                LOG_DEBUG("        tag : %s", tagFoundIter->second->getInfo().c_str() );
            }
            if(copyTagFoundIter!=unit->m_copiedTags.end()){
                std::unordered_set<std::pair<TUnitPtr,SymbolType>,pair_hash> * setPtr = copyTagFoundIter->second;
                std::unordered_set<std::pair<TUnitPtr,SymbolType>,pair_hash>::iterator copyTagIter = setPtr->begin();
                for(;copyTagIter!=setPtr->end();++copyTagIter){
                    const std::pair<TUnitPtr,SymbolType> & tagPair = *copyTagIter;
                    int offsetX, offsetY;
                    queryOffset(tagPair.first,offsetX,offsetY);
                    LOG_DEBUG("        copytag : %s, %d,%d", m_ruleMgr->queryNameBySymbolType(tagPair.second).c_str(), offsetY, offsetX );
                }
            }
        }
    }
    LOG_DEBUG("===================================");
}

void WfstParser::displayTable(int level){
    LOG_DEBUG("==============================WFST Table, level:%d==============================",level);
    TTable table;
    for (int i = 0; i < m_tableSize; ++i)
    {
        sogou::TRow row;
        for (int j = 0; j < m_tableSize; ++j)
        {
            //displayTableUnit(i,j);
            TableUnit *unit = m_table[i][j];
            if(nullptr==unit){
                row.push_back(TCell());
            }else{
                TCell cell;
                cell.push_back(querySource(j,i));
                std::unordered_set<SymbolType>::iterator typeIter = unit->m_matchedTypes.begin();
                for(;typeIter!=unit->m_matchedTypes.end();++typeIter){
                    SymbolType sType = *typeIter;
                    const std::string& ruleName = m_ruleMgr->queryNameBySymbolType(sType);  //命中的规则名
                    // if(ruleName == ""){
                    //     ruleName = queryNameBySymbolType(sType); 
                    // }
                    cell.push_back(ruleName);
                }
                row.push_back(cell);
            }
        }
        table.push_back(row);
    }
    std::stringstream strout;
    Printer::printTable(table,strout,30);
    LOG_DEBUG("\n%s",strout.str().c_str());
    LOG_DEBUG("======================================================================");
}


}
