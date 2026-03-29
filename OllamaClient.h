#pragma once
#include "types.h"
#include <string>

class OllamaClient {
public:
    OllamaClient(const std::string& base_url = "http://127.0.0.1:11434");

    OllamaResponse generateChat(
        const std::string& model,
        const std::string& system_prompt,
        const std::string& user_message,
        float temperature = 0.7f,
        int max_tokens = 800,
        bool stream = false);

    OllamaEmbedding generateEmbedding(
        const std::string& embed_model,
        const std::string& text);

private:
    std::string base_url_;
};