
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
    auto pro = parse(data);
    pro.BBopt();
    pro.print();
    return 0;
}