#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <ostream>
#include <sched.h>
#include <sstream>
#include <algorithm>
#include <vector>
#include <functional>
#include <limits.h>
#include "Songinfo.h"
#include "Secrets/SpotifyToken.h"
#include "Secrets/secrets.h"

using namespace std;
using namespace cpr;
using json = nlohmann::json;

const string lrclib = "https://lrclib.net/api/get";
const string gemini= "https://generativelanguage.googleapis.com/v1beta/models/gemini-3.8-flash:generateContent";
const string spotify ="https://api.spotify.com/v1/me/player/currently-playing";
const string spotifyPlay = "https://api.spotify.com/v1/me/player/play";
const string spotifyDevices = "https://api.spotify.com/v1/me/player/devices";


string Songinfo::getSongline(double time) const
{
    auto it =upper_bound(times.begin(), times.end(), time);
    if (it == times.begin()) return "(---empty Line---)";
    return lines[(it - times.begin()) - 1];
}

double Songinfo::nextTime(double t) const
{
    auto it = upper_bound(times.begin(), times.end(), t);
    return it == times.end() ? duration : *it;
}

void Songinfo::setArr()
{
    lines.clear();
    times.clear();
    rawLyrics.clear();
    Response r = Get(
        Url{lrclib},
        Parameters{
            {"artist_name", artist},
            {"track_name", name},
            {"duration", to_string(static_cast<int>(duration))}},
                     Header{{"User-Agent", "LyricsTranslatorGemini"}},
                     Timeout{20000});

    if (r.status_code == 200)
    {
        try{
            json j = json::parse(r.text);
            cout << "Song found!" << endl;

            if (j["syncedLyrics"].is_string()) {
                rawLyrics = j["syncedLyrics"].get<string>();
                istringstream in (rawLyrics);
                string line;
                while (getline (in, line)){
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    int min=0;
                    double sec =0;
                    int cntVal = 0;
                    if (sscanf(line.c_str(), "[%d:%lf]%n", &min, &sec, &cntVal) == 2 &&cntVal > 0)
                    {
                        string text = line.substr(cntVal);
                        if (!text.empty() && text[0] == ' ') text.erase(0,1);
                        times.push_back(min * 60 + sec);
                        lines.push_back(text);
                    }
                }
            }
        }catch (const json::parse_error& e) {
            cerr << "Error: JSON Parser: " << e.what() << "\nPosition: " << e.byte << endl;
        }
        catch (const json::exception& e) {
            cerr << "JSON Error: " << e.what() << endl;
        }

    } else if (r.status_code == 404)
    {
        cout << "Error 404: Song couldn't be found!" << endl;
    }else{
        cerr << "Error " << r.status_code << endl
        << r.text << endl;
    }

}

void Songinfo::ArrFromCache(const string lyrics)
{
    lines.clear();
    times.clear();
    rawLyrics = lyrics;
    string line = "";
    istringstream ss(lyrics);
    while (getline (ss, line)){
        if (!line.empty() && line.back() == '\r') line.pop_back();
        int min=0;
        double sec =0;
        int cntVal = 0;
        if (sscanf(line.c_str(), "[%d:%lf]%n", &min, &sec, &cntVal) == 2 &&cntVal > 0)
        {
            string text = line.substr(cntVal);
            if (!text.empty() && text[0] == ' ') text.erase(0,1);
            times.push_back(min * 60 + sec);
            lines.push_back(text);
        }
    }
}

string Translation::getSongline(double time) const
{
    auto it =upper_bound(original->times.begin(), original->times.end(), time);
    if (it == original->times.begin()) return "(---empty Line---)";
    size_t i = (it - original->times.begin()) - 1;
    if (i >= translation.size()) return ""; // Übersetzung fehlt oder ist kürzer
    return translation[i];
}


void Translation::setArr(const string key)
{
    translation.clear();
    if (original->getLines().empty()) {
        cerr << "Gemini: no lyrics to translate." << endl;
        return;
    }

    json jsonPayload = {
        {"systemInstruction", {
            {"parts", json::array({
                {{"text", "Du bist ein Übersetzer für Songtexte. Übersetze jede Zeile "
                    "sinngemäß in die Zielsprache. Behalte Anzahl und Reihenfolge "
                    "der Zeilen exakt bei. Leere Zeilen bleiben leer."}}
            })}
        }},
        {"contents", json::array({
            {
                {"role", "user"},
                {"parts", json::array({
                    {{"text", "Zielsprache: " + lang +
                        "\nArtist: " + original->artist +
                        "\nTitel: " + original->name +
                        "\n\nZeilen (JSON):\n" + json(original->getLines()).dump()}}
                })}
            }
        })},
        {"generationConfig", {
            {"temperature", 0.3},
            {"responseMimeType", "application/json"},
            {"responseSchema", {
                {"type", "ARRAY"},
                {"items", {{"type", "STRING"}}}
            }}
        }}
    };



    Response r = Post(
        Url{gemini},
        Header{{"User-Agent", "LyricsTranslatorGemini"},
        {"Content-Type", "application/json"},
        {"x-goog-api-key", key}},
        Body{jsonPayload.dump()},
                      Timeout{120000});

    if (r.status_code == 0) {cerr << "Gemini: request failed: " << r.error.message << endl; return;}
    if (r.status_code != 200) {cerr << "Gemini Error: " << r.status_code << ":\n" << r.text << endl; return;}
    json j = json::parse(r.text, nullptr, false);

    if (!j.contains("candidates") || !j["candidates"].is_array() || j["candidates"].empty()) {
        cerr << "Gemini: no candidates";
        if (j.contains("promptFeedback"))
            cerr << " (" << j["promptFeedback"].value("blockReason", "?") << ")";
        cerr << endl;
        return;
    }

    const json& cand = j["candidates"][0];
    string finish = cand.value("finishReason", "");
    if (finish != "STOP") {
        cerr << "Gemini: canceled with finishReason=" << finish << endl;
        return;
    }
    if (cand.contains("content") && cand["content"].contains("parts")
        && cand["content"]["parts"].is_array() && !cand["content"]["parts"].empty()
        && cand["content"]["parts"][0].contains("text")
        && cand["content"]["parts"][0]["text"].is_string()) {

        json inner = json::parse(cand["content"]["parts"][0]["text"].get<string>(), nullptr, false);
    if (!inner.is_discarded() && inner.is_array()) {
        for (const auto& line : inner)
            translation.push_back(line.is_string() ? line.get<string>() : "");
    }
        }

        if (translation.size() != original->getLines().size()) {
            cerr << "Gemini: Count of lines does not match original count. (" << translation.size()
            << " instead of " << original->getLines().size() << ")" << endl;

        }
}

