#pragma once
#include <nlohmann/json.hpp>
#include <optional>
#include "Songinfo.h"

using json = nlohmann::json;
struct CacheItem
{
    Songinfo *original;
    Translation *translation;
};

class Cache
{
private:
    nlohmann::json index;

public:
    void loadIndex();
    CacheItem readCache();
    void writeIndex();
    void addToCache(CacheItem a);
    CacheItem searchCache(string name, string artist, double duration, string lang);

    Cache(){loadIndex();}
};

static optional<json> parseFilename(const string &name, const string &sep, const string &file);
