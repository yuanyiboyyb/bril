
#include "../include/ir.hpp"

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

        
    json data;
    try {
        data = json::parse(content);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return 1;
    }
    Context context;
    context.init(data);

    context.add_pass(std::make_unique<cfg_generate_pass>());
    context.add_pass(std::make_unique<live_analyze_pass>());
    context.add_pass(std::make_unique< ssa_generate_pass>());
    context.add_pass(std::make_unique< ssa_delete_pass>());

    context.runpass();
    context.print();
    return 0;
}