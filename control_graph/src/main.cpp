
#include "../include/json.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <sstream>

using json = nlohmann::json;



class Instruction{
public:
    json temp;
};
class BasicBlock{
public:
    std::string name;
    std::vector<Instruction> instrs;

    BasicBlock() = default;

    BasicBlock(BasicBlock&& other) noexcept
        : name(std::move(other.name)),
          instrs(std::move(other.instrs)) {}

    BasicBlock& operator=(BasicBlock&& other) noexcept {
        if (this != &other) {
            name = std::move(other.name);
            instrs = std::move(other.instrs);
        }
        return *this;
    }
};




void removeBrilComments( std::string& input) {
    std::regex commentRegex(R"(#.*)"); // 匹配以 # 开头的注释
    input =  std::regex_replace(input, commentRegex, ""); // 替换为空字符串
    return;
}

void printcfg(std::unordered_map<std::string,BasicBlock>&record){
    for(auto &[name,it]:record){
        if(it.instrs[it.instrs.size()-1].temp["op"]=="jmp"){
            std::cout << it.name << " -> " <<  it.instrs[it.instrs.size()-1].temp["labels"][0].get<std::string>() << std::endl;
        }else if(it.instrs[it.instrs.size()-1].temp["op"]=="br"){
            std::cout << it.name << " -> " <<  it.instrs[it.instrs.size()-1].temp["labels"][0].get<std::string>()<< std::endl;
            std::cout << it.name << " -> " <<  it.instrs[it.instrs.size()-1].temp["labels"][1].get<std::string>()<< std::endl;
        }
    }
}

void parse(json & data){
    for (const auto& func : data){
        std::unordered_map<std::string,BasicBlock> record;
        BasicBlock now;
        now.name = func["name"];
        for(const auto& intstr:func["instrs"]){
            if(intstr.contains("op")){
                if(intstr["op"]=="ret"){
                    now.instrs.push_back({intstr});
                    record[now.name] = std::move(now);
                }else if(intstr["op"]=="jmp"){
                    now.instrs.push_back({intstr});
                    record[now.name] = std::move(now);
                }else if(intstr["op"]=="br"){
                    now.instrs.push_back({intstr});
                    record[now.name] = std::move(now);
                }else{
                    now.instrs.push_back({intstr});
                }
            }else if(intstr.contains("label")){
                now.name = intstr["label"];
            }else{
                now.instrs.push_back({intstr});
            }
        } 
        printcfg(record);
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); 
    file.close();
    std::string content = buffer.str();

    removeBrilComments(content);
        
    json data;
    try {
        data = json::parse(content);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return 1;
    }
    parse(data["functions"]);
    return 0;
}