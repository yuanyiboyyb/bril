#include"../include/ir.hpp"


BasicBlock& BasicBlock::operator=(BasicBlock&& other) noexcept {
    if (this != &other) {
        name = std::move(other.name);
        instrs = std::move(other.instrs);
        init = other.init; 
        other.init = false;
    }
    return *this;
}

program Context::parse(json & data){
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

void BasicBlock::insert_phi(std::vector<std::string>presc,std::string var,std::string type){
    Instruction phi_inst;
    phi_inst.instr["op"] = "phi";
    phi_inst.instr["dest"] = var;
    phi_inst.instr["type"] = type;

    phi_inst.instr["args"] = json::array();
    phi_inst.instr["labels"] = json::array();
    for(auto &pre:presc){
        phi_inst.instr["args"].push_back(var);   // 先填原变量名
        phi_inst.instr["labels"].push_back(pre);
    }
    instrs.emplace_front(std::move(phi_inst));
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
        json label_json;
        if(it!=this->name){
            label_json["label"] = it;
            temp.push_back(std::move(label_json));
        }
        append_array_to_array(temp,this->basicblocks[it].print());
    }
    result["instrs"] = std::move(temp);
    result["name"] = this->name;
    if(!this->type.is_null()){
        result["type"] = this->type;
    }
    return result;
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


//pass_manager
void pass_manager::add_pass(std::unique_ptr<Pass> ps){
    auto name = ps->getName();
    auto dependencies = ps->forward();
    pass.insert({name,std::move(ps)});
    for(auto& forward:dependencies){
        back[forward].push_back(name);
    }
    count[name] = dependencies.size();
}

void pass_manager::run(program&pg,cache_manager&cm){
    auto temp = count;
    std::queue<std::string>worklist;
    for(auto& [name,number]:temp){
        if(number == 0){
            worklist.push(name);
        }
    }
    while(!worklist.empty()){
        auto it = worklist.front();
        worklist.pop();
        pass[it]->run(pg,cm);
        for(auto&last:back[it]){
            temp[last]--;
            if(temp[last]==0){
                worklist.push(last);
            }
        }
    }
}

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




