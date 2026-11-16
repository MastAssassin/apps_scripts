#include <cpr/cpr.h>
#include <cstddef>
#include <nlohmann/json.hpp>
#include <iostream>
#include <ostream>
#include <sched.h>
#include <cstdlib>
#include <algorithm>
#include <limits.h>
#include <chrono>
#include <thread>
#include <poll.h>
#include <unistd.h>
#include <cwchar>
#include <wchar.h>
#include "caching.h"
#include "utfcheck.h"

using json = nlohmann::json;
using namespace std;
using namespace cpr;

void menu(string *language);
void playLoop(Cache *cache, string *language);
void print (Songinfo *original, Translation *translation, double t);
size_t stringWidth (const string &s);
string fillWSpace(const string &s, size_t width);

int main()
{
     utfcheck();
    Cache *ch = new Cache();
    string targetlang;
    cout << "What's your target language?" << endl << "input: ";
    cin >> targetlang;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string *ptr= &targetlang;

    playLoop(ch, ptr);

    delete ch;
    return 0;
}

void print (Songinfo *original, Translation *translation, double t)
{
    static string title = " ";
    static string artist = " ";
    static size_t col = 0;
    if (original->getName() != title || original->getArtist() != artist)
    {
        col = 0;
        for (const auto& l : original->getLines()) col = max(col, stringWidth(l));
        title = original->getName();
        artist = original->getArtist();
        cout << "You are listening to " << title <<" by " << artist << " on spotify." << endl
        << "Lyrics found on lrclib.net. Translated by Gemini 3.8 Flash." << endl;
    }
    if (original->getLines().empty())
    {
        cout << "No synced lyrics found on lrclib.net for this song." << endl;
        return;
    }
    string a = original->getSongline(t);
    string b = translation->getSongline(t);
    cout << fillWSpace(a, col + 8) << b << endl;
}

size_t stringWidth (const string &s)
{
    size_t w = 0;
    mbstate_t state{};
    const char *p = s.data();
    size_t left = s.size();
    while (left > 0)
    {
        wchar_t wc;
        size_t len = mbrtowc(&wc, p, left, &state);
        if (len == (size_t)-1 || len == (size_t)-2) {
            state = mbstate_t{};
            ++w; ++p; --left;
            continue;
        }
        if (len == 0) len = 1;
        int cw = wcwidth(wc);
        if (cw > 0) w += cw;
        p += len; left -= len;
    }
    return w;
}

string fillWSpace(const string &s, size_t width)
{
    size_t w = stringWidth(s);
    return w >= width ? s:s + string(width - w, ' ');
}

void menu(string *language)
{
    int d = 0;
    for (;;){
        string line = "";
        cout<< endl << endl << "... menu ..." << endl
        <<"[0] continue (default)" <<endl
        <<"[1] change language" << endl
        <<"[2] pause" << endl
        <<"[3] exit" << endl
        << "Input: ";
        if (!getline(cin, line)) return;
        d = line.empty() ? 0 : atoi(line.c_str());
        if (d != 0 && d!=1 && d!=2 && d!=3) break;
        switch (d)
        {
            case 0: return;
            case 1: cout << "language: ";
                    cin >> *language;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    menu(language);
                    return;
            case 2:
                {cout << "Lyrics Translator is paused. Enter any key to continue..." << endl;
                string wait = "";
                getline(cin, wait);
                return;}
            case 3: exit(EXIT_SUCCESS);
            default: return;
        }
    }

}


using Clock = chrono::steady_clock;

struct SongClock
{
    Clock::time_point syncPoint = Clock::now();
    double offset = 0.0;
    bool playing = false;

    void sync(const Playback &p)
    {
        syncPoint = Clock::now();
        offset = p.progress;
        playing = p.playing;
    }
    double now() const
    {
        if (!playing) return offset;
        return offset + chrono::duration<double>(Clock::now() - syncPoint).count();
    }
}; //structs können Methoden haben ??? WTF!

static bool waitForInput(int timeoutMs)
{
    pollfd p{STDIN_FILENO, POLLIN, 0};
    return poll(&p, 1, max(timeoutMs, 0)) > 0 && (p.revents & POLLIN);
}

static optional<Playback> waitUntilPlaying()
{
    for (;;)
    {
        auto pb = getPlayback();
        if (pb && pb->playing) return pb;

        cout << endl << (pb ? "Spotify is paused." : "Nothing is playing right now.") << endl
             << "[Enter] play   [q] quit" << endl << "Input: ";
        string line;
        if (!getline(cin, line) || line == "q" || line == "Q") return nullopt;

        if (startPlayback())
            this_thread::sleep_for(chrono::milliseconds(800));
    }
}

void playLoop(Cache *cache, string *language)
{
    for (;;)
    {
        Songinfo *original = nullptr;
        Translation *translation = nullptr;
        SongClock clock;

        auto pb = waitUntilPlaying();
        if (!pb) return;
        CacheItem item = cache->searchCache(pb->name, pb->artist, pb->duration, *language);
        original = item.original;
        translation = item.translation;

        auto resync = [&]() -> bool {
            auto s = getPlayback();
            if (!s || s->name != original->getName() || s->artist != original->getArtist())
                return false;
            clock.sync(*s);
            return true;
        };

        bool sameSong = resync();
        double lastNext = -1;
        bool pauseHintShown = false;

        while (sameSong)
        {
            double t = clock.now();
            if (t >= original->getDuration()) break;

            double next = original->nextTime(t);
            if (next != lastNext)
            {
                print(original, translation, t);
                lastNext = next;
            }
            int waitMs = clock.playing ? static_cast<int>((next - t) * 1000) + 5 : 2000;

            if (!clock.playing && !pauseHintShown)
            {
                cout << "Spotify is paused. [p] + Enter = play, Enter = menu" << endl;
                pauseHintShown = true;
            }
            if (clock.playing) pauseHintShown = false;

            if (waitForInput(waitMs))
            {
                string input;
                getline(cin, input);
                if (input == "p" || input == "P") // Play-Button
                {
                    if (startPlayback()) this_thread::sleep_for(chrono::milliseconds(800));
                    sameSong = resync();
                    continue;
                }
                string oldLang = *language;
                menu(language);
                if (*language != oldLang) break;
                sameSong = resync();
                lastNext = -1;
            }
            else if (!clock.playing)
            {
                sameSong = resync();
            }
        }

        delete translation;
        delete original;
    }
}
