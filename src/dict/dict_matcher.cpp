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

DictMatcher::DictMatcher(const std::string &dict_name, 
    std::unordered_set<std::string> &patterns) : dict_name_(dict_name),
                                                 patterns_(std::move(patterns)),
                                                 //patterns_(patterns),
                                                 max_size_(0)
{
}

DictMatcher::~DictMatcher()
{
}

bool DictMatcher::init()
{
    if (patterns_.size() > 0)
    {
        max_size_ = 100;
        LOG_DEBUG("dict %s init success, size : %d, max length: %d", dict_name_.c_str(), patterns_.size(), max_size_);
        return true;
    }
    LOG_DEBUG("dict %s init failed", dict_name_.c_str());
    return false;
}

int DictMatcher::match(const SplitResult &splitResult, std::vector<MatchUnit> &result)
{
    if (splitResult.result.size() == 0)
    {
        return 0;
    }
    MatchUnit unit;
    if (patterns_.count(splitResult.rawQuery))
    {
        unit.text_ = unit.norm_ = splitResult.rawQuery;
        unit.count_ = splitResult.rawQuery.size();
        unit.offset_ = 0;
        unit.tag_ = dict_name_;
        unit.source_ = "dict";
        result.push_back(unit);
        return 1;
    }
    std::string prefix = "";
    std::vector<std::string> matchedWords;
    for (size_t i = 0; i < splitResult.result.size(); i++)
    {
        std::string temp = "";
        for (size_t j = 0; j < max_size_ && j + i < splitResult.result.size(); j++)
        {
            temp += splitResult.result[i + j].word;
            if (patterns_.count(temp))
            {
                matchedWords.push_back(temp);
            }
        }
        if (!matchedWords.empty())
        {
            for (std::vector<std::string>::iterator matchedWordsIter = matchedWords.begin(); matchedWordsIter != matchedWords.end(); ++matchedWordsIter)
            {
                unit.text_ = unit.norm_ = *matchedWordsIter;
                unit.count_ = (*matchedWordsIter).size();
                unit.offset_ = prefix.size();
                unit.tag_ = dict_name_;
                unit.source_ = "dict";
                result.push_back(unit);
            }
            matchedWords.clear();
        }
        prefix += splitResult.result[i].word;
    }
    return result.size();
}

DictMatcherMgr::DictMatcherMgr()
{
}

DictMatcherMgr::~DictMatcherMgr()
{
    for (std::map<std::string, DictMatcher *>::iterator it = matchers_.begin(); it != matchers_.end(); it++)
    {
        if (it->second != NULL)
        {
            delete it->second;
        }
    }
}

bool DictMatcherMgr::isRegularFile(const char *filename)
{
    struct stat buf;
    if (!(stat(filename, &buf) == 0))
    {
        return false;
    }
    return (buf.st_mode & S_IFREG) != 0;
}

int DictMatcherMgr::readFileList(const std::string &basePath, const std::string &terminal, std::vector<std::string> &files)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1000];
    files.clear();
    std::vector<std::string> parts;
    sogou::Splitter::Split(basePath, ";", parts);
    for (size_t i = 0; i < parts.size(); ++i)
    {
        if ((dir = opendir(parts[i].c_str())) == NULL)
        {
            LOG_ERROR("Open dir error : %s", parts[i].c_str());
            continue;
        }
        while ((ptr = readdir(dir)) != NULL)
        {
            std::string file(ptr->d_name);
            if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            {
                continue;
            }
            else if (isRegularFile((parts[i] + "/" + file).c_str()))
            {
                if (terminal.size() > 0)
                {
                    if (file.find(terminal) == file.size() - terminal.size())
                    {
                        LOG_DEBUG("d_name:%s/%s", parts[i].c_str(), ptr->d_name);
                        files.push_back(parts[i] + "/" + file);
                    }
                }
                else
                {
                    LOG_DEBUG("d_name:%s/%s", parts[i].c_str(), ptr->d_name);
                    files.push_back(parts[i] + "/" + file);
                }
            }
        }
        closedir(dir);
    }
    return files.size();
}

