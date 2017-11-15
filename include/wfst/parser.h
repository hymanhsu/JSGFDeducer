/*************************************************  
Author: Kevin.XU
Date:   2017-10-26
Desc:   解析
**************************************************/ 

#ifndef PARSER_H
#define PARSER_H

#include <wfst/rule.h>
#include <util/splitter.h>
#include <dict/matcher.h>

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>


namespace sogou
{


//标签有两种创建情况，1，本身规则命中，2，推导来源规则的标签拷贝过来的
class TagInfo{
public:
    TagInfo(int offsetX_,int offsetY_,const std::string& info_)
        :offsetX(offsetX_),offsetY(offsetY_),info(info_){
    }
    ~TagInfo(){}
    int getOffsetX() const {
        return offsetX;
    }
    int getOffsetY() const {
        return offsetY;
    }
    const std::string& getInfo() const{
        return info;
    }
private:
    /***规则被命中后，该规则在表格中所在的坐标**/
    int offsetX;  //x
    int offsetY;  //y
    std::string info;  //标签内容, 例如{Intent.music}
};
typedef TagInfo* TagInfoPtr;

class TableUnit;
typedef TableUnit* TUnitPtr;

typedef std::unordered_set<TagInfo*> *TagListPtr;
typedef std::unordered_set<std::pair<TUnitPtr,SymbolType>,pair_hash> *CopiedTagListPtr;

class TableUnit{
public:
    TableUnit();
    ~TableUnit();
    /**
     * 加一个类型
     **/
    void addType(SymbolType type);
    /**
     * 为类型加一个tag
     **/
    void addTag(SymbolType type, TagInfo* tag);
    /**
     * 把源格子的一个类型的tag拷贝过来
     * 实际上是建立一个树，用于最后取结果用
     **/
    void copyTag(SymbolType type, TableUnit* sourceUnit, SymbolType sourceType);
    /**
     * 判断某一个类型是否包含tag(包括本格子内部的和从其他格子继承过来的)
     **/
    bool hasValidTag(SymbolType type);
public:
    std::unordered_set<SymbolType>                    m_matchedTypes; //匹配上的类型集合
    std::unordered_map<SymbolType, TagInfoPtr>        m_tags;         //每个类型都可能带一个自身标签对象
    std::unordered_map<SymbolType, CopiedTagListPtr>  m_copiedTags;   //通过拷贝传递过来标签来源,每个类型都可能带多个标签对象
};


class DeduceResult{
public:
    std::unordered_set<std::pair<std::string,std::string>,pair_hash> slots; //包括各种Slots，如：Domain,Intent,etc
};


class WfstParser{
public:
    WfstParser(RuleMgr* ruleMgr, SplitResult *splitResult, std::vector<MatchUnit> *dictResult, bool debug = false);
    ~WfstParser();
    /**
     * 访问表格某个位置的元素，仅仅访问不会主动创建
     **/
    TableUnit* getUnit(int offsetX,int offsetY);
    /**
     * 访问表格某个位置的元素，如果没有就创建
     **/
    TableUnit* accessUnit(int offsetX,int offsetY);
    /**
     * 采用WFST表格，自底向上进行推导
     **/
    bool deduce();
    /**
     * 分析推导结果
     **/
    void analyzeDeduceResult(std::vector<DeduceResult>& deduceResults, bool format = true);
    /**
     * 从坐标查询原串
     **/
    std::string querySource(int offsetX,int offsetY);
    /**
     * 打印WFST表格
     **/
    void displayTable(int level);
    /**
     * 打印WFST表格的单元格
     **/
    void displayTableUnit(int i,int j);
    /**
     * 根据类型返回规则原名
     **/
    //std::string queryNameBySymbolType(SymbolType type);
protected:
    /**
     * 基于一个坐标产生一个新的tag对象
     **/
    TagInfo* generateTag(int offsetX_,int offsetY_,const std::string& info_);
    /**
     * 在一个坐标上填充一个类型
     **/
    TableUnit* fillUnit(SymbolType sType, int offsetX, int offsetY);
    /**
     * 查询单元格对应的坐标
     **/
    void queryOffset(TableUnit* unit, int& offsetX, int& offsetY);
    /**
     * 查询词典匹配结果对应的表格坐标
     **/
    void queryOffset(MatchUnit& matchUnit, int& offsetX,int& offsetY);
    /**
     * 判断是否输入所对应的格子已经被全部覆盖
     **/
    bool isAllInputTransfed();
    /**
     * 给坐标对应的格子进行染色
     **/
    void fillColor(int offsetX,int offsetY);
    /**
     * 推导两个位置之间的格子
     * 将对Rule{offsetPos1,offsetPos2}的匹配，转为 Rule1{offsetPos1,k} Rule2{k,offsetPos2}的匹配
     * Rule1 Rule2 => Rule
     **/
    bool deduce(int offsetPos1, int offsetPos2, int k);
    /**
     * 推导某一个格子上的某一个类型的向上连接
     **/
    int deduceLink(int offsetPos1, int offsetPos2, SymbolType symbolType,TableUnit * unit);
    /**
     * 把源类型的关联tag拷贝过来
     **/
    bool copyTag(SymbolType sourceType, TableUnit *sourceUnit, SymbolType targetType, TableUnit *targetUnit);
    /**
     * 萃取某一个格子中的某种类型的关联tag
     **/
    void extractTags(SymbolType sType, TableUnit *unit, DeduceResult& deduceResult);
private:
    bool                             m_debug;       //是否调试模式
    RuleMgr*                         m_ruleMgr;     //规则管理器
    SplitResult*                      m_splitResult; //输入
    std::vector<MatchUnit>*           m_dictResult;  //词典匹配结果
    int                              m_tableSize;   //表格行数/列数
    int*                             m_colorFlags;  //填充染色板
    TUnitPtr**                       m_table;       //WFST表格
    std::unordered_set<TagInfo*>     m_tagInfos;    //标签对象
    //std::unordered_map<SymbolType, std::string>  m_typeToName; //处理过程中产生的type->name
};


}

#endif

