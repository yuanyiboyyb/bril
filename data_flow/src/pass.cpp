#include "../include/pass.hpp"
#include "../include/ir.hpp" 


template<typename T>
void get_union(std::unordered_set<T>& a,
               const std::unordered_set<T>& b)
{
    for (const auto& it : b) {
        a.insert(it);
    }
}
template<typename T>
void get_intersection(std::unordered_set<T>& a,
                      const std::unordered_set<T>& b)
{
    for (auto it = a.begin(); it != a.end(); ) {
        if (b.count(*it) == 0) {
            it = a.erase(it);  // ✔ erase 返回下一个 iterator
        } else {
            ++it;
        }
    }
}
template<typename T>
void get_div(std::unordered_set<T>& a,
             const std::unordered_set<T>& b)
{
    for (const auto& it : b) {
        a.erase(it);
    }
}
some_set generate_domi_set(function&fc,cfg_fc_type& cfg,cfg_fc_type& back_cfg){
    some_set domi_tree;

    std::unordered_set<std::string>temp;
    for(auto&it:cfg){
        temp.insert(it.first);
    }
    temp.insert(fc.name);
    for(auto&it:cfg){
        domi_tree[it.first] = temp;
    }
    domi_tree[fc.name] = {fc.name};
    

    std::queue<std::string>worklist;
    for(auto&it:cfg[fc.name]){
        worklist.push(it);
    }
    
    while(!worklist.empty()){
        bool change = false;
        auto& bc_name = worklist.front();

        std::unordered_set<std::string> new_dom = temp;
        for(auto&pre:back_cfg[bc_name]){
            get_intersection(new_dom ,domi_tree[pre]);
        }
        new_dom.insert(bc_name);

        if(new_dom != domi_tree[bc_name]){
            change = true;
            domi_tree[bc_name] = new_dom;
        }
        
        if(change){
            for(auto&it:cfg[bc_name]){
                worklist.push(it);
            }
        }
        worklist.pop();   
    }
    return domi_tree;
}
some_set generate_domi_frontier(some_set& domi_set,cfg_fc_type& cfg){
    some_set domi_set_1;
    for(auto& [bc_name,set]:domi_set){
        for(auto&domid:set){
            domi_set_1[domid].insert(bc_name);
        }
    }
    some_set domi_frontier;
    for(auto&[bc_name,set]:domi_set_1){
        for(auto&domid:set){
            for(auto&sucs:cfg[domid]){
                if(set.count(sucs)==0){
                    domi_frontier[bc_name].insert(sucs);
                }
            }
        }
    }
    return domi_frontier;
}



void cfg_generate_pass::run(program&pg,cache_manager&cm){
    std::unordered_map<std::string,std::unordered_map<std::string,std::vector<std::string>>>cfg;
    std::unordered_map<std::string,std::unordered_map<std::string,std::vector<std::string>>>back_cfg;
    for(auto &func:pg.functions){
        std::unordered_map<std::string,std::vector<std::string>>fc_cfg;
        std::unordered_map<std::string,std::vector<std::string>>fc_back_cfg;
        auto &fc = func.second;
        for(auto it = fc.seq.begin();it!=fc.seq.end();it++){
            auto& temp = *it;
            auto&bc = fc.basicblocks[temp];
            auto&is = bc.instrs.back();
            if(is.instr["op"] == "jmp"){
                fc_cfg[temp].push_back(is.instr["labels"][0].get<std::string>());
                fc_back_cfg[is.instr["labels"][0].get<std::string>()].push_back(temp);
            }else if(is.instr["op"] == "br"){
                fc_cfg[temp].push_back(is.instr["labels"][0].get<std::string>());
                fc_cfg[temp].push_back(is.instr["labels"][1].get<std::string>());
                fc_back_cfg[is.instr["labels"][0].get<std::string>()].push_back(temp);
                fc_back_cfg[is.instr["labels"][1].get<std::string>()].push_back(temp);         
            }else if(is.instr["op"] == "ret"){
                continue;
            }
        }
        cfg[fc.name.get<std::string>()] = std::move(fc_cfg);
        back_cfg[fc.name.get<std::string>()] = std::move(fc_back_cfg);
    }
    cm.insert("cfg",std::move(cfg));
    cm.insert("back_cfg",std::move(back_cfg));
}   


