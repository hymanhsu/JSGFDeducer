#include <dict/matcher.h>
#include <util/splitter.h>
#include <util/timer.h>
#include <simple_log.h>

#include <dirent.h>
#include <sys/stat.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <functional>

namespace sogou
{


RegexMatcher::RegexMatcher()
{
    std::string re_number = "(((\\d|一|二|三|四|五|六|七|八|九|十|零)(百|千|万|亿|幺)?)+";
    re_number += "(点((\\d|一|二|三|四|五|六|七|八|九|十|零))+)?)";
    std::string re_number1("((\\d|一|二|三|四|五|六|七|八|九|十|零|百|千|万|[A-Za-z]|甲|乙|丙|-)+)");

    // string re_poikeyword = "";
    // re_poikeyword = "(与(.+)交叉口)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "(" + re_number + "(县道|乡道|村道|省道|国道|站台))";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "((西北|西南|东北|东南|东|南|西|北)?[A-Za-z0-9]*(出入|出|入)口)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "((西北|西南|东北|东南|东|南|西|北)[A-Za-z0-9]*口)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // /////////
    // re_poikeyword = "((西北|西南|东北|东南|东|南|西|北)"+ re_number + "米)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // /////////
    // re_poikeyword = "((中|东北|西北|东南|西南|东|南|西|北)" + re_number + "?门)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "((第)?(东北|西北|东南|西南|东|南|西|北)?[A-Za-z0-9-]+(号)?门)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "((东|南|西|北|中|正)?(后)?(小|大)(东|南|西|北)?门)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "((东|南|西|北)?(正|后|边|宫后)门)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // /////////
    // re_poikeyword = "(" + re_number1 + "(号|栋|幢|临|座)(楼|院)(东|南|西|北|西南|东南|西北|东北)?)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "(" + re_number + "弄)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "(" + re_number + "区)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "(" + re_number1 + "(号|栋|幢|临|座)(楼)?)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "(" + re_number + "单元)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "(" + re_number + "层(楼)?)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "(" + re_number + "室)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    // re_poikeyword = "((第)?(东|南|西|北)[A-Za-z0-9]+(区|期)([A-Za-z0-9-]+号)?)";
    // regs_.push_back(make_pair<string, string>("REG.POI_KEYWORD", "(.*?)" + re_poikeyword + "(.*?)") );
    ////////
    /////////
    //define <REG.NUMBER>
    std::string re_number2 = "((两(百|千|万|亿))(\\d|一|二|三|四|五|六|七|八|九|十|零)*";
    re_number2 += "((点|.)(\\d|一|二|三|四|五|六|七|八|九|十|零)+)?)";
    std::string re_number3 = "((\\d|一|二|三|四|五|六|七|八|九|十|零|百|千|万|亿|两)+";
    re_number3 += "((点|.)(\\d|一|二|三|四|五|六|七|八|九|十|零)+)?)";
    regs_.push_back(make_tuple("REG.NUMBER", "re_number2", "(.*?)" + re_number2 + "(.*?)"));
    regs_.push_back(make_tuple("REG.NUMBER", "re_number3", "(.*?)" + re_number3 + "(.*?)"));
    regs_.push_back(make_tuple("REG.NUMBER", "re_number", "(.*?)" + re_number + "(.*?)"));

    //    regs_.push_back(make_tuple("REG.NUMBER", "re_number_total", "(.*?)("
    //        + re_number + "|"
    //        + re_number2 + "|"
    //        + re_number3 + ")(.*?)"));

    //define <REG.LETTER>
    std::string re_letter = "([A-Za-z]+)";
    regs_.push_back(make_tuple("REG.LETTER", "re_letter", "(.*?)" + re_letter + "(.*?)"));

    //To improve the performance, put data and time to regex dict
    //Modified by Kevin.XU @ 2017.9.30
    //define <REG.DATE>
    std::string str_num_all = "(两|一|二|三|四|五|六|七|八|九|十|零|壹|贰|叁|肆|伍|陆|柒|捌|玖|拾|1|2|3|4|5|6|7|8|9|0|";
    str_num_all += "廿一|廿二|廿三|廿四|廿五|廿六|廿七|廿八|廿九|卅)";
    std::string flagWords_ymd = "年|月份|月|日|号|hao|nian|yue|ri|NIAN|YUE|RI|HAO";
    std::string flagWords_stop = ",| |＼|／|\\\\|-|/|\\||。|，|、|｜|－|．|初|闰";
    std::string all_flagWord = flagWords_ymd + "|" + flagWords_stop;
    std::string date_pattern_only_num = "(阳历|公历|农历|阴历|旧历|新历|闰|)?(" + str_num_all + "{4,})((" + flagWords_stop + ")*(" + str_num_all + "{1,}))?((" + flagWords_stop + ")*(" + str_num_all + "{1,}))?";
    std::string date_pattern_only_stop = "(阳历|公历|农历|阴历|旧历|新历|闰|)?(" + str_num_all + "+)((" + all_flagWord + ")+)((阳历|公历|农历|阴历|旧历|新历|闰|)?(" + str_num_all + "+)((" + all_flagWord + ")*))?((" + str_num_all + "+)((" + all_flagWord + ")*))?";
    std::string pattern_month_cut = "(阳历|公历|农历|阴历|旧历|新历|闰|)?((一|二|三|四|五|六|七|八|九|零|壹|贰|叁|肆|伍|陆|柒|捌|玖|1|2|3|4|5|6|7|8|9|0)*)(十二|十一|一|二|三|四|五|六|七|八|九|十|11|12|10|1|2|3|4|5|6|7|8|9|0)((" + flagWords_stop + ")*)(月|YUE|yue)((" + str_num_all + "+)((" + all_flagWord + ")*))?";
    std::string datePattern = "(^([0-9]{2,4}(-|/|_| )(0?[0-9]|1[0-2])(-|/|_| )0)$)|(^(([0-9]{2,4}(-|/|_| )((0?[1-9]|1[0-2])(-|/|_| )?(0?[1-9]|1[0-9]|2[0-8])?|(0?[13-9]|1[0-2])(-|/|_| )?(29|30)?|(0?[13578]|1[02])(-|/|_| )?(31)?))|([0-9]{2}(0[48]|[2468][048]|[13579][26])|(0[48]|[2468][048]|[13579][26])00)(-|/|_| )0?2(-|/|_| )?(29)?)$)";
    std::string dateWordsPattern = "(阳历|公历|农历|阴历|旧历|新历|闰|)?(((" + str_num_all + "+)|(今年|明年|去年|后年|前年|大后年|大前年))(年|nian|NIAN)?)?(阳历|公历|农历|阴历|旧历|新历|闰|)?(祭灶节|下元节|祭祖节|地藏节|中元节|中秋节|端午节|仲秋节|元宵节|腊八节|正月|冬月|腊月|中秋|元宵|端午|仲秋|腊八|小年|龙抬头|七夕|乞巧节|重阳节|重阳|老人节|祭祖节|春节|新年|大年初一|除夕|过年|大年三十)((初)?(" + str_num_all + "+))?";
    std::string gongliDateWordsPattern = "(阳历|公历|农历|阴历|旧历|新历|闰|)?(((" + str_num_all + "+)|(今年|明年|去年|后年|前年|大后年|大前年))(年|nian|NIAN)?)?(阳历|公历|农历|阴历|旧历|新历|闰|)?((光棍节|圣诞节|平安夜|万圣节|盲人节|十一节|国庆节|国庆|教师节|八一节|建军节|七一节|建党节|六一节|儿童节|护士节|五四节|青年节|五一节|劳动节|愚人节|植树节|三八节|妇女节|情人节|元旦|五一|六一|七一|八一|十一|清明节|清明)|(母亲节|父亲节|感恩节))";
    std::string dateSysYearWordsPattern = "(阳历|公历|农历|阴历|旧历|新历|闰|)?(今年|明年|去年|后年|前年|大后年|大前年)((" + all_flagWord + ")*)((阳历|公历|农历|阴历|旧历|新历|闰|)?(" + str_num_all + "+)((" + all_flagWord + ")*))?((" + str_num_all + "+)((" + all_flagWord + ")*))?";
    std::string dateSysDayWordsPattern = "(今儿个|昨儿个|后儿个|前儿个|前儿|今个|昨个|明个|今日|今天|今晚|今夜|今早|今晨|明晨|明天|明早|明晚|明夜|明日|大大后天|大后天|后日|后天|昨天|昨日|昨儿|大大前天|大前天|前日|前天|今儿|明儿|后儿)";
    std::string date_only_num_for_xingzuo = "(阳历|公历|农历|阴历|旧历|新历|闰|)?(" + str_num_all + "+)";
    std::string cycPatternStr = "((每(一个|一|1个|1|个|)月(的|))(" + str_num_all + "+)(号|日))|((每(一个|一|1个|1|个|)周(的|))(一|二|三|四|五|六|1|2|3|4|日|5|6))|(每(一|1|)(天|日))|(每(一|1|)年)|((每(一个|一|个|)周末)|(((每(一个|一|个|))|)工作日))";
    std::string interzoneDatePatternStr = "(月(的|地|)(上|中|下)旬)|(((下|后)|(未来|将来|之后|往后|往下|往后数|往后算|明后|今后|后)|(前|上)|(过去|之前|往上|往前数|往前算|最近)|(本|这))(的|地|)((" + str_num_all + "+)|(半)|(几)|)(个|)((周|星期|礼拜)|(月)|(天|昼夜)))|((上|下|本|这|)(个|)周末)|((" + str_num_all + "+)((天)|(周|星期|礼拜))内)|(((未来|将来)|(最近))(" + str_num_all + "+)(到|－|至)(" + str_num_all + "+)((天)|(周|星期|礼拜)))|(过两天|过两日|过几天|过几日|几天后|几日后|过两三天|过两三日|近日|近期|近几天|近三个月|近三月|近几个月|近几月|近两三个月|近两三月|近两个月|近两月|近3个月|近3月|近2月|近2个月|近两三天|近两三日)";
    std::string weekDatePattern = "((((下)|(上)|这|本)(个|一个|1个|))|)(周|星期|礼拜)(一|二|三|四|五|六|1|2|3|4|日|5|6|天)";
    std::string monthDatrPattern = "(农历|阴历|旧历|公历|阳历|新历|)((下)|(上)|这|本)(个|一个|１个|)月(初|)(" + str_num_all + "+)(号|日|)";
    std::string numWMDatePattern = "((" + str_num_all + "+)|(半)|)(个|)((天|日)|(周|星期|礼拜)|(月))(之|以|)(前|后)";
    std::string month_week_st_Pattern = "月第(1|2|3|4|5|一|二|三|四|五)(个|)(星期|周|礼拜)(一|二|三|四|五|六|1|2|3|4|日|5|6)";
    std::string only_day_Pattern = "((" + str_num_all + "+)|今|去|明|前|后|)(年|)(农历|阴历|旧历|公历|阳历|新历|)(初|)(" + str_num_all + "+)(号|日|)";
    regs_.push_back(make_tuple("REG.DATE", "date_pattern_only_num", "(.*?)(" + date_pattern_only_num + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "date_pattern_only_stop", "(.*?)(" + date_pattern_only_stop + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "pattern_month_cut", "(.*?)(" + pattern_month_cut + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "datePattern", "(.*?)(" + datePattern + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "dateWordsPattern", "(.*?)(" + dateWordsPattern + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "gongliDateWordsPattern", "(.*?)(" + gongliDateWordsPattern + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "dateSysYearWordsPattern", "(.*?)(" + dateSysYearWordsPattern + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "dateSysDayWordsPattern", "(.*?)(" + dateSysDayWordsPattern + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "date_only_num_for_xingzuo", "(.*?)(" + date_only_num_for_xingzuo + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "cycPatternStr", "(.*?)(" + cycPatternStr + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "interzoneDatePatternStr", "(.*?)(" + interzoneDatePatternStr + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "weekDatePattern", "(.*?)(" + weekDatePattern + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "monthDatrPattern", "(.*?)(" + monthDatrPattern + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "numWMDatePattern", "(.*?)(" + numWMDatePattern + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "month_week_st_Pattern", "(.*?)(" + month_week_st_Pattern + ")(.*?)"));
    regs_.push_back(make_tuple("REG.DATE", "only_day_Pattern", "(.*?)(" + only_day_Pattern + ")(.*?)"));

    //    regs_.push_back(make_tuple("REG.DATE", "re_date_total", "(.*?)("
    //        + date_pattern_only_num + "|"
    //        + date_pattern_only_stop + "|"
    //        + pattern_month_cut + "|"
    //        + datePattern + "|"
    //        + dateWordsPattern + "|"
    //        + gongliDateWordsPattern + "|"
    //        + dateSysYearWordsPattern + "|"
    //        + dateSysDayWordsPattern + "|"
    //        + date_only_num_for_xingzuo + "|"
    //        + cycPatternStr + "|"
    //        + interzoneDatePatternStr + "|"
    //        + weekDatePattern + "|"
    //        + monthDatrPattern + "|"
    //        + numWMDatePattern + "|"
    //        + month_week_st_Pattern + "|"
    //        + only_day_Pattern + ")(.*?)"));

    //define <REG.TIME>
    std::string szCHNumRegex = "(零|一|二|三|四|五|六|七|八|九|十|百|千|仟|万){1,15}";
    std::string szTimeDescripRegex = "(现在|刚刚|当前|立刻|马上|凌晨|清晨|早晨|早上|上午|中午|下午|白天|晚上|夜晚|夜里|半夜|午夜|(今|明)?((早|晚)(上)?|夜(里)?)|AM|PM|am|pm)";
    std::string szStdHour24Regex = "((0)?(0|1|2|3|4|5|6|7|8|9)|1(0|1|2|3|4|5|6|7|8|9)|2(0|1|2|3))";
    std::string szStdMinSec24Regex = "(0|1|2|3|4|5)?(0|1|2|3|4|5|6|7|8|9)";
    std::string szCHStdHour24Regex = "(二十三|二十二|二十一|二十|十九|十八|十七|十六|十五|十四|十三|十二|十一|十|九|八|七|六|五|四|三|二|两|一|零)";
    std::string szCHStdMinSec24Regex = "(零)?((五十|四十|三十|二十|十)(一|二|三|四|五|六|七|八|九)|(五十|四十|三十|二十|十|九|八|七|六|五|四|三|二|两|一|零))";
    std::string szTimeDiffRegex = "(" + szTimeDescripRegex + "?(" + szCHStdHour24Regex + "|" + szStdHour24Regex + ")(点钟|点|时)差((" + szCHStdMinSec24Regex + "|" + szStdMinSec24Regex + ")(分钟|分)(" + szCHStdMinSec24Regex + "|" + szStdMinSec24Regex + ")(秒钟|秒)|(1|2|3|一|二|两|三)(刻钟|刻)|(" + szCHStdMinSec24Regex + "|" + szStdMinSec24Regex + ")(分钟|分|秒钟|秒)?))";
    std::string szTimeDiffRegex2 = "(差((" + szCHStdMinSec24Regex + "|" + szStdMinSec24Regex + ")(分钟|分)(" + szCHStdMinSec24Regex + "|" + szStdMinSec24Regex + ")(秒钟|秒)|(" + szCHStdMinSec24Regex + "|" + szStdMinSec24Regex + ")(分钟|分|秒钟|秒)|(1|2|3|一|二|两|三)(刻钟|刻))" + szTimeDescripRegex + "?(" + szCHStdHour24Regex + "|" + szStdHour24Regex + ")(点钟|点|时))";
    std::string szCalcTimeRangeRegex = "(((0|1|2|3|4|5|6|7|8|9)+|" + szCHNumRegex + "|两)(个)?(半)?(小时)((((零)?(0|1|2|3|4|5|6|7|8|9)+|" + szCHStdMinSec24Regex + "|半)(分钟|分|秒钟|秒))|(零)?(1|2|3|一|二|两|三)(刻钟|刻))?|半(个)?小时(((零)?(0|1|2|3|4|5|6|7|8|9)+|" + szCHStdMinSec24Regex + "|半)(分钟|分))?|((0|1|2|3|4|5|6|7|8|9)+|" + szCHStdMinSec24Regex + "|半)(分钟|分)(((零)?(0|1|2|3|4|5|6|7|8|9)+|" + szCHStdMinSec24Regex + ")(秒钟|秒))?|(((0|1|2|3|4|5|6|7|8|9)+(.(0|1|2|3|4|5|6|7|8|9)+)?|" + szCHNumRegex + "|两)(个半小时|个半钟头|个半分钟|个小时|小时|个钟头|钟头|分钟|分|秒钟|秒))|(一|二|两|三|1|2|3)(刻钟|刻))(之前|之后|以前|以后|前|后)";
    std::string szCalcTimePointRegex = "(((" + szTimeDescripRegex + "((" + szStdHour24Regex + ":" + szStdMinSec24Regex + ":" + szStdMinSec24Regex + ")|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒))|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点)(一十)(一|二|三|四|五|六|七|八|九)?(分钟|分)?)|(((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(1|2|3|一|二|两|三)(刻钟|刻))(" + szCHStdMinSec24Regex + "(秒钟|秒))?)|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(:|时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)?)"
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              "|((" +
                                       szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(:|分钟|分)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒)?)|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点半钟|点半|点钟|点|时))))|(((" + szStdHour24Regex + ":" + szStdMinSec24Regex + ":" + szStdMinSec24Regex + ")|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒))|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点)(一十)(一|二|三|四|五|六|七|八|九)?(分钟|分)?)|(((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(1|2|3|一|二|两|三)(刻钟|刻))(" + szCHStdMinSec24Regex + "(秒钟|秒))?)"
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   "|((" +
                                       szStdHour24Regex + "|" + szCHStdHour24Regex + ")(:|时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)?)|((" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(:|分钟|分)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒)?)|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点半钟|点半|点钟|点|时)))" + szTimeDescripRegex + ")|((((" + szCHStdHour24Regex + "|" + szStdHour24Regex + ")(时|点)(1|2|3|一|二|两|三)(刻钟|刻))|(" + szCHStdHour24Regex + "点半(钟)?))))|((" + szStdHour24Regex + ":" + szStdMinSec24Regex + ":" + szStdMinSec24Regex + ")|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒))"
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         "|((" +
                                       szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点)(一十)(一|二|三|四|五|六|七|八|九)?(分钟|分)?)|(((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(1|2|3|一|二|两|三)(刻钟|刻))(" + szCHStdMinSec24Regex + "(秒钟|秒))?)|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(:|时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)?)|((" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(:|分钟|分)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒)?)|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点半钟|点半|点钟|点|时)))|(((" + szCHStdHour24Regex + "|" + szStdHour24Regex + ")(时|点)(1|2|3|一|二|两|三)(刻钟|刻))|(" + szCHStdHour24Regex + "点半(钟)?)))(之前|之后|以前|以后|前|后)";
    std::string szStdTimeRegex = "((" + szTimeDescripRegex + "((" + szStdHour24Regex + ":" + szStdMinSec24Regex + ":" + szStdMinSec24Regex + ")|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒))|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点)(一十)(一|二|三|四|五|六|七|八|九)?(分钟|分)?)|(((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(1|2|3|一|二|两|三)(刻钟|刻))(" + szCHStdMinSec24Regex + "(秒钟|秒))?)|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(:|时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)?)|((" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(:|分钟|分)"
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             "(" +
                                 szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒)?)|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点半钟|点半|点钟|点|时))))|(((" + szStdHour24Regex + ":" + szStdMinSec24Regex + ":" + szStdMinSec24Regex + ")|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒))|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点)(一十)(一|二|三|四|五|六|七|八|九)?(分钟|分)?)|(((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(1|2|3|一|二|两|三)(刻钟|刻))(" + szCHStdMinSec24Regex + "(秒钟|秒))?)|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(:|时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)?)|((" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(:|分钟|分)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒)?)|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点半钟|点半|点钟|点|时)))" + szTimeDescripRegex + ")|((((" + szCHStdHour24Regex + "|" + szStdHour24Regex + ")(时|点)(1|2|3|一|二|两|三)(刻钟|刻))|(" + szCHStdHour24Regex + "点半(钟)?))))";
    std::string szStdTime24Regex = "((" + szStdHour24Regex + ":" + szStdMinSec24Regex + ":" + szStdMinSec24Regex + ")|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒))|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点)(一十)(一|二|三|四|五|六|七|八|九)?(分钟|分)?)|(((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(时|点)(1|2|3|一|二|两|三)(刻钟|刻))(" + szCHStdMinSec24Regex + "(秒钟|秒))?)|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(:|时|点)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(分钟|分)?)|((" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(:|分钟|分)(" + szStdMinSec24Regex + "|" + szCHStdMinSec24Regex + ")(秒钟|秒)?)|((" + szStdHour24Regex + "|" + szCHStdHour24Regex + ")(点半钟|点半|点钟|点|时)))";
    std::string szTimeLenRegex = "(((0|1|2|3|4|5|6|7|8|9)+|" + szCHNumRegex + "|两)(个)?(半)?(小时)((((零)?(0|1|2|3|4|5|6|7|8|9)+|" + szCHStdMinSec24Regex + "|半)(分钟|分|秒钟|秒))|(零)?(1|2|3|一|二|两|三)(刻钟|刻))?|半(个)?小时(((零)?(0|1|2|3|4|5|6|7|8|9)+|" + szCHStdMinSec24Regex + "|半)(分钟|分))?|((0|1|2|3|4|5|6|7|8|9)+|" + szCHStdMinSec24Regex + "|半)(分钟|分)(((零)?(0|1|2|3|4|5|6|7|8|9)+|" + szCHStdMinSec24Regex + ")(秒钟|秒))?|(((0|1|2|3|4|5|6|7|8|9)+(.(0|1|2|3|4|5|6|7|8|9)+)?|" + szCHNumRegex + "|两)(个半小时|个半钟头|个半分钟|个小时|小时|个钟头|钟头|分钟|分|秒钟|秒))|(一|二|两|三|1|2|3)(刻钟|刻))";
    regs_.push_back(make_tuple("REG.TIME", "szTimeDiffRegex", "(.*?)(" + szTimeDiffRegex + ")(.*?)"));
    regs_.push_back(make_tuple("REG.TIME", "szTimeDiffRegex2", "(.*?)(" + szTimeDiffRegex2 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.TIME", "szCalcTimeRangeRegex", "(.*?)(" + szCalcTimeRangeRegex + ")(.*?)"));
    regs_.push_back(make_tuple("REG.TIME", "szCalcTimePointRegex", "(.*?)(" + szCalcTimePointRegex + ")(.*?)"));
    regs_.push_back(make_tuple("REG.TIME", "szStdTimeRegex", "(.*?)(" + szStdTimeRegex + ")(.*?)"));
    regs_.push_back(make_tuple("REG.TIME", "szStdTime24Regex", "(.*?)(" + szStdTime24Regex + ")(.*?)"));
    regs_.push_back(make_tuple("REG.TIME", "szTimeLenRegex", "(.*?)(" + szTimeLenRegex + ")(.*?)"));
    regs_.push_back(make_tuple("REG.TIME", "szTimeDescripRegex", "(.*?)(" + szTimeDescripRegex + ")(.*?)"));

    //    regs_.push_back(make_tuple("REG.TIME", "re_time_total", "(.*?)("
    //        + szTimeDiffRegex + "|"
    //        + szTimeDiffRegex2 + "|"
    //        + szCalcTimeRangeRegex + "|"
    //        + szCalcTimePointRegex + "|"
    //        + szStdTimeRegex + "|"
    //        + szStdTime24Regex + "|"
    //        + szTimeLenRegex + "|"
    //        + szTimeDescripRegex + ")(.*?)"));

    //define <REG.TEL>
    std::string telNumber = "(0|1|2|3|4|5|6|7|8|9)";
    std::string szOrigTelNoRegex = "((零|幺|一|二|三|四|五|六|七|八|九|0|1|2|3|4|5|6|7|8|9|－|＋){3,17})";
    std::string szQuhao3 = "(010|020|021|022|023|024|025|027|028|029)";
    std::string szQuhao4 = "(0310|0311|0312|0313|0314|0315|0316|0317|0318|0319|0335|0349|0350|0351|0352|0353|0354|0355|0356|0357|0358|0359|0370|0371|0372|0373|0374|0375|0376|0377|0378|0379|0391|0392|0393|0394|0395|0398|0410|0411|0412|0413|0414|0415|0416|0417|0418|0419|0421|0427|0429|0431|0432|0433|0434|0435|0436|0437|0438|0439|0451|0452|0453|0454|0455|0456|0457|0458|0459|0464|0467|0468|0469|0470|0471|0472|0473|0474|0475|0476|0477|0478|0479|0482|0483|0510|0511|0512|0513|0514|0515|0516|0517|0518|0519|0523|0527|0530|0531|0532|0533|0534|0535|0536|0537|0538|0539|0543|0546|0550|0551|0552|0553|0554|0555|0556|0557|0558|0559|0561|0562|0563|0564|0565|0566|0570|0571|0572|0573|0574|0575|0576|0577|0578|0579|0580|0591|0592|0593|0594|0595|0596|0597|0598|0599|0631|0632|0633|0634|0635|0660|0662|0663|0668|0691|0692|0701|0710|0711|0712|0713|0714|0715|0716|0717|0718|0719|0722|0724|0728|0730|0731|0732|0733|0734|0735|0736|0737|0738|0739|0743|0744|0745|0746|0750|0751|0752|0753|0754|0755|0756|0757|0758|0759|0760|0762|0763|0766|0768|0769|0770|0771|0772|0773|0774|0775|0776|0777|0778|0779|0790|0791|0792|0793|0794|0795|0796|0797|0798|0799|0812|0813|0816|0817|0818|0825|0826|0827|0830|0831|0832|0833|0834|0835|0836|0837|0838|0839|0851|0852|0853|0854|0855|0856|0857|0858|0859|0870|0871|0872|0873|0874|0875|0876|0877|0878|0879|0883|0886|0887|0888|0891|0892|0893|0894|0895|0896|0897|0898|0901|0902|0903|0906|0908|0909|0911|0912|0913|0914|0915|0916|0917|0919|0930|0931|0932|0933|0934|0935|0936|0937|0938|0939|0941|0943|0951|0952|0953|0954|0955|0970|0971|0972|0973|0974|0975|0979|0990|0991|0993|0994|0995|0996|0997|0998|0999)";
    std::string szPhoneHeader = "(130|131|132|133|134|135|136|137|138|139|145|147|150|151|152|153|154|155|156|157|158|159|170|176|177|178|180|181|182|183|184|185|186|187|188|189)";
    std::string szHotTel = "((400|800)(0|1|2|3|4|5|6|7|8|9|－){7,10})";
    std::string szSepTel = "((955|100|123)" + telNumber + "{2}|17951|17911|17900|12110|12121|12117|12119|96102|96300|999|110|114|119|120|122)";
    std::string szTelNoRegex = "(";
    szTelNoRegex += "((＋)?(86)?(－)?";
    szTelNoRegex += "(" + szQuhao4 + "(－)?(0|1|2|3|4|5|6|7|8|9){7}";
    szTelNoRegex += "|" + szQuhao3 + "(－)?(0|1|2|3|4|5|6|7|8|9){4}(－)?(0|1|2|3|4|5|6|7|8|9){4}";
    szTelNoRegex += "|" + szPhoneHeader + "(－)?(0|1|2|3|4|5|6|7|8|9){4}(－)?(0|1|2|3|4|5|6|7|8|9){4}))";
    szTelNoRegex += "|(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9){6,7}";
    szTelNoRegex += "|" + szHotTel + "|" + szSepTel;
    szTelNoRegex += ")";
    regs_.push_back(make_tuple("REG.TEL", "szOrigTelNoRegex", "(.*?)(" + szOrigTelNoRegex + ")(.*?)"));
    regs_.push_back(make_tuple("REG.TEL", "szTelNoRegex", "(.*?)(" + szTelNoRegex + ")(.*?)"));

    //    regs_.push_back(make_tuple("REG.TEL", "re_tel_total", "(.*?)("
    //        + szOrigTelNoRegex + "|"
    //        + szTelNoRegex + ")(.*?)"));

    //define <REG.CALC>
    std::string calcPattern1 = "^.*(=|等于|得|是|为).*(((\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?\\^(\\d+|\\((\\+|\\-)?\\d+\\)|\\((\\+|\\-)?1/[1-9]+\\))|\\d+!|(\\+|\\-|正|负)?(0|1|2|3|4|5|6|7|8|9|\\.|p|零|一|二|三|四|五|六|七|八|九|十|百|千|万|亿|点|壹|贰|叁|肆|伍|陆|柒|捌|玖|拾|佰|仟|萬){1,64})|(自然对数|正百分之|负百分之|正百分|负百分|的百分|正根号|负根号|正跟号|负跟号|平方根|立方根|次方根|次根号|次跟号|百分之|百分|乘积|平方|立方|次方|次幂|根号|跟号|阶乘|对数|正弦|余弦|正切|余切|sin|cos|tan|cot|log|ln|和|差|积|商|倍|折|!)|(\\+|\\-|\\*|×|x|/|÷|加上|减去|乘以|乘上|除上|除以|分之|分|和|与|加|减|乘|除)).*$";
    std::string calcPattern2 = "^[0-9\\.\\(\\)eE!\\^%/p]+$";
    std::string calcPattern3 = "^[0-9\\.\\(\\)eE!\\^%\\+\\-/p]+$";
    std::string calcPattern4 = "^(自然对数|开平方根|开立方根|平方根|立方根|次方根|次根号|次跟号|开平方|开立方|百分之|然后再|然后|加上|减去|乘上|除上|乘以|除以|分之|乘积|平方|立方|次方|开方|次幂|根号|跟号|阶乘|对数|正弦|余弦|正切|余切|开|sin|cos|tan|cot|log|ln|加|减|乘|除|和|差|积|商|倍|折|与|零|半|一|二|三|四|五|六|七|八|九|十|百|千|万|亿|点|壹|贰|叁|肆|伍|陆|柒|捌|玖|拾|佰|仟|萬|正|负|的|得|之后|之|再|又|0|1|2|3|4|5|6|7|8|9|e|pi|%|\\^|\\.|\\+|\\-|\\*|×|x|/|÷|=|\\?|\\(|\\)|!){2,80}$";
    std::string calcPattern5 = "^(0|1|2|3|4|5|6|7|8|9|\\.|p|\\(|\\)|e|\\^|%|\\+|\\-|\\*|x|/|!|sin|cos|tan|cot|log|ln)+$";
    std::string calcPattern6 = "((\\d+)(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\d+)(\\.\\d+)?%|(\\d+)\\^(\\d+|\\(\\d+\\)|\\(1/\\d+\\))|(\\d+)!|(\\d+)(\\.\\d+)?|p)";
    std::string calcPattern7 = "^\\(?((\\+|\\-|\\*|×|x|/|÷|\\^)(\\(|\\)){0,}(sin|cos|tan|cot|log|ln)|(\\+|\\-|\\*|×|x|/|÷|\\^)(\\(|\\)){0,}((\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?\\^(\\d+|\\((\\+|\\-)?\\d+\\)|\\((\\+|\\-)?1/[1-9]+\\))|\\d+!|((\\+|\\-)?\\d+)(\\.\\d+)?|p|e)|(sin|cos|tan|cot|log|ln)|((\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?\\^(\\d+|\\((\\+|\\-)?\\d+\\)|\\((\\+|\\-)?1/[1-9]+\\))|\\d+!|((\\+|\\-)?\\d+)(\\.\\d+)?|p|e)|!|\\(|\\))+\\)?$";
    std::string calcPattern8 = "^((\\+|\\-)?\\d+)(\\.\\d+)?$";
    std::string calcPattern9 = "^\\(*((\\)|\\*|/|÷|×|x|\\^|!).*|(\\+|\\-){2}.*|(\\(\\)){1,}|.*\\)\\(.*|(\\+|\\-)?86[0-9\\-]{11,14}|(400|800|4006|4008)\\-[0-9\\-]{6,10}|(0[0-9]|[0-9]{4})(\\-)([1-9]|1[0-2])(\\-)([1-9]|1[0-9]|2[0-9]|3[0-1])|(0[0-9]|[0-9]{4})(/)([1-9]|1[0-2])(/)([1-9]|1[0-9]|2[0-9]|3[0-1])|([1-9]|1[0-2])(\\-)([1-9]|1[0-9]|2[0-9]|3[0-1])(\\-)([0-9]{2}|[0-9]{4})|([1-9]|1[0-2])(/)([1-9]|1[0-9]|2[0-9]|3[0-1])(/)([0-9]{2}|[0-9]{4})|([1-9]|1[0-9]|2[0-9]|3[0-1])(\\-)([1-9]|1[0-2])(\\-)([0-9]{2}|[0-9]{4})|([1-9]|1[0-9]|2[0-9]|3[0-1])(/)([1-9]|1[0-2])(/)([0-9]{2}|[0-9]{4})|.*(\\+|\\-|\\*|x|/|×|÷|\\^|\\(|sin|cos|tan|cot|log|ln)|0[0-9xX].*|.*[^0-9\\.]0[0-9].*|.*(\\-|/)0[0-9]+.*|.*[0-9\\.%]+(\\(|\\)){1,}[0-9]+.*|.*(/|÷)0(\\.0+)?([^0-9\\.].*)?|.*(!|\\^|%|\\.){2}.*|.*\\d+e|.*(\\d+(\\.\\d+)?e){2}.*|\\d+(ex|xe)(\\d+)?|ex?\\-?\\d+.*|.*ex?(\\.|e|p).*|pxe|.*p(\\d+(\\.\\d+)?|\\.|e|p).*|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|((\\+|\\-)?\\d+)(\\.\\d+)?|.*(\\d+)(\\.\\d+)?%(\\d+(\\.\\d+)?).*|.*((\\d+)(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\d+)(\\.\\d+)?%|(\\d+)\\^(\\d+|\\(\\d+\\)|\\(1/\\d+\\))){2}|(\\d+\\.\\d+){2}.*|.*\\d+(\\.\\d+)?\\(+((\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?\\^(\\d+|\\((\\+|\\-)?\\d+\\)|\\((\\+|\\-)?1/[1-9]+\\))|\\d+!|p|e).*|.*((\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?\\^(\\d+|\\((\\+|\\-)?\\d+\\)|\\((\\+|\\-)?1/[1-9]+\\))|\\d+!|p|e)\\)+\\d+(\\.\\d+)?.*|(sin|cos|tan|cot|log|ln)+|(\\+|\\-|\\*|×|x|/|÷|\\^)+|.*((\\+|\\-|\\*|×|x|/|÷|\\^)|(sin|cos|tan|cot|log|ln))\\).*|.*\\((\\*|/|÷|×|x|!|\\^).*|.*(sin|cos|tan|cot|log|ln)(\\*|/|÷|\\^|%|×|x).*|.*(\\(|\\)){0,}((\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?\\^(\\d+|\\((\\+|\\-)?\\d+\\)|\\((\\+|\\-)?1/[1-9]+\\))|\\d+!|((\\+|\\-)?\\d+)(\\.\\d+)?|p|e)(\\(|\\)){0,}(sin|cos|tan|cot|log|ln).*|.*((\\(|\\)){0,}(sin|cos|tan|cot|log|ln)(\\(|\\)){0,}((\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?\\^(\\d+|\\((\\+|\\-)?\\d+\\)|\\((\\+|\\-)?1/[1-9]+\\))|\\d+!|((\\+|\\-)?\\d+)(\\.\\d+)?|p|e)){2}.*)\\)*$";
    std::string calcPattern10 = "^\\(?((正|负)?(再|又|的|地|得|之)?(自然对数|正百分之|负百分之|正百分|负百分|的百分|正根号|负根号|正跟号|负跟号|平方根|立方根|次方根|次根号|次跟号|百分之|百分|乘积|平方|立方|次方|次幂|根号|跟号|阶乘|对数|正弦|余弦|正切|余切|sin|cos|tan|cot|log|ln|和|差|积|商|倍|折|!)|(的|地|得|之)?((\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?\\^(\\d+|\\((\\+|\\-)?\\d+\\)|\\((\\+|\\-)?1/[1-9]+\\))|(\\+|\\-|正|负)?(0|1|2|3|4|5|6|7|8|9|\\.|p|零|一|二|三|四|五|六|七|八|九|十|百|千|万|亿|点|壹|贰|叁|肆|伍|陆|柒|捌|玖|拾|佰|仟|萬){1,64})|(再|又|的|地|得|之)?(\\+|\\-|\\*|×|x|/|÷|加上|减去|乘以|乘上|除上|除以|分之|分|和|与|加|减|乘|除)|\\^|\\(|\\))+\\)?$";
    std::string calcPattern11 = "^\\(*(\\).*|.*\\(|(\\(\\)){1,}|.*(\\+|\\-|\\*|×|x|/|÷|\\^|sin|cos|tan|cot|log|ln)|(的|地|得|之)?((\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?\\^(\\d+|\\((\\+|\\-)?\\d+\\)|\\((\\+|\\-)?1/[1-9]+\\))|(\\+|\\-|正|负)?(0|1|2|3|4|5|6|7|8|9|\\.|p|零|一|二|三|四|五|六|七|八|九|十|百|千|万|亿|点|壹|贰|叁|肆|伍|陆|柒|捌|玖|拾|佰|仟|萬){1,64})|(正|负)?(再|又|的|地|得|之)?(自然对数|正百分之|负百分之|正百分|负百分|的百分|正根号|负根号|正跟号|负跟号|平方根|立方根|次方根|次根号|次跟号|百分之|百分|乘积|平方|立方|次方|次幂|根号|跟号|阶乘|对数|正弦|余弦|正切|余切|sin|cos|tan|cot|log|ln|和|差|积|商|倍|折|!)+|(再|又|的|地|得|之)?(\\+|\\-|\\*|×|x|/|÷|加上|减去|乘以|乘上|除上|除以|分之|分|和|与|加|减|乘|除)+|(再|又|的|地|得|之)?(\\+|\\-|\\*|×|x|/|÷|加上|减去|乘以|乘上|除上|除以|分之|分|和|与|加|减|乘|除)(\\(|\\)){0,}(的|地|得|之)?((\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?\\^(\\d+|\\((\\+|\\-)?\\d+\\)|\\((\\+|\\-)?1/[1-9]+\\))|(\\+|\\-|正|负)?(0|1|2|3|4|5|6|7|8|9|\\.|p|零|一|二|三|四|五|六|七|八|九|十|百|千|万|亿|点|壹|贰|叁|肆|伍|陆|柒|捌|玖|拾|佰|仟|萬){1,64})|(的|地|得|之)?((\\+|\\-)?\\d+(\\.\\d+)?(e)((\\+|\\-)?\\d+)(\\.\\d+)?|(\\+|\\-)?\\d+(\\.\\d+)?%|(\\+|\\-)?\\d+(\\.\\d+)?\\^(\\d+|\\((\\+|\\-)?\\d+\\)|\\((\\+|\\-)?1/[1-9]+\\))|(\\+|\\-|正|负)?(0|1|2|3|4|5|6|7|8|9|\\.|p|零|一|二|三|四|五|六|七|八|九|十|百|千|万|亿|点|壹|贰|叁|肆|伍|陆|柒|捌|玖|拾|佰|仟|萬){1,64})(\\(|\\)){0,}(再|又|的|地|得|之)?(\\+|\\-|\\*|×|x|/|÷|加上|减去|乘以|乘上|除上|除以|分之|分|和|与|加|减|乘|除))\\)*$";
    std::string calcPattern12 = "^.*[0-9]( )+[0-9].*$";
    std::string calcPattern13 = "(自然对数|平方根|立方根|次方根|次根号|次跟号|百分之|然后再|然后|加上|减去|乘上|除上|乘以|除以|分之|乘积|次幂|根号|跟号|阶乘|指数|对数|正弦|余弦|正切|余切|\\(|\\)|sin|cos|tan|cot|log|ln|平方|立方|次方|\\+|\\-|正|负|0|1|2|3|4|5|6|7|8|9|\\.|p|e|%|零|一|二|三|四|五|六|七|八|九|十|百|千|万|亿|点|壹|贰|叁|肆|伍|陆|柒|捌|玖|拾|佰|仟|萬|与|和|差|积|商|倍|折|加|减|乘|除|的|得|之后|之|再|又|\\*|×|x|/|÷|\\^|!)+";
    std::string calcPattern14 = "(\\+|\\-|\\*|×|x|/|÷|加上|减去|乘以|乘上|除上|除以|分之|分|和|与|加|减|乘|除)|(自然对数|正百分之|负百分之|正百分|负百分|的百分|正根号|负根号|正跟号|负跟号|平方根|立方根|次方根|次根号|次跟号|百分之|百分|乘积|平方|立方|次方|次幂|根号|跟号|阶乘|对数|正弦|余弦|正切|余切|sin|cos|tan|cot|log|ln|和|差|积|商|倍|折|!)|的|再|又";
    std::string calcPattern15 = "(自然对数|开平方根|开立方根|平方根|立方根|次方根|次根号|次跟号|开平方|开立方|百分之|然后再|然后|加上|减去|乘以|除以|分之|乘积|乘上|除上|相加|相减|相乘|相除|平方|立方|次方|开方|次幂|根号|跟号|阶乘|对数|正弦|余弦|正切|余切|元钱|块钱|块|元|开|（|）|sin|cos|tan|cot|log|ln|+|-|正|负|0|1|2|3|4|5|6|7|8|9|．|π|pi|e|%|加|减|乘|除|和|差|积|商|倍|打|折|与|零|半|两|一|二|三|四|五|六|七|八|九|十|百|千|万|亿|点|壹|贰|叁|肆|伍|陆|柒|捌|玖|拾|佰|仟|萬|正|负|的|之后|之|再|在|又|个|\\*|×|x|/|÷|\\^|!)+";
    std::string calcPattern16 = "(打)?((零|一|二|三|四|五|六|七|八|九)(点)?(零|一|二|三|四|五|六|七|八|九)?|[0-9](\\.)?[0-9]?)折";
    std::string calcPattern17 = "((零|一|二|三|四|五|六|七|八|九|十|百|千|万|亿|壹|贰|叁|肆|伍|陆|柒|捌|玖|拾|佰|仟|萬)+|[0-9]+)(个)((零|一|二|三|四|五|六|七|八|九|十|百|千|万|亿|点|壹|贰|叁|肆|伍|陆|柒|捌|玖|拾|佰|仟|萬)+|[0-9](\\.)?[0-9]?)(相加|相减|相乘|相除)";
    regs_.push_back(make_tuple("REG.CALC", "calcPattern1", "(.*?)(" + calcPattern1 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern2", "(.*?)(" + calcPattern2 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern3", "(.*?)(" + calcPattern3 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern4", "(.*?)(" + calcPattern4 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern5", "(.*?)(" + calcPattern5 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern6", "(.*?)(" + calcPattern6 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern7", "(.*?)(" + calcPattern7 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern8", "(.*?)(" + calcPattern8 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern9", "(.*?)(" + calcPattern9 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern10", "(.*?)(" + calcPattern10 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern11", "(.*?)(" + calcPattern11 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern12", "(.*?)(" + calcPattern12 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern13", "(.*?)(" + calcPattern13 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern14", "(.*?)(" + calcPattern14 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern15", "(.*?)(" + calcPattern15 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern16", "(.*?)(" + calcPattern16 + ")(.*?)"));
    regs_.push_back(make_tuple("REG.CALC", "calcPattern17", "(.*?)(" + calcPattern17 + ")(.*?)"));
    
    for (std::vector<std::tuple<std::string, std::string, std::string>>::iterator itre = regs_.begin(); itre != regs_.end(); itre++)
    {
        std::tuple<std::string, std::string, std::string> &item = *itre;
        std::string &patternTag = std::get<0>(item);
        std::string &patternName = std::get<1>(item);
        std::string &pattern = std::get<2>(item);
        //LOG_DEBUG("%s : %s : %s ", patternTag.c_str(), patternName.c_str(), pattern.c_str());

        std::string wrapped_pattern = "(" + pattern + ")";
        RE2::Options opt;
        opt.set_log_errors(false);
        opt.set_case_sensitive(false);
        //opt.set_utf8(false);
        RE2 *re2 = new RE2(wrapped_pattern, opt);
        if (!re2->ok())
        {
            /// Failed to compile regular expression.
            LOG_ERROR("Pattern compiled faield : %s, %s", patternTag.c_str(), patternName.c_str());
            delete re2;
        }
        else
        {
            saveCompiledPattern(pattern, re2);
        }
    }
}

RegexMatcher::~RegexMatcher()
{
    std::map<std::string, RE2 *>::iterator it;
    for (it = compiledPatterns_.begin(); it != compiledPatterns_.end(); ++it)
    {
        delete it->second;
    }
}

void RegexMatcher::saveCompiledPattern(const std::string &pattern, RE2 *re2)
{
    compiledPatterns_[pattern] = re2;
}

RE2 *RegexMatcher::searchCompiledPattern(const std::string &pattern)
{
    std::map<std::string, RE2 *>::iterator it;
    it = compiledPatterns_.find(pattern);
    if (it != compiledPatterns_.end())
    {
        return it->second;
    }
    return nullptr;
}

bool RegexMatcher::re2_full_match(const std::string &pattern, const std::string &patternTag, const std::string &patternName, const std::string &str, std::vector<std::string> &results)
{
    //    std::size_t str_hash = std::hash<std::string>{}(pattern);
    struct timeval start, end;
    results.clear();

    RE2 *re2ptr = searchCompiledPattern(pattern);
    if (re2ptr == nullptr)
    {
        return false;
    }

    RE2 &re2 = *re2ptr;

    /// Argument vector.
    std::vector<RE2::Arg> arguments;
    /// Vercor of pointers to arguments.
    std::vector<RE2::Arg *> arguments_ptrs;

    /// Get number of arguments.
    std::size_t args_count = re2.NumberOfCapturingGroups();

    /// Adjust vectors sizes.
    arguments.resize(args_count);
    arguments_ptrs.resize(args_count);
    results.resize(args_count);
    /// Capture pointers to stack objects and result object in vector..
    for (std::size_t i = 0; i < args_count; ++i)
    {
        /// Bind argument to string from vector.
        arguments[i] = &results[i];
        /// Save pointer to argument.
        arguments_ptrs[i] = &arguments[i];
    }

    re2::StringPiece piece(str);
    bool ret = RE2::FullMatchN(piece, re2, arguments_ptrs.data(), args_count);

    if (ret)
    {
        std::ostringstream oss;
        std::copy(results.begin(), results.end() - 1, std::ostream_iterator<std::string>(oss, ", "));
        oss << results.back();
        LOG_DEBUG("match query : %s , pattern : %s, %s", str.c_str(), patternTag.c_str(), patternName.c_str());
        LOG_DEBUG("matched : %s ", oss.str().c_str());
    }
    return ret;
}

void RegexMatcher::match(const SplitResult &splitResult, std::vector<MatchUnit> &result)
{
    for (std::vector<std::tuple<std::string, std::string, std::string>>::iterator itre = regs_.begin(); itre != regs_.end(); itre++)
    {
        std::tuple<std::string, std::string, std::string> &item = *itre;
        std::string &patternTag = std::get<0>(item);
        std::string &patternName = std::get<1>(item);
        std::string &pattern = std::get<2>(item);
        std::list<MatchUnit> sents;
        MatchUnit unit;
        unit.text_ = splitResult.rawQuery;
        sents.insert(sents.begin(), unit);
        matchSinglePattern(sents, pattern, patternTag, patternName);
        short offset = 0;
        for (std::list<MatchUnit>::iterator is = sents.begin(); is != sents.end(); is++)
        {
            if (is->tag_.find("REG.") != std::string::npos)
            {
                MatchUnit unit;
                unit.text_ = is->text_;
                unit.tag_ = is->tag_;
                unit.offset_ = offset;
                unit.count_ = is->text_.size();
                unit.norm_ = is->norm_;
                unit.source_ = "regex"; 
                result.push_back(unit);
            }
            offset += is->text_.size();
        }
    }
}

void RegexMatcher::matchSinglePattern(std::list<MatchUnit> &sents, const std::string &pattern, const std::string &patternTag, const std::string &patternName)
{
    if (sents.size() == 0)
    {
        return;
    }
    for (std::list<MatchUnit>::iterator it = sents.begin(); it != sents.end(); it++)
    {
        if (it->tag_.find("REG.") != std::string::npos)
        {
            continue;
        }
        std::string query = it->text_;
        std::list<MatchUnit>::iterator curr = it;
        std::string lst = "";
        std::string matched = "";
        std::string rst = "";
        std::list<MatchUnit>::iterator last = it;
        bool has_match = false;
        std::vector<std::string> results;
        while (re2_full_match(pattern, patternTag, patternName, query, results))
        {
            if (results.size() < 3)
            {
                LOG_ERROR("regex expression format errror: %s", pattern.c_str());
                break;
            }
            query = results[results.size() - 1];
            MatchUnit unit;
            if (results[1].size() > 0)
            {
                unit.text_ = results[1];
                unit.tag_ = it->tag_;
                curr++;
                curr = sents.insert(curr, unit);
            }
            unit.text_ = unit.norm_ = results[2];
            unit.tag_ = patternTag;
            curr++;
            curr = sents.insert(curr, unit);
            LOG_DEBUG("left: %s, matched: %s, norm: %s, right: %s", results[1].c_str(), results[2].c_str(), unit.norm_.c_str(), query.c_str());
            has_match = true;
        }
        if (has_match)
        {
            sents.erase(last);
            LOG_DEBUG("re2 finished");
            if (query.size() > 0)
            {
                MatchUnit unit;
                unit.text_ = query;
                curr++;
                curr = sents.insert(curr, unit);
            }
            it = curr;
        }
    }
}



}
