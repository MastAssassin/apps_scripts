#pragma once
#include <chrono>
#include <optional>
#include <string>
#include <cpr/cpr.h>

using namespace std;

struct Tokens { string access; chrono::steady_clock::time_point expiresAt; string scope; };
optional<Tokens> refreshAccessToken(const string& clientId, const string& refreshToken);
Tokens ensureLogin(const string& clientId);
Tokens fullBrowserLogin(const string& clientId);
