/*************************************************  
Author: Kevin.XU
Date:   2017-10-26
Desc:   单词分割
**************************************************/ 

#ifndef SPLITER_H
#define SPLITER_H

#include <string>
#include <vector>

namespace sogou
{

enum CharContinueType
{
    INVALID,
    ENGLISH,
    DIGIT
};

enum CharType
{
  NON,
  MONOCHAR,
  BICHAR,
  TRICHAR
};

struct SplitWord
{
  std::string word;
  int offset;
  int len;
  int start;
  int end;
};

struct SplitResult
{
  std::string rawQuery;
  std::vector<SplitWord> result;
};

class Splitter
{
public:
  static bool Split(const std::string &str,
                    const std::string &delimiters,
                    std::vector<std::string> &tok);

  static bool SplitByChar(const std::string &str, SplitResult &result);

private:
  static CharType CheckType(const std::string &str);
  static bool GetChar(std::string &word, std::string &str);
};
}
#endif
