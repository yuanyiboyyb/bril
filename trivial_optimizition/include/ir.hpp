#ifndef IR_HPP
#define IR_HPP
#include <utility>
#include <string>
#include <vector>
#include <variant>
#include "../include/json.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple> 
#include <list>
#include <unordered_set>

using json = nlohmann::json;
class Instruction{
public:
    json instr;
    json print();
};
class BasicBlock{
public:
    std::string name;
    std::list<Instruction> instrs;
    bool init = false;

    BasicBlock& operator=(BasicBlock&& other) noexcept;

    BasicBlock() = default;
    ~BasicBlock() = default;
    BasicBlock(const BasicBlock&) = default;
    BasicBlock(BasicBlock&&) = default;
    BasicBlock& operator=(const BasicBlock&) = default;

    void lvn_optimization();
    void deadcode_delete();
    json print();
};

class function{
public:
    json args;
    std::unordered_map<std::string,BasicBlock>basicblocks;
    std::vector<std::string> seq;
    json name;
    json type;
    
    json print();
    void BBopt();
};
class program{
public:
    std::unordered_map<std::string,function>functions;

    void print();
    void BBopt();
};

class lvn{
public:
    int count = 0;
    //name-number
    std::unordered_map<std::string,int>map1;
    //number-leader
    std::vector<std::string>map2;
    //expr-number
    std::unordered_map<std::string,int>map3;
    
    void push_back(Instruction& it);
private:
    bool is_alt_expr(const std::string& it);
    std::pair<std::string&,int> find_alt_expr(const std::string& it);
    bool is_alt_name(const std::string& it);
    std::string& find_alt_name(const std::string& it);
};

program parse(json & data);
#endif