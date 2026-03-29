#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>

struct CommandInfo {
    std::string scpi;
    std::string description;
    std::string group;
};

struct Section {
    std::string title;
    std::string content;
    std::vector<float> embedding;
};

struct InstrumentData {
    std::string device_id;
    std::string normalized_name;
    std::string manufacturer;
    std::string model;
    std::string short_description;
    std::vector<Section> sections;
    std::map<std::string, CommandInfo> commands;

    bool is_valid() const { return !normalized_name.empty(); }
};

struct OllamaEmbedding {
    std::vector<float> vector;
    bool success = false;
    std::string error;
};

struct OllamaResponse {
    std::string content;
    bool success = false;
    std::string error;
};