void live_analyze_pass::run(program&pg,cache_manager&cm){
    cfg_type &cfg = std::any_cast<cfg_type&>(cm.get_any_ref("cfg"));
    cfg_type &back_cfg = std::any_cast<cfg_type&>(cm.get_any_ref("back_cfg"));
    std::unordered_map<std::string,std::unordered_map<std::string,std::string>>type;
    live_analyze in_fc;
    live_analyze out_fc;
    for(auto& fc_pair:pg.functions){
        std::unordered_map<std::string,std::string>fc_type;
        auto &fc = fc_pair.second;
        some_set in;
        some_set out;
        some_set kill;
        some_set gen;
        std::queue<std::string>worklist;
        for(auto& bc_pair:fc.basicblocks){
            auto& bc = bc_pair.second;
            worklist.push(bc.name);
            std::unordered_set<std::string>kill_bc;
            std::unordered_set<std::string>gen_bc;
            for(auto& instr:bc.instrs){
                if(instr.instr["op"] == "const"){
                    kill_bc.insert(instr.instr["dest"]);
                    fc_type[instr.instr["dest"]] = instr.instr["type"];
                }else if(istwocaculate(instr.instr["op"])){
                    kill_bc.insert(instr.instr["dest"]);
                    fc_type[instr.instr["dest"]] = instr.instr["type"];
                    if(!kill_bc.count(instr.instr["args"][0])){
                        gen_bc.insert(instr.instr["args"][0]);
                    }
                    if(!kill_bc.count(instr.instr["args"][1])){
                        gen_bc.insert(instr.instr["args"][1]);
                    }
                }else if(issinglecaculate(instr.instr["op"])){
                    kill_bc.insert(instr.instr["dest"]);
                    fc_type[instr.instr["dest"]] = instr.instr["type"];
                    if(!kill_bc.count(instr.instr["args"][0])){
                        gen_bc.insert(instr.instr["args"][0]);
                    }
                }else if(instr.instr["op"] == "br" || instr.instr["op"] == "print"){
                    if(!kill_bc.count(instr.instr["args"][0])){
                        gen_bc.insert(instr.instr["args"][0]);
                    }
                }else if(instr.instr["op"] == "ret"){
                    if(instr.instr.contains("args")){
                        if(!kill_bc.count(instr.instr["args"][0])){
                            gen_bc.insert(instr.instr["args"][0]);
                        }
                    }
                }else if(instr.instr["op"] == "call"){
                    if(instr.instr.contains("dest")){
                        kill_bc.insert(instr.instr["dest"]);
                    }
                    for(int i = 0;i < instr.instr["args"].size();i++){
                        if(!kill_bc.count(instr.instr["args"][i])){
                        gen_bc.insert(instr.instr["args"][i]);
                    }
                    }
                }
            }
            kill[bc.name] = std::move(kill_bc);
            gen[bc.name] = std::move(gen_bc);
        }

        auto& cfg_fc = cfg[fc.name];
        auto& cfg_back_fc = back_cfg[fc.name];
        while(!worklist.empty()){
            bool changed = false;
            std::unordered_set<std::string>new_out;
            std::unordered_set<std::string>new_in;
            auto& temp = worklist.front();
            for(auto&it:cfg_fc[temp]){
                get_union(new_out,in[it]);
            }
            new_in = new_out;
            get_div(new_in,kill[temp]);
            get_union(new_in,gen[temp]);
            if(in[temp]!=new_in || out[temp]!=new_out){
                in[temp] = std::move(new_in);
                out[temp] = std::move(new_out);
                changed = true;
            }
            if(changed){
                for(auto&it:cfg_back_fc[temp]){
                    worklist.push(it);
                }
            }
            worklist.pop();
        }
        type[fc.name] = std::move(fc_type);
        in_fc[fc.name] = std::move(in); 
        out_fc[fc.name] = std::move(out);  
    }
    cm.insert("live_analyze_in",std::move(in_fc));
    cm.insert("live_analyze_out",std::move(out_fc));
    cm.insert("var_type_record",std::move(type));
}

