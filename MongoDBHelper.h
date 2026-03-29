#pragma once
#include "types.h"
#include <string>
#include <optional>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

class MongoDBHelper {
public:
    explicit MongoDBHelper(
        const std::string& uri = "mongodb://localhost:27017");

    static void init();
    std::optional<InstrumentData> findInstrument(const std::string& device_id) const;
    bool insertInstrument(const InstrumentData& data);
    bool isConnected() const;

private:
    mongocxx::client client_;
    static mongocxx::instance instance_;

    InstrumentData bson_to_instrument(const bsoncxx::document::view& doc) const;
};