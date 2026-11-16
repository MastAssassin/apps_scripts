#include <chrono>
#include <optional>
#include <random>
#include <iostream>
#include <sstream>
#include <cpr/cpr.h>
#include "cpr/cprtypes.h"
#include "secrets.h"
#include "SpotifyToken.h"
#include <nlohmann/json.hpp>
#include <openssl/sha.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

using namespace std;
using namespace cpr;
using json = nlohmann::json;

static const string tokenUrl   = "https://accounts.spotify.com/api/token";
static const string authUrl    = "https://accounts.spotify.com/authorize";
static const string redirectUri = "http://127.0.0.1:8888/callback";
static const int    redirectPort = 8888;
static const string scopes     = "user-read-currently-playing user-read-playback-state user-modify-playback-state";

// ---------- Hilfsfunktionen ----------

static string randomString(size_t len)
{
    static const char chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
    random_device rd;
    uniform_int_distribution<size_t> dist(0, sizeof(chars) - 2);
    string s;
    for (size_t i = 0; i < len; ++i) s += chars[dist(rd)];
    return s;
}

static string base64url(const unsigned char *data, size_t len)
{
    static const char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    string out;
    size_t i = 0;
    for (; i + 2 < len; i += 3) {
        uint32_t n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
        out += tbl[(n >> 18) & 63]; out += tbl[(n >> 12) & 63];
        out += tbl[(n >> 6) & 63];  out += tbl[n & 63];
    }
    if (len - i == 1) {
        uint32_t n = data[i] << 16;
        out += tbl[(n >> 18) & 63]; out += tbl[(n >> 12) & 63];
    } else if (len - i == 2) {
        uint32_t n = (data[i] << 16) | (data[i + 1] << 8);
        out += tbl[(n >> 18) & 63]; out += tbl[(n >> 12) & 63];
        out += tbl[(n >> 6) & 63];
    }
    return out; // ohne '=' Padding, wie von PKCE verlangt
}

static string codeChallenge(const string &verifier)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(verifier.data()), verifier.size(), hash);
    return base64url(hash, SHA256_DIGEST_LENGTH);
}

// Liest einen Query-Parameter aus "GET /callback?a=b&c=d HTTP/1.1"
static optional<string> queryParam(const string &requestLine, const string &key)
{
    size_t q = requestLine.find('?');
    if (q == string::npos) return nullopt;
    size_t end = requestLine.find(' ', q);
    string query = requestLine.substr(q + 1, end == string::npos ? string::npos : end - q - 1);

    size_t pos = 0;
    while (pos <= query.size()) {
        size_t amp = query.find('&', pos);
        string pair = query.substr(pos, amp == string::npos ? string::npos : amp - pos);
        size_t eq = pair.find('=');
        if (eq != string::npos && pair.substr(0, eq) == key)
            return util::urlDecode(pair.substr(eq + 1));
        if (amp == string::npos) break;
        pos = amp + 1;
    }
    return nullopt;
}

static void sendHtml(int fd, const string &status, const string &body)
{
    string resp = "HTTP/1.1 " + status + "\r\n"
                  "Content-Type: text/html; charset=utf-8\r\n"
                  "Content-Length: " + to_string(body.size()) + "\r\n"
                  "Connection: close\r\n\r\n" + body;
    (void)!write(fd, resp.data(), resp.size());
}

// Wartet auf den Redirect von Spotify und gibt den "code" zurück
static string waitForCallback(int server, const string &expectedState)
{
    const auto deadline = chrono::steady_clock::now() + chrono::minutes(5);

    while (chrono::steady_clock::now() < deadline) {
        pollfd p{server, POLLIN, 0};
        if (poll(&p, 1, 1000) <= 0) continue;

        int client = accept(server, nullptr, nullptr);
        if (client < 0) continue;

        char buf[8192];
        ssize_t n = read(client, buf, sizeof(buf) - 1);
        if (n <= 0) { close(client); continue; }
        buf[n] = '\0';
        string request(buf);
        string firstLine = request.substr(0, request.find("\r\n"));

        // z.B. favicon-Anfragen ignorieren
        if (firstLine.rfind("GET /callback", 0) != 0) {
            sendHtml(client, "404 Not Found", "");
            close(client);
            continue;
        }

        auto state = queryParam(firstLine, "state");
        auto code  = queryParam(firstLine, "code");
        auto error = queryParam(firstLine, "error");

        if (error) {
            sendHtml(client, "200 OK", "<h2>Spotify login canceled.</h2>You can close this tab.");
            close(client);
            throw runtime_error("Spotify login denied: " + *error);
        }
        if (!state || *state != expectedState || !code) {
            sendHtml(client, "400 Bad Request", "<h2>Invalid login response.</h2>");
            close(client);
            throw runtime_error("Spotify login: invalid state or missing code");
        }

        sendHtml(client, "200 OK",
                 "<h2>Login successful!</h2>You can close this tab and return to LyricsTranslatorGemini.");
        close(client);
        return *code;
    }
    throw runtime_error("Spotify login: timed out waiting for browser callback");
}

