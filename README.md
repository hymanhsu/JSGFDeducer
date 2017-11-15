JSGF解析
====

基于JSGF规则语法，使用自下而上的方法进行解析

<br><br>


#功能介绍

输入一句话，可以解析出相关的各种词槽信息，辅助业务程序进行句子意图的理解


<br><br>

#编译

mkdir build

cd build

cmake ..

make 

make install

<br><br>


#注意：

1, grammar名字就是Domain
<br>
2, 目标规则应该以rule_或pattern_开头
<br>
3, 所有正则词典名为 REG.*, 所有普通词典名为 USER.*
<br>


#使用方法

单个测试：
<br>
./WfstParserTest ../test/gram/music.gram  播放理查德钢琴曲
<br>
<br>
<br>

批量测试：
<br>
./BatchTest  ~/new_jsgf_parser/test/dict/  ~/new_jsgf_parser/test/gram/music.gram  ~/test/testfile.txt   ~/test/testoutput5.txt 
<br>
testfile.txt 为一行一个Query
<br>
testoutput5.txt 为输出的测试报告
<br>

