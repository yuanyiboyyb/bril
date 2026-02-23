#ifndef IR_HPP
#define IR_HPP
#include "./json.hpp"

#include <utility>
#include <string>
#include <vector>
#include <variant>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple> 
#include <list>
#include <unordered_set>
#include <any>
#include <queue>

#include "./pass.hpp"

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

    void insert_phi(std::vector<std::string>presc,std::string var,std::string type);
    json print();
};

class function{
public:
    json args;
    std::unordered_map<std::string,BasicBlock>basicblocks;
    std::list<std::string> seq;
    json name;
    json type;
    
    json print();
};
class program{
public:

    std::unordered_map<std::string,function>functions;

    void print();
};


class cache_manager {
public:
    void insert(const std::string& key, std::any value) {
        caches[key] = std::move(value);
    }

    std::any& get_any_ref(const std::string& key) {
        auto it = caches.find(key);
        if (it == caches.end()) {
            throw std::out_of_range("Cache key not found: " + key);
        }
        return it->second;
    }

    void remove(const std::string& key) {
        caches.erase(key);
    }
    size_t size() const { return caches.size(); }

private:
    std::unordered_map<std::string, std::any> caches;
};


class pass_manager{
public:
    void add_pass(std::unique_ptr<Pass> ps);
    void run(program&pg,cache_manager&cm);
private:
    std::unordered_map<std::string,std::unique_ptr<Pass>> pass;
    std::unordered_map<std::string,unsigned int>count;
    std::unordered_map<std::string,std::vector<std::string>>back;
};

class Context{
    using Callback_1 = std::function<void(program&,cache_manager&)>;
public:
    void init(json &data){
        pg = parse(data);
    }
    void add_pass(std::unique_ptr<Pass> ps){
        pm.add_pass(std::move(ps));
    }
    void print(){
        pg.print();
    }
    void runpass(){
        pm.run(pg,cm);
    }
    program parse(json& data);
private:
    program pg;
    pass_manager pm;
    cache_manager cm;
};


program parse(json & data);

// no class
bool iscaculate(const std::string& op);
bool istwocaculate(const std::string& op);
bool issinglecaculate(const std::string& op);
#endif