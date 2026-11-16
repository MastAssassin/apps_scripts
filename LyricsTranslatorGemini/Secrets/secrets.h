#pragma once
#include <optional>
#include <string>

using namespace std;

string getSecret(const string &name);
optional<string> loadSecret(const string&name);
bool saveSecret (const string& name, const string &value);
bool removeSecret (const string &name);
string setSecret(const string &name);
