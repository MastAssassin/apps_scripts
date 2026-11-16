#include "secrets.h"
#include <cstdlib>
#include <iostream>
#include <keychain/keychain.h>

using namespace keychain;
using namespace std;

namespace {
const string PACKAGE ="lyricstranslator";
const string SERVICE = "lyrics-translator";

    optional<string> fromEnv(const string &name) {
    const char *v = getenv(name.c_str());
    if (v && *v) return string(v);
    return nullopt;
    }
}
optional<string> loadSecret(const string &name) {
  Error err;
  string value = getPassword(PACKAGE, SERVICE, name, err);
  if (err)
  {
      if (err.type != ErrorType::NotFound)
          cerr << "keychain: " << err.message << "\n";
      return nullopt;
  }
return value;
}
bool saveSecret(const string& name, const string& value) {
    Error err;
    setPassword(PACKAGE, SERVICE, name, value, err);
    if (err) {
        cerr << "Could not save " << name << " to keychain: " << err.message << "\n";
        return false;
    }
    return true;
}

bool removeSecret(const string& name) {
    Error err;
    deletePassword(PACKAGE, SERVICE, name, err);
    return !err;
}

string getSecret(const string& name) {
    cout << "Searching for " << name << " in environment variables..." << endl;
    if (auto v = fromEnv(name)) return *v;
    cout << "Searching for " << name << " in OS-keychain..." << endl;
    if (auto v = loadSecret(name)) return *v;

    cout << "Nothing found there. Creating new entry " + name + "." << endl;
    string input;
    cout << "Secret: ";
    cin >>input;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (!input.empty() && saveSecret(name, input)) {
        cout << "Saved to keychain.\n";
        return input;
    }
     cout << "ERROR: something went wrong. :(" << endl;
    return "";
}

string getSecretSilence(const string &name)
{
    if (auto v = fromEnv(name)) return *v;
    if (auto v = loadSecret(name)) return *v;
    cout << "Could not find " << name << " in keychain nor environment."<<endl;
    return "";
}

string setSecret(const string &name)
{
    cout << "Paste your API-key here" << endl;
    string input;
    cout << "Secret: ";
    cin >>input;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (input.empty()) {
        cerr << "No secret entered.\n";
        return "";
    }
    if (saveSecret(name, input))
        cout << "Saved to keychain.\n";
    else
        cerr << "Warning: could not save to keychain, using key for this session only.\n";
    return input;
}

