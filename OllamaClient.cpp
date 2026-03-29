#include "OllamaClient.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

OllamaClient::OllamaClient(const std::string& base_url)
    : base_url_(base_url) {}

OllamaResponse OllamaClient::generateChat(
    const std::string& model,
    const std::string& system_prompt,
    const std::string& user_message,
    float temperature,
    int max_tokens,
    bool stream)
{
    OllamaResponse result;
    httplib::Client cli(base_url_);
    cli.set_connection_timeout(30);

    json request;
    request["model"] = model;

    json messages = json::array();
    messages.push_back({ {"role", "system"}, {"content", system_prompt} });
    messages.push_back({ {"role", "user"}, {"content", user_message} });

    request["messages"] = messages;
    request["stream"] = stream;
    request["temperature"] = temperature;
    request["max_tokens"] = max_tokens;

    auto res = cli.Post("/api/chat", request.dump(), "application/json");

    if (!res) {
        result.error = "Нет ответа от сервера Ollama. Убедитесь, что Ollama запущена.";
        return result;
    }
    if (res->status != 200) {
        result.error = "HTTP ошибка " + std::to_string(res->status);
        return result;
    }

    try {
        auto j = json::parse(res->body);
        if (j.contains("message") && j["message"].contains("content")) {
            result.content = j["message"]["content"].get<std::string>();
            result.success = true;
        }
        else {
            result.error = "Не найден ключ 'message.content' в ответе";
        }
    }
    catch (const std::exception& e) {
        result.error = "Ошибка парсинга JSON: " + std::string(e.what());
    }

    return result;
}

OllamaEmbedding OllamaClient::generateEmbedding(
    const std::string& embed_model,
    const std::string& text)
{
    OllamaEmbedding result;
    httplib::Client cli(base_url_);
    cli.set_connection_timeout(15);

    json request;
    request["model"] = embed_model;
    request["input"] = text;

    auto res = cli.Post("/api/embed", request.dump(), "application/json");

    if (!res || res->status != 200) {
        result.error = "Ошибка при получении эмбеддинга от Ollama";
        return result;
    }

    try {
        auto j = json::parse(res->body);
        if (j.contains("embedding") && j["embedding"].is_array()) {
            result.vector = j["embedding"].get<std::vector<float>>();
            result.success = true;
        }
        else if (j.contains("embeddings") && !j["embeddings"].empty()) {
            result.vector = j["embeddings"][0].get<std::vector<float>>();
            result.success = true;
        }
        else {
            result.error = "Не найден ключ 'embedding' или 'embeddings'";
        }
    }
    catch (const std::exception& e) {
        result.error = "Ошибка парсинга JSON эмбеддинга: " + std::string(e.what());
    }

    return result;
}