bool DictMatcherMgr::init(const std::string &vocab_dir)
{
    std::vector<std::string> files;
    if (!readFileList(vocab_dir, ".vocab", files))
    {
        LOG_ERROR("read vocab dir %s error!", vocab_dir.c_str());
        return false;
    }
    for (size_t i = 0; i < files.size(); i++)
    {
        sogou::Timer timer;
        timer.start();
        std::string file = files[i];
        LOG_DEBUG("***** dict file %s begin", file.c_str());
        std::ifstream infile(file.c_str());
        if (!infile)
        {
            LOG_ERROR("init matcher vocab failed");
            return false;
        }
        infile.seekg(0, std::ios::end);
        std::streampos length = infile.tellg();
        infile.seekg(0, std::ios::beg);
        //read the whole file into the buffer
        std::vector<char> buffer(length);
        infile.read(&buffer[0], length);
        infile.close();
        //create stringsteam
        std::stringstream localStream;
        localStream.rdbuf()->pubsetbuf(&buffer[0], length);
        //LOG_DEBUG("dict file %s read into string stream", file.c_str());
        //parse content
        std::string line = "";
        std::string tag = "";
        std::unordered_set<std::string> patternList;
        int duplicatedCount = 0;
        while (getline(localStream, line))
        {
            if (line.size() == 0)
            {
                continue;
            }
            if (line.at(0) == '<' && line.at(line.size() - 1) == '>')
            {
                if (line[1] == '/')
                {
                    if (tag.size() > 0 && tag.find("USER.") != std::string::npos)
                    {
                        DictMatcher *matcher = new DictMatcher(tag, patternList);
                        //LOG_DEBUG("dict %s read completed", tag.c_str());
                        if (matcher->init())
                        {
                            matchers_[tag] = matcher;
                        }
                        else
                        {
                            delete matcher;
                        }
                    }
                    //LOG_DEBUG("dict %s duplicated count : %d", tag.c_str(), duplicatedCount);
                    tag = "";
                    patternList.clear();
                    duplicatedCount = 0;
                }
                else
                {
                    tag = line;
                    //tag = line.substr(1, line.size() - 2);
                    //LOG_DEBUG("dict %s read start", tag.c_str());
                }
            }
            else
            {
                std::pair<std::unordered_set<std::string>::iterator, bool> ret = patternList.insert(line);
                if (ret.second == false)
                {
                    ++duplicatedCount;
                }
            }
        }
        timer.stop();
        //LOG_DEBUG("dict file %s cost time : %d us", file.c_str(), timer.costTime());
    }
    return true;
}

void DictMatcherMgr::match(const SplitResult &splitResult, std::vector<MatchUnit> &result)
{
    for (std::map<std::string, DictMatcher *>::iterator it = matchers_.begin(); it != matchers_.end(); it++)
    {
        std::vector<MatchUnit> myset;
        it->second->match(splitResult, myset);
        for (std::vector<MatchUnit>::iterator iter = myset.begin(); iter != myset.end(); iter++)
        {
            //iter->tag_ = it->first;
            result.push_back(*iter);
        }
    }
}

/**
 * 过滤重复的匹配结果
**/
void deduplicateMatchUnits(std::vector<MatchUnit> &result)
{
    std::vector<MatchUnit> filtratedResult;
    for (std::vector<MatchUnit>::iterator it = result.begin(); it != result.end(); it++)
    {
        if (it->text_ == "")
        {
            continue;
        }
        MatchUnit &unit = *it;
        bool found = false;
        for (std::vector<MatchUnit>::iterator iter = filtratedResult.begin(); iter != filtratedResult.end(); iter++)
        {
            MatchUnit &item = *iter;
            if (item.text_ == unit.text_ && item.norm_ == unit.norm_ && item.tag_ == unit.tag_ && item.offset_ == unit.offset_ && item.count_ == unit.count_ && item.source_ == unit.source_)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            filtratedResult.push_back(unit);
        }
    }
    result.clear();
    result.assign(filtratedResult.begin(), filtratedResult.end());
}


}
