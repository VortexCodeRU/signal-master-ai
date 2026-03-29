#pragma once
#include "types.h"
#include "MongoDBHelper.h"
#include "OllamaClient.h"
#include <string>
#include <optional>

class Assistant {
public:
    explicit Assistant(
        const std::string& mongo_uri = "mongodb://localhost:27017",
        const std::string& ollama_url = "http://127.0.0.1:11434",
        const std::string& default_model = "qwen3:4b");

    std::string getResponse(const std::string& user_query, const std::string& device_id);
    std::optional<InstrumentData> findInstrument(const std::string& device_id) const;

    void setGenerationModel(const std::string& model_name);
    std::string getCurrentModel() const;

    static constexpr const char* FAST_MODEL = "qwen3:1.7b";
    static constexpr const char* DEEP_MODEL = "qwen3:4b";
    static constexpr const char* EMBED_MODEL = "nomic-embed-text";

private:
    MongoDBHelper mongo_;
    OllamaClient ollama_;
    std::string current_gen_model_;

    std::string buildSystemPrompt(const std::string& instrument_name) const;
    std::string buildRagContext(
        const InstrumentData& instr,
        const std::vector<float>& query_embedding,
        int top_k = 3) const;

    static float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);
};