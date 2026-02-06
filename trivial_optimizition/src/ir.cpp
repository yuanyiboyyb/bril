#include"../include/ir.hpp"


// no class
bool iscaculate(const std::string& op){
    return op == "add" ||op == "sub" || op == "add" 
    || op == "div" || op == "mod" || op == "ne" 
    || op == "eq" || op == "lt" || op == "le" 
    || op == "gt" || op == "ge" || op == "and"
    || op == "and" || op == "or" ||op == "not"
    || op == "const" || op =="id";   
}
bool istwocaculate(const std::string& op){
    return op == "add" ||op == "sub" || op == "add" 
    || op == "div" || op == "mod" || op == "ne" 
    || op == "eq" || op == "lt" || op == "le" 
    || op == "gt" || op == "ge" || op == "and"
    || op == "and" || op == "or" ;   
}
bool issinglecaculate(const std::string& op){
    return   op == "not" || op == "id";
}
BasicBlock& BasicBlock::operator=(BasicBlock&& other) noexcept {
    if (this != &other) {
        name = std::move(other.name);
        instrs = std::move(other.instrs);
        init = other.init; 
        other.init = false;
    }
    return *this;
}

program parse(json & data){
    program global;
    for (auto& func_json :data["functions"]){
        function func;
        func.name = func_json["name"];
        bool ifret = true;
        if(func_json.contains("type")){
            ifret = false;
            func.type = func_json["type"];
        }
        for(auto& args_json:func_json["args"]){
            func.args.push_back(args_json);
        }
        BasicBlock basicblock;
        basicblock.name = func.name;
        basicblock.init = true;
        for(auto& instrs_json:func_json["instrs"]){
            if(instrs_json.contains("label")){
                basicblock.name = instrs_json["label"];
            }else{
                if(iscaculate(instrs_json["op"]) || instrs_json["op"] == "call" || instrs_json["op"] == "print"){
                    basicblock.instrs.push_back(Instruction{std::move(instrs_json)});
                }else if(instrs_json["op"] == "jmp"){
                    basicblock.instrs.push_back(Instruction{std::move(instrs_json)}); 
                    func.seq.push_back(basicblock.name);
                    func.basicblocks.insert({basicblock.name,std::move(basicblock)});
                }else if(instrs_json["op"] == "br"){
                    basicblock.instrs.push_back(Instruction{std::move(instrs_json)});
                    func.seq.push_back(basicblock.name);
                    func.basicblocks.insert({basicblock.name,std::move(basicblock)}); 
                }else if(instrs_json["op"] == "ret"){
                    basicblock.instrs.push_back(Instruction{std::move(instrs_json)});
                    func.seq.push_back(basicblock.name);
                    func.basicblocks.insert({basicblock.name,std::move(basicblock)});
                }
            }
        }
        if(ifret){
            func.seq.push_back(basicblock.name);
            func.basicblocks.insert({basicblock.name,std::move(basicblock)});
        }
        global.functions.insert({func.name,std::move(func)});
    }
    return global;
}

//instruction

json Instruction::print(){
    return instr;
}



//BasicBlock
json BasicBlock::print(){
    json result = json::array();
    if(!this->init){
        result.push_back({"label",this->name});
    }
    for(auto &it:this->instrs){
        result.push_back(it.print());
    }
    return result;
}


void BasicBlock::lvn_optimization(){
    lvn device;
    for(auto& it:this->instrs){
        device.push_back(it);
    }
}
void BasicBlock::deadcode_delete(){
    std::unordered_set<std::string>flag;
    for (auto it = this->instrs.rbegin(); it != this->instrs.rend(); ++it) {
        json & temp = (*it).instr;
        if(iscaculate(temp["op"])){
            std::string dest =  temp["dest"].get<std::string>();
            if(flag.count(dest)){
                auto forward_it = it.base(); 
                --forward_it;              
                auto next_forward = this->instrs.erase(forward_it);
                it = std::make_reverse_iterator(next_forward);
            }else{
                flag.insert(temp["dest"]);
                for(auto &arg:temp["args"]){
                    flag.erase(arg.get<std::string>());
                }
            }
        }
    }
}