void Translation::ArrFromCache(const string lyrics)
{
    translation.clear();
    istringstream stream(lyrics);
    string line = "";
    while (getline(stream, line))
    {
        translation.push_back(line);
    }
}

// Führt einen Spotify-Request mit gültigem Token aus, bei 401 einmal mit neuem Token
static optional<Response> spotifyRequest(const function<Response(const string &token)> &send)
{
    static optional<Tokens> tok;
    static const string clientId = getSecret("SPOTIFY_CLIENT_ID");

    Response r;
    for (int attempt = 0; attempt < 2; ++attempt)
    {
        if (!tok || chrono::steady_clock::now() >= tok->expiresAt)
        {
            try {
                tok = ensureLogin(clientId); // Refresh oder Browser-Login
            } catch (const exception& e) {
                cerr << "Spotify: " << e.what() << endl;
                return nullopt;
            }
        }
        r = send(tok->access);
        if (r.status_code != 401) break;
        tok.reset();
    }
    return r;
}

static Response playOn(const string &token, const string &deviceId)
{
    Parameters p{};
    if (!deviceId.empty()) p.Add({"device_id", deviceId});
    return Put(Url{spotifyPlay}, p, Bearer{token},
               Header{{"User-Agent", "LyricsTranslatorGemini"}, {"Content-Length", "0"}},
               Timeout{20000});
}

bool startPlayback()
{
    auto r = spotifyRequest([](const string &t) { return playOn(t, ""); });
    if (!r) return false;

    if (r->status_code == 404) // kein aktives Gerät -> erstes verfügbares nehmen
    {
        auto d = spotifyRequest([](const string &t) {
            return Get(Url{spotifyDevices}, Bearer{t},
                       Header{{"User-Agent", "LyricsTranslatorGemini"}}, Timeout{20000});
        });
        json j = d ? json::parse(d->text, nullptr, false) : json();
        if (j.is_discarded() || !j.contains("devices") || !j["devices"].is_array() || j["devices"].empty()) {
            cout << "Spotify: no device found. Please open Spotify on any device." << endl;
            return false;
        }
        string id = j["devices"][0].value("id", "");
        cout << "Spotify: starting playback on " << j["devices"][0].value("name", "?") << "." << endl;
        r = spotifyRequest([&](const string &t) { return playOn(t, id); });
        if (!r) return false;
    }

    if (r->status_code == 200 || r->status_code == 202 || r->status_code == 204) return true;
    if (r->status_code == 403)
        cerr << "Spotify: playback can't be started (Spotify Premium required)." << endl;
    else
        cerr << "Spotify Error " << r->status_code << ":\n" << r->text << endl;
    return false;
}

optional<Playback> getPlayback()
{
    auto res = spotifyRequest([](const string &t) {
        return Get(Url{spotify}, Bearer{t},
                   Header{{"User-Agent", "LyricsTranslatorGemini"}}, Timeout{20000});
    });
    if (!res) return nullopt;
    Response &r = *res;

    if (r.status_code == 204) {
        cout << "Spotify: nothing is playing right now." << endl;
        return nullopt;
    }
    if (r.status_code == 401) {
        cerr << "Spotify: access token rejected." << endl;
        return nullopt;
    }
    if (r.status_code != 200) {
        cerr << "Spotify Error " << r.status_code << ":\n" << r.text << endl;
        return nullopt;
    }

    json j = json::parse(r.text, nullptr, false);
    if (j.is_discarded() || !j.contains("item") || !j["item"].is_object()) {
        cerr << "Spotify: unexpected response." << endl;
        return nullopt;
    }

    const json& item = j["item"];
    if (item.value("type", "") != "track") {
        cout << "Spotify: currently playing item is not a song." << endl;
        return nullopt;
    }

    string name = item.value("name", "");
    string artist;
    if (item.contains("artists") && item["artists"].is_array() && !item["artists"].empty())
        artist = item["artists"][0].value("name", "");
    double duration = item.value("duration_ms", 0) / 1000.0;

    if (name.empty() || artist.empty()) {
        cerr << "Spotify: missing title or artist." << endl;
        return nullopt;
    }
     erase(name,'/');
     erase(artist,'/');
     erase(name,'\\');
     erase(artist,'\\');

    double progress = j.value("progress_ms", 0) / 1000.0;
    bool playing = j.value("is_playing", false);

    return Playback{name, artist, duration, progress, playing};
}

Songinfo *getsonginfo()
{
    auto p = getPlayback();
    if (!p) return nullptr;
    return new Songinfo(p->name, p->artist, p->duration);
}