static Tokens parseTokenResponse(const Response &r)
{
    json j = json::parse(r.text);
    if (j.contains("refresh_token") && j["refresh_token"].is_string())
        saveSecret("SPOTIFY_REFRESH_TOKEN", j["refresh_token"].get<string>());
    int expiresIn = j.value("expires_in", 3600);
    return Tokens{j["access_token"].get<string>(),
                  chrono::steady_clock::now() + chrono::seconds(expiresIn - 60),
                  j.value("scope", "")};
}

// ---------- öffentliche Funktionen ----------

optional<Tokens> refreshAccessToken(const string& clientId, const string& refreshToken)
{
    auto r = Post (
        Url{tokenUrl},
        Payload{{"grant_type", "refresh_token"},
               {"refresh_token", refreshToken },
               { "client_id", clientId}});

    if (r.status_code == 400)
    {
        auto j = json::parse(r.text, nullptr, false);
        if (!j.is_discarded() && j.value("error", "") == "invalid_grant") return nullopt;
    }
    if (r.status_code != 200) throw runtime_error ("Token-Refresh failure: " + std::to_string(r.status_code));

    return parseTokenResponse(r);
}

Tokens fullBrowserLogin(const string& clientId)
{
    // Lokalen Server zuerst öffnen, damit der Redirect nicht ins Leere geht
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) throw runtime_error("Spotify login: socket() failed");
    int yes = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(redirectPort);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(server, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0 || listen(server, 4) < 0) {
        close(server);
        throw runtime_error("Spotify login: port " + to_string(redirectPort) + " is already in use");
    }

    const string verifier = randomString(64);
    const string state = randomString(16);

    string url = authUrl +
        "?response_type=code"
        "&client_id=" + util::urlEncode(clientId) +
        "&scope=" + util::urlEncode(scopes) +
        "&redirect_uri=" + util::urlEncode(redirectUri) +
        "&state=" + state +
        "&code_challenge_method=S256"
        "&code_challenge=" + codeChallenge(verifier);

    cout << "Please log in to Spotify in your browser. If it doesn't open automatically, visit:\n"
         << url << endl;
    string cmd = "xdg-open '" + url + "' >/dev/null 2>&1 &";
    (void)!system(cmd.c_str());

    string code;
    try {
        code = waitForCallback(server, state);
    } catch (...) {
        close(server);
        throw;
    }
    close(server);

    auto r = Post(
        Url{tokenUrl},
        Payload{{"grant_type", "authorization_code"},
                {"code", code},
                {"redirect_uri", redirectUri},
                {"client_id", clientId},
                {"code_verifier", verifier}});

    if (r.status_code != 200)
        throw runtime_error("Spotify login: token exchange failed (" + to_string(r.status_code) + "): " + r.text);

    cout << "Spotify login successful." << endl;
    return parseTokenResponse(r);
}

Tokens ensureLogin(const string& clientId)
{
    if (auto rt = loadSecret("SPOTIFY_REFRESH_TOKEN")) {
        if (auto t = refreshAccessToken(clientId, *rt)) {
            // Alte Tokens haben neue Scopes (z.B. Play) noch nicht -> einmal neu einloggen
            bool allScopes = true;
            istringstream need(scopes);
            for (string sc; need >> sc;)
                if (t->scope.find(sc) == string::npos) allScopes = false;
            if (allScopes) return *t;
            cout << "Spotify: new permissions required, please log in again." << endl;
        }
        removeSecret("SPOTIFY_REFRESH_TOKEN"); // abgelaufen/widerrufen/zu wenig Rechte
    }
    return fullBrowserLogin(clientId);
}