void insert_var(Instruction &instr,std::unordered_map<std::string, ssa_variable>&new_name){
    std::string dest_key = instr.instr["dest"].get<std::string>();
    auto [it, inserted] = new_name.try_emplace(dest_key, dest_key); // 传递变量名构造
    instr.instr["dest"] = it->second.get_new_name();
}
void ssa_generate_pass::run(program&pg,cache_manager&cm){
    cfg_type &cfg = std::any_cast<cfg_type&>(cm.get_any_ref("cfg"));
    cfg_type &back_cfg = std::any_cast<cfg_type&>(cm.get_any_ref("back_cfg"));
    auto &type = std::any_cast<std::unordered_map<std::string,std::unordered_map<std::string,std::string>>&>(cm.get_any_ref("var_type_record"));
    live_analyze &live_analyze_in = std::any_cast<live_analyze&>(cm.get_any_ref("live_analyze_in"));
    live_analyze &live_analyze_out = std::any_cast<live_analyze&>(cm.get_any_ref("live_analyze_out"));
    for(auto&[fc_name,fc]:pg.functions){
        auto& fc_cfg = cfg[fc.name];
        auto& fc_back_cfg = back_cfg[fc.name];
        auto& fc_in = live_analyze_in[fc_name];
        auto& fc_out = live_analyze_out[fc_name];
        auto& fc_type = type[fc_name];
        auto domi_set = generate_domi_set(fc,fc_cfg,fc_back_cfg);
        auto domi_frontier = generate_domi_frontier(domi_set,fc_cfg);
        
        std::unordered_map<std::string,ssa_variable>variable;
        std::unordered_set<std::string>var_flag;
        
        for(auto& [bc_name,bc]:fc.basicblocks){
            for(auto& frontier:domi_frontier[bc_name]){
                for(auto& var:fc_out[bc_name]){
                    if(fc_in[frontier].count(var) && !var_flag.count(frontier+var)){
                        fc.basicblocks[frontier].insert_phi(fc_back_cfg[frontier],var,fc_type[var]);
                        var_flag.insert(frontier+var);
                    }
                }
            }
        }

        std::unordered_map<std::string,ssa_variable>new_name;
        for(auto&arg:fc.args){
            std::string dest_key = arg["name"].get<std::string>();
            auto [it, inserted] = new_name.try_emplace(dest_key, dest_key); // 传递变量名构造
            arg["name"] = it->second.get_new_name();
        }

        std::unordered_set<std::string>flag;
        std::queue<std::string>worklist;
        worklist.push(fc.name);
        flag.insert(fc.name);
        while(!worklist.empty()){
            auto&temp = worklist.front();
            for(auto& instr:fc.basicblocks[temp].instrs){
                if(instr.instr["op"]=="phi"||instr.instr["op"] == "const"){
                    insert_var(instr,new_name);
                }else if(istwocaculate(instr.instr["op"])){
                    instr.instr["args"][0] = new_name[instr.instr["args"][0]].get_name();
                    instr.instr["args"][1] = new_name[instr.instr["args"][1]].get_name();
                    insert_var(instr,new_name);
                }else if(issinglecaculate(instr.instr["op"])){
                    instr.instr["args"][0] = new_name[instr.instr["args"][0]].get_name();
                    insert_var(instr,new_name);
                }else if(instr.instr["op"] == "br" || instr.instr["op"] == "print"){
                    instr.instr["args"][0] = new_name[instr.instr["args"][0]].get_name();
                }else if(instr.instr["op"] == "ret"){
                    if(instr.instr.contains("args")){
                        instr.instr["args"][0] = new_name[instr.instr["args"][0]].get_name();
                    }
                }else if(instr.instr["op"] == "call"){
                    for(int i = 0;i < instr.instr["args"].size();i++){
                        instr.instr["args"][i] = new_name[instr.instr["args"][i]].get_name();
                    }
                }
            }
            for(auto& suc:fc_cfg[temp]){
                int i = 0;
                for(auto& back:fc_back_cfg[suc]){
                    if(back == temp){
                        break;
                    }
                    i++;
                }
                for(auto& suc_instr:fc.basicblocks[suc].instrs){
                    if(suc_instr.instr["op"]=="phi"){

                        suc_instr.instr["args"][i] = new_name[suc_instr.instr["args"][i]].get_new_name();
                    }else{
                        break;
                    }
                }
                if(flag.count(suc)==0){
                    worklist.push(suc);
                    flag.insert(suc);
                }
            }
            worklist.pop();
        }
    }
}


