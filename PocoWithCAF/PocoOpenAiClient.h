#pragma once
#include <string>

std::string getEnv(std::string envVar);

class OpenAIClient {
private:
    std::string apiKey;
    std::string model;
public:
    OpenAIClient(const std::string& modelName = "gpt-4");
    std::string generateCompletion(const std::string& prompt);
};