inline void append_array_to_array(json& target, const json& source) {
    for (const auto& item : source) {
        target.push_back(std::move(item));
    }
}
//fucntion
json function::print() {
    json result = json::object();
    result["args"] = this->args;
    json temp = json::array();
    for(auto &it:this->seq){
        append_array_to_array(temp,this->basicblocks[it].print());
    }
    result["instrs"] = std::move(temp);
    result["name"] = this->name;
    if(!this->type.is_null()){
        result["type"] = this->type;
    }
    return result;
}

void function::BBopt(){
    for(auto& [_,it]:this->basicblocks){
        it.lvn_optimization();
        it.deadcode_delete();
    }
}




//program
void program::print(){
    json result = json::object();
    json temp = json::array();
    for(auto &it:this->functions){
        temp.push_back(it.second.print());
    }
    result["functions"] = std::move(temp);
    std::cout << result.dump(2) << std::endl;
}

void program::BBopt(){
    for(auto& [_,it]:this->functions){
        it.BBopt();
    }
}




//lvn
void lvn::push_back(Instruction& instrs){
    json&  it =  instrs.instr;
    if(!it.contains("label")){
        if(istwocaculate(it["op"].get<std::string>())){
            std::string arg_1 = it["args"][0].get<std::string>();
            std::string arg_2 = it["args"][1].get<std::string>();
            if(this->is_alt_name(arg_1)){
                arg_1 = this->find_alt_name(arg_1);
            }
            if(this->is_alt_name(arg_2)){
                arg_2 = this->find_alt_name(arg_2);
            }
            if(arg_1 > arg_2){
                std::swap(arg_1, arg_2);
            }
            it["args"] = {arg_1,arg_2};
            std::string expr = arg_1+it["op"].get<std::string>()+arg_2;
            std::string dest = it["dest"].get<std::string>();
            if(this->is_alt_expr(expr)){
                auto [new_expr,index] = this->find_alt_expr(expr);
                it = {
                    {"op","id"},
                    {"dest",it["dest"]},
                    {"type",it["type"]},
                    {"args",{new_expr}}
                };
                map1[dest] = index;
            }else{
                map1[dest] = count;
                map3[expr] = count;
                count++;
                map2.push_back(dest);
            }
        }else if(issinglecaculate(it["op"].get<std::string>())){
            std::string arg_1 = it["args"][0].get<std::string>();
            if(this->is_alt_name(arg_1)){
                arg_1 = this->find_alt_name(arg_1);
                it["args"][0] = arg_1;
            }
            std::string expr = it["op"].get<std::string>()+arg_1;
            std::string dest = it["dest"].get<std::string>();
            if(this->is_alt_expr(expr)){
                auto [new_expr,index] = this->find_alt_expr(expr);
                it = {
                    {"op","id"},
                    {"dest",it["dest"]},
                    {"type",it["type"]},
                    {"args",{new_expr}}
                };
                map1[dest] = index;
            }else{
                map1[dest] = count;
                map3[expr] = count;
                count++;
                map2.push_back(dest);
            }
        }else if(it["op"].get<std::string>() == "const"){
            map1[it["dest"].get<std::string>()] = count;
            map3[it["dest"].get<std::string>()] = count;
            count++;
            map2.push_back(it["dest"].get<std::string>());
        }
    }
}
bool lvn::is_alt_expr(const std::string& it){
    return (map3.find(it) != map3.end() && map1[map2[map3[it]]]==map3[it]);
}
std::pair<std::string&,int> lvn::find_alt_expr(const std::string& it){
    int a = map3[it];
    return {map2[a],a};
}
bool lvn::is_alt_name(const std::string& it){
    return (map1.find(it) != map1.end() && map1[map2[map1[it]]]==map1[it]);
}
std::string& lvn::find_alt_name(const std::string& it){
    return map2[map1[it]];
}