void ssa_delete_pass::run(program&pg,cache_manager&cm){
    cfg_type &cfg = std::any_cast<cfg_type&>(cm.get_any_ref("cfg"));
    cfg_type &back_cfg = std::any_cast<cfg_type&>(cm.get_any_ref("back_cfg"));
    temp_basicblock basic_name;
    for(auto&[fc_name,fc]:pg.functions){
        auto&fc_cfg = cfg[fc_name];
        auto&fc_back_cfg = back_cfg[fc_name];
        for(auto&[bc_name,bc]:fc.basicblocks){
            bool flag = true;
            for(auto&pre:fc_back_cfg[bc_name]){
                if(fc_cfg[pre].size()!=1){
                    flag = false;
                    break;
                }
            }
            if(flag){
                for(auto it = bc.instrs.begin();(*it).instr["op"]=="phi";){
                    std::string dest = (*it).instr["dest"];
                    int i = 0;
                    for(auto& pre:fc_back_cfg[bc_name]){
                        json instr;
                        instr["op"] = "id";
                        instr["dest"] = dest;
                        instr["type"] =(*it).instr["type"];
                        instr["args"] = { (*it).instr["args"][i]};
                        auto num1 = std::prev(fc.basicblocks[pre].instrs.end());
                        fc.basicblocks[pre].instrs.insert(it,{std::move(instr)});
                        i++;
                    }
                    it = bc.instrs.erase(it);
                }
            }else{
                for(auto it = bc.instrs.begin();(*it).instr["op"]=="phi";){
                    std::string dest = (*it).instr["dest"];
                    int i = 0;
                    for(auto& pre:fc_back_cfg[bc_name]){
                        BasicBlock temp;
                        temp.name = basic_name.get_new_name();
                        json instr;
                        instr["op"] = "id";
                        instr["dest"] = dest;
                        instr["type"] =(*it).instr["type"];
                        instr["args"] = { (*it).instr["args"][i]};
                        temp.instrs.push_back({std::move(instr)});
                        instr["op"] = "jmp";
                        instr["labels"] ={bc_name};
                        temp.instrs.push_back({std::move(instr)});
                        fc.basicblocks[temp.name] = std::move(temp);
                        auto& cha = fc.basicblocks[pre].instrs.back();
                        if(cha.instr["op"] == "jmp"){
                            cha.instr["labels"][0] = basic_name.get_name();
                        }else{
                            if(cha.instr["labels"][0] == bc_name){
                                cha.instr["labels"][0] = basic_name.get_name();
                            }else{
                                 cha.instr["labels"][1] = basic_name.get_name();
                            }
                        }
                        i++;
                    }
                    it = bc.instrs.erase(it);
                }
            }
        }  
    }
}

