#include <util/splitter.h>
#include <simple_log.h>

#include <iostream>
#include <sstream>
#include <cassert>
#include <stdlib.h>
#include <ctype.h>

namespace sogou
{

bool Splitter::Split(const std::string &str,
                     const std::string &delimiter,
                     std::vector<std::string> &tok)
{
    if (str.empty())
    {
        return false;
    }
    tok.clear();
    // Skip delimiter at beginning.
    std::string::size_type lastPos = 0;
    // Find first "non-delimiter".
    std::string::size_type pos = str.find(delimiter, lastPos);
    if (pos == std::string::npos)
    {
        tok.push_back(str);
        return true;
    }
    while (std::string::npos != pos)
    {
        // Found a token, add it to the vector.
        tok.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiter.  Note the "not_of"
        lastPos = pos + delimiter.size();
        // Find next "non-delimiter"
        pos = str.find(delimiter, lastPos);
    }
    std::string last = str.substr(lastPos);
    if (!last.empty())
    {
        tok.push_back(last);
    }
    return true;
}

bool Splitter::SplitByChar(const std::string &str, SplitResult &splitResult)
{
    std::string buf;
    std::string word;
    buf = str;
    bool finished = true;
    int offset = 0;
    int pos = 0;
    
    CharContinueType currContinueType = INVALID;
    CharContinueType lastContinueType = INVALID;
    std::stringstream continueWordBuf;
    int continueWordOffset = -1;

    while (GetChar(word, buf))
    {
        CharType ch_type = CheckType(word);
        if (ch_type != NON)
        {
            bool makeTag = false;
            if(ch_type == MONOCHAR){
                char ch = word.at(0);
                bool foundEnglishChar = isalpha(ch);
                bool foundDigitChar = isdigit(ch) || ch == '.';
                if(foundEnglishChar){
                    currContinueType = ENGLISH;
                }
                if(foundDigitChar){
                    currContinueType = DIGIT;
                }
                if(currContinueType == ENGLISH || currContinueType == DIGIT){
                    if(currContinueType != lastContinueType){
                        if(continueWordOffset == -1){
                            continueWordOffset = offset;
                        }else{
                            std::string englishWord = continueWordBuf.str();
                            SplitWord splitWord;
                            splitWord.word = englishWord;
                            splitWord.offset = continueWordOffset;
                            splitWord.len = englishWord.size();
                            splitWord.start = pos;
                            splitWord.end = pos+1;
                            splitResult.result.push_back(splitWord);
                            ++pos;
                            //重置状态变量
                            continueWordOffset = offset;
                            continueWordBuf.str("");
                        }
                    }
                    continueWordBuf << ch;
                }else{
                    if(continueWordOffset != -1){
                        std::string englishWord = continueWordBuf.str();
                        SplitWord splitWord;
                        splitWord.word = englishWord;
                        splitWord.offset = continueWordOffset;
                        splitWord.len = englishWord.size();
                        splitWord.start = pos;
                        splitWord.end = pos+1;
                        splitResult.result.push_back(splitWord);
                        ++pos;
                        //重置状态变量
                        continueWordOffset = -1;
                        continueWordBuf.str("");
                    }
                    //是一个非英文单字符
                    makeTag = true;
                }
                //记录本次类型
                lastContinueType = currContinueType;
            }else{
                if(continueWordOffset != -1){
                    std::string englishWord = continueWordBuf.str();
                    SplitWord splitWord;
                    splitWord.word = englishWord;
                    splitWord.offset = continueWordOffset;
                    splitWord.len = englishWord.size();
                    splitWord.start = pos;
                    splitWord.end = pos+1;
                    splitResult.result.push_back(splitWord);
                    ++pos;
                    //重置状态变量
                    continueWordOffset = -1;
                    continueWordBuf.str("");
                }
                //是一个双字节或三字节字符
                makeTag = true;
            }

            if(makeTag){
                SplitWord splitWord;
                splitWord.word = word;
                splitWord.offset = offset;
                splitWord.len = word.size();
                splitWord.start = pos;
                splitWord.end = pos+1;
                splitResult.result.push_back(splitWord);
                ++pos;
            }

            //累加offset
            offset += word.size();
        }
        else
        {
            LOG_DEBUG("SplitByChar failed : %s has a non-utf8 char", str.c_str());
            finished = false;
            break;
        }
    }
    if (!finished)
    {
        return false;
    }
    //如果是以英文结束
    if(continueWordOffset != -1){
        std::string englishWord = continueWordBuf.str();
        SplitWord splitWord;
        splitWord.word = englishWord;
        splitWord.offset = continueWordOffset;
        splitWord.len = englishWord.size();
        splitWord.start = pos;
        splitWord.end = pos+1;
        splitResult.result.push_back(splitWord);
        ++pos;
        //重置状态变量
        continueWordOffset = -1;
        continueWordBuf.str("");
    }    
    splitResult.rawQuery = str;
    return true;
}

CharType Splitter::CheckType(const std::string &str)
{
    size_t len = str.size();
    if ((str[0] & 0x80) != 0x80)
    {
        return MONOCHAR;
    }
    else if ((str[0] & 0xc0) == 0xc0 && (str[0] & 0x20) != 0x20)
    {
        if (len > 1)
        {
            if ((str[1] & 0x80) == 0x80 && (str[1] & 0x40) != 0x40)
            {
                return BICHAR;
            }
            else
            {
                return NON;
            }
        }
        else
        {
            return NON;
        }
    }
    else if ((str[0] & 0xE0) == 0xE0 && (str[0] & 0x10) != 0x10)
    {
        if (len > 2)
        {
            if ((str[1] & 0x80) == 0x80 && (str[1] & 0x40) != 0x40 &&
                (str[2] & 0x80) == 0x80 && (str[2] & 0x40) != 0x40)
            {
                return TRICHAR;
            }
            else
            {
                return NON;
            }
        }
        else
        {
            return NON;
        }
    }
    else
    {
        return NON;
    }
}

bool Splitter::GetChar(std::string &word, std::string &str)
{
    size_t len = str.size();
    if (len == 0)
    {
        return false;
    }
    CharType type = CheckType(str);
    if (type == MONOCHAR)
    {
        word = str.substr(0, 1);
        if (str.size() > 0)
        {
            str = str.substr(1);
        }
        return true;
    }
    else if (type == BICHAR)
    {
        word = str.substr(0, 2);
        if (str.size() > 1)
            str = str.substr(2);
        return true;
    }
    else if (type == TRICHAR)
    {
        word = str.substr(0, 3);
        if (str.size() > 2)
            str = str.substr(3);
        return true;
    }
    else
    {
        return false;
    }
}
}
