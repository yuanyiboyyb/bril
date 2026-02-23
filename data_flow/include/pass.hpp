
#pragma once


#include<string>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<queue>
#include<list>
#include<memory>
#include<any>

class function;
class program;
class cache_manager;


using cfg_type = std::unordered_map<std::string,std::unordered_map<std::string,std::vector<std::string>>>;
using live_analyze = std::unordered_map<std::string,std::unordered_map<std::string,std::unordered_set<std::string>>>;
using some_set = std::unordered_map<std::string,std::unordered_set<std::string>>;
using cfg_fc_type = std::unordered_map<std::string,std::vector<std::string>>;
class Pass{
public:
    virtual ~Pass() = default;
    virtual std::string getName() const = 0;
    virtual void run(program&pg,cache_manager&cm) = 0;
    virtual std::vector<std::string> forward() const = 0;
protected:
    Pass() = default;
};

class cfg_generate_pass : public Pass{
public:
    std::string getName() const override {
        return "cfg_generate_pass";
    }
    std::vector<std::string> forward() const override{
        return {};
    }
    void run(program&pg,cache_manager&cm) override;    
};

class live_analyze_pass : public Pass{
public:
    std::string getName() const override {
        return "live_analyze_pass";
    }
    std::vector<std::string> forward() const override{
        return {"cfg_generate_pass"};
    }
    void run(program&pg,cache_manager&cm) override;    
};



class ssa_generate_pass : public Pass{
public:
    std::string getName() const override {
        return "ssa_generate_pass";
    }
    std::vector<std::string> forward() const override{
        return {"cfg_generate_pass","live_analyze_pass"};
    }
    void run(program&pg,cache_manager&cm) override; 
};

class ssa_variable{
public:
    std::string name;
    std::string new_name;
    int count = 0;


    ssa_variable() = default;
    ssa_variable(std::string var) : name(var), new_name(""), count(0) {}
    std::string get_new_name(){
        new_name = name + "_" +  std::to_string(count);
        count++;
        return new_name;
    }
    std::string get_name(){
        return new_name;
    }
};

class temp_basicblock{
public:
    std::string new_name;
    int count = 0;
    std::string get_new_name(){
        new_name =  "temp_" +  std::to_string(count);
        count++;
        return new_name;
    }
    std::string get_name(){
        return new_name;
    }
};

class ssa_delete_pass : public Pass{
public:
    std::string getName() const override {
        return "ssa_delete_pass";
    }
    std::vector<std::string> forward() const override{
        return {"ssa_generate_pass"};
    }
    void run(program&pg,cache_manager&cm) override; 
};



template<typename T>
void get_union(std::unordered_set<T>& a,const std::unordered_set<T>& b);
template<typename T>
void get_intersection(std::unordered_set<T>& a,const std::unordered_set<T>& b);
template<typename T>
void get_div(std::unordered_set<T>& a,const std::unordered_set<T>& b);

some_set generate_domi_set(function&fc,cfg_fc_type& cfg,cfg_fc_type& back_cfg);
some_set generate_domi_frontier(some_set& domi_set,cfg_fc_type& cfg);


