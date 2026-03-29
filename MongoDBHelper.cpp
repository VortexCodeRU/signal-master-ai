#include "MongoDBHelper.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>

using namespace bsoncxx::builder::stream;

mongocxx::instance MongoDBHelper::instance_{};

MongoDBHelper::MongoDBHelper(const std::string& mongo_uri)
    : client_{ mongocxx::uri{mongo_uri} }
{
    try {
        client_["admin"].run_command(bsoncxx::from_json(R"({"ping": 1})"));
        std::cout << "[MongoDBHelper] Подключение к MongoDB успешно\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[MongoDBHelper] Ошибка подключения к MongoDB: " << e.what() << "\n";
    }
}

void MongoDBHelper::init() {}

bool MongoDBHelper::isConnected() const {
    try {
        client_["admin"].run_command(bsoncxx::from_json(R"({"ping": 1})"));
        return true;
    }
    catch (...) {
        return false;
    }
}

std::optional<InstrumentData> MongoDBHelper::findInstrument(const std::string& device_id) const{
    if (!isConnected()) {
        std::cerr << "[MongoDBHelper] MongoDB не подключён\n";
        return std::nullopt;
    }

    auto db = client_["instruments"];
    auto coll = db["devices"];

    // search by normalized_name
    bsoncxx::builder::basic::document filter{};
    filter.append(bsoncxx::builder::basic::kvp("normalized_name", device_id));
    auto cursor = coll.find(filter.view());

    if (cursor.begin() != cursor.end()) {
        auto doc = *cursor.begin();
        std::cout << "[MongoDBHelper] Найден по normalized_name: " << device_id << "\n";
        return bson_to_instrument(doc);
    }

    // search by model
    filter.clear();
    filter.append(bsoncxx::builder::basic::kvp("model", device_id));
    cursor = coll.find(filter.view());

    if (cursor.begin() != cursor.end()) {
        auto doc = *cursor.begin();
        std::cout << "[MongoDBHelper] Найден по model: " << device_id << "\n";
        return bson_to_instrument(doc);
    }

    std::cout << "[MongoDBHelper] Прибор НЕ найден: " << device_id << "\n";
    return std::nullopt;
}

bool MongoDBHelper::insertInstrument(const InstrumentData& data) {
    if (!isConnected()) {
        std::cerr << "[MongoDBHelper] MongoDB не подключён\n";
        return false;
    }

    auto db = client_["instruments"];
    auto coll = db["devices"];

    document builder{};
    builder << "device_id" << data.device_id
        << "normalized_name" << data.normalized_name
        << "manufacturer" << data.manufacturer
        << "model" << data.model
        << "short_description" << data.short_description;

    // Sections
    array sections_array;
    for (const auto& sec : data.sections) {
        document sec_doc{};
        sec_doc << "title" << sec.title
            << "content" << sec.content;

        array emb_array;
        for (float val : sec.embedding) {
            emb_array << val;
        }
        sec_doc << "embedding" << emb_array;
        sections_array << sec_doc;
    }
    builder << "sections" << sections_array;

    // Commands
    document commands_doc;
    for (const auto& [key, cmd] : data.commands) {
        commands_doc << key << open_document
            << "scpi" << cmd.scpi
            << "description" << cmd.description
            << "group" << cmd.group
            << close_document;
    }
    builder << "commands" << commands_doc;

    try {
        coll.insert_one(builder.view());
        std::cout << "[MongoDBHelper] Прибор сохранён: " << data.normalized_name << "\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[MongoDBHelper] Ошибка вставки: " << e.what() << "\n";
        return false;
    }
}

InstrumentData MongoDBHelper::bson_to_instrument(const bsoncxx::document::view& doc) const {
    InstrumentData data;

    if (auto el = doc["device_id"]; el && el.type() == bsoncxx::type::k_string)
        data.device_id = std::string(el.get_string().value);

    if (auto el = doc["normalized_name"]; el && el.type() == bsoncxx::type::k_string)
        data.normalized_name = std::string(el.get_string().value);

    if (auto el = doc["manufacturer"]; el && el.type() == bsoncxx::type::k_string)
        data.manufacturer = std::string(el.get_string().value);

    if (auto el = doc["model"]; el && el.type() == bsoncxx::type::k_string)
        data.model = std::string(el.get_string().value);

    if (auto el = doc["short_description"]; el && el.type() == bsoncxx::type::k_string)
        data.short_description = std::string(el.get_string().value);

    // Sections
    if (auto sections_el = doc["sections"]; sections_el && sections_el.type() == bsoncxx::type::k_array) {
        for (auto&& elem : sections_el.get_array().value) {
            if (elem.type() == bsoncxx::type::k_document) {
                auto subdoc = elem.get_document().value;
                Section s;

                if (auto t = subdoc["title"]; t && t.type() == bsoncxx::type::k_string)
                    s.title = std::string(t.get_string().value);
                if (auto c = subdoc["content"]; c && c.type() == bsoncxx::type::k_string)
                    s.content = std::string(c.get_string().value);

                // Embedding
                if (auto emb_el = subdoc["embedding"]; emb_el && emb_el.type() == bsoncxx::type::k_array) {
                    for (auto&& val : emb_el.get_array().value) {
                        if (val.type() == bsoncxx::type::k_double)
                            s.embedding.push_back(static_cast<float>(val.get_double().value));
                        else if (val.type() == bsoncxx::type::k_int32)
                            s.embedding.push_back(static_cast<float>(val.get_int32().value));
                        else if (val.type() == bsoncxx::type::k_int64)
                            s.embedding.push_back(static_cast<float>(val.get_int64().value));
                    }
                }
                data.sections.push_back(s);
            }
        }
    }

    // Commands
    if (auto cmds_el = doc["commands"]; cmds_el && cmds_el.type() == bsoncxx::type::k_document) {
        auto cmds_doc = cmds_el.get_document().value;
        for (auto&& elem : cmds_doc) {
            std::string key = std::string(elem.key());
            if (elem.type() == bsoncxx::type::k_document) {
                auto cmd_doc = elem.get_document().value;
                CommandInfo cmd;

                if (auto scpi_el = cmd_doc["scpi"]; scpi_el && scpi_el.type() == bsoncxx::type::k_string)
                    cmd.scpi = std::string(scpi_el.get_string().value);
                if (auto desc_el = cmd_doc["description"]; desc_el && desc_el.type() == bsoncxx::type::k_string)
                    cmd.description = std::string(desc_el.get_string().value);
                if (auto grp_el = cmd_doc["group"]; grp_el && grp_el.type() == bsoncxx::type::k_string)
                    cmd.group = std::string(grp_el.get_string().value);

                data.commands[key] = cmd;
            }
        }
    }

    return data;
}