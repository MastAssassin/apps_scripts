#pragma once
#include <optional>
#include <string>
#include <vector>

using namespace std;


class Songinfo
{
    friend class Translation;
private:
    string artist;
    string name;
    double duration;
    vector<double> times;
    vector<string> lines;
    string rawLyrics;

public:
    Songinfo (string n, string a, double d):artist(a),name(n), duration(d){};
    string getArtist ()const{return artist;}
    string getName ()const{return name;}
    double getDuration()const{return duration;}
    vector<string> getLines() const{return lines;}
    string getRawLyrics() const{return rawLyrics;}
    string getSongline(double time) const;
    double nextTime(double t) const;
    void setArr();
    void ArrFromCache(const string lyrics);
};

class Translation
{

private:
    vector<string> translation;
    string lang;
    Songinfo *original;

public:
    Translation(string l, Songinfo *s): lang(l),original(s){};
    string getLang() const {return lang;}
    string getSongline(double time) const;
    vector<string> getLines() const{return translation;}
    void setArr(const string key);
    void ArrFromCache(const string lyrics);
};

struct Playback
{
    string name;
    string artist;
    double duration;
    double progress;
    bool playing;
};

optional<Playback> getPlayback();
bool startPlayback();
Songinfo *getsonginfo();
