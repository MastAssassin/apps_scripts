#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include "caching.h"
#include "Secrets/secrets.h"

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;
fs::path indexPath = fs::path(".cache/index.json");
bool cacheError = false;
const string sep = "_";


static string joinLines(const vector<string> &lines)
{
    string text;
    for (const auto &line : lines) text += line + '\n';
    return text;
}

static string readFile(const fs::path &p)
{
    ifstream file(p);
    if (!file.is_open())
    {
        cerr << "Error: File " << p << " could not be read." << endl;
        exit(EXIT_FAILURE);
    }
    stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static optional<json> parseFilename(const string &name, const string &sep, const string &file)
{
    size_t first = name.find(sep);
    size_t last = name.rfind(sep);
    if (first == string::npos || last == first) return nullopt;
    size_t durSep = name.rfind(sep, last - 1);
    if (durSep == string::npos || durSep == first) return nullopt;

    string artist = name.substr(0, first);
    string title = name.substr(first + sep.size(), durSep - first - sep.size());
    string durStr = name.substr(durSep + sep.size(), last - durSep - sep.size());
    string lang = name.substr(last + sep.size());

    double duration = 0.0;
    try {
        size_t used = 0;
        duration = stod(durStr, &used);
        if (used != durStr.size()) return nullopt;
    } catch (const exception &) { return nullopt; }

    if (artist.empty() || title.empty() || lang.empty() || duration <= 0.0) return nullopt;

    return json{
        {"artist", artist},
        {"title", title},
        {"duration", duration},
        {"language", lang},
        {"file", file}};
}

void Cache::loadIndex()
{
    if(!fs::exists(".cache")) {fs::create_directory(".cache");}
    if (!fs::exists(indexPath))
    {
        cout << "There's no Indexfile! Creating new one." << endl
             << "Searching for cached lyrics." << endl;
         index = json::array();
        for (const auto & entry: fs::directory_iterator(".cache"))
        {
            if (!entry.is_regular_file()) continue;
            if (entry.path().filename() == "index.json") continue;

            string name = entry.path().stem().string();
            auto e = parseFilename(name, sep, entry.path().filename().string());
            if (!e) {cerr << "Invalid name: " << name << endl; continue;}

            index.push_back(*e);
            cout << "Found: " << entry.path().filename().string() << endl;
        }
        writeIndex();

    } else{

     ifstream file(indexPath);

        if (!file.is_open())
        {
            cerr << "Error: Indexfile could not be read."<< endl
                 << "WHY? I don't know." << endl;
            exit(EXIT_FAILURE);
        }
        index =json::parse(file);
        file.close();
        }
        char d = ' ';
        if (!cacheError){
            for (;;) {
                cout << "Integrity Check? (y/N)" << endl;
                string line;
                if (!getline(cin, line)) return;
                d = line.empty() ? 'n' : line[0];
                if (d == 'y' || d == 'Y' || d == 'n' || d == 'N') break;
                cout << "Illegal input!" << endl;
            }
        } else {d = 'y';}
        switch (d) {
            case 'n':
            case 'N':
            case '\n':return;
            case 'Y':
            case 'y':{
                cout << "Searching for missing index entries..." << endl;
                int cnt = 0;
                for (const auto & entry: fs::directory_iterator(".cache"))
                {
                    if (!entry.is_regular_file()) continue;
                    if (entry.path().filename() == "index.json") continue;
                    cnt++;
                    cout << "Found: " << entry.path().filename().string() << endl;
                }
                 int indexcnt = index.size();
                 cout << cnt << " entries found in cache" << endl
                 << indexcnt << " entries found in index" <<endl;
                 if (cnt > indexcnt)
                    {
                        cout << (cnt - indexcnt) << " entrie(s) missing in index." << endl;
                        for (const auto & entry: fs::directory_iterator(".cache"))
                        {
                            if (!entry.is_regular_file()) continue;
                            if (entry.path().filename() == "index.json") continue;
                            string fname = entry.path().filename().string();

                            auto it = find_if(index.begin(), index.end(),
                                [&](const json& e) { return e.value("file", "") == fname; });
                            if (it == index.end()){
                                cout << "Missing in index: " << fname << endl;
                                auto e = parseFilename(entry.path().stem().string(), sep, entry.path().filename().string());
                                if (!e) {cerr << "Invalid name: " << fname << endl; continue;}

                                index.push_back(*e);
                                cout << "Added to index: " << entry.path().filename().string() << endl;
                            }
                        }
                    }
                 if(cnt < indexcnt)
                 {
                     cout << (indexcnt - cnt) << " entrie(s) missing in cache." << endl;
                     for (auto it = index.begin(); it != index.end(); )
                     {
                         string fname = it->value("file", "");
                         if (fname.empty()) {
                             cerr << "Entry without filename! Deleting index entry." << endl;
                             it = index.erase(it);
                             continue;
                         }

                         if (!fs::exists(".cache/" + fname))
                            {
                                cout << "Missing in cache: " << fname <<endl
                                     << "Getting Lyrics..." <<endl;
                                string title = it->value("title", "");
                                string artist =it->value("artist", "");
                                string lang = it->value("language", "");
                                double duration = it->value("duration", 0.0);
                                if (title.empty() || artist.empty() || duration <= 0.0)
                                {
                                    cerr << "Entry without name, artist or duration! Deleting index entry.";
                                    it = index.erase(it);
                                    continue;
                                }
                                string fname = artist + sep + title + sep + to_string(duration) + sep + lang + ".txt";
                                string text = "";
                                Songinfo *repair=new Songinfo(title,artist,duration);
                                repair->setArr();
                                if (lang == "original")
                                {
                                    text = repair->getRawLyrics();
                                }else{
                                    Translation *rtrans = new Translation(lang, repair);
                                    rtrans->setArr(getSecret("GEMINI_API_KEY"));
                                    text = joinLines(rtrans->getLines());
                                    delete rtrans;
                                }
                                delete repair;
                                ofstream file(".cache/" +fname);

                                if (!file.is_open())
                                   {
                                      cerr << "Error while creating file.\n";
                                      cerr << "errno: " << errno << "\n";
                                      cerr << "Description: "
                                      << strerror(errno) << "\n";
                                      exit(EXIT_FAILURE);
                                    }
                                file << text;
                                cout << "Added to cache: " << fname <<endl;
                                file.close();
                            }
                            ++it;
                    }

                }
                writeIndex();
                cout << "Searching for empty cache entries..." << endl;
                for (const auto & entry: fs::directory_iterator(".cache"))
                {
                    if (!entry.is_regular_file()) continue;
                    if (entry.path().filename() == "index.json") continue;
                    string fname = entry.path().filename().string();
                    ifstream file(entry.path());
                    if (!file.is_open())
                    {
                        cerr << "Error: File: "<< fname << " could not be read."<< endl
                        << "WHY? I don't know." << endl;
                        exit(EXIT_FAILURE);
                    }
                    file.close();
                        if (fs::file_size(entry.path())==0)
                        {
                            cout << fname << " is empty. Getting lyrics..." << endl;
                            auto missing = parseFilename(entry.path().stem().string() ,sep ,fname);
                            if (!missing)
                            {
                                cerr << "Error while creating index entry for: " << fname<< endl;
                                return;
                            }
                            string title = missing->value("title", "");
                            string artist= missing->value("artist", "");
                            double duration=missing->value("duration", 0.0);
                            string lang =missing->value("language", "");
                            string text = "";
                            Songinfo *repair=new Songinfo(title,artist,duration);
                            repair->setArr();
                            if (lang == "original")
                            {
                                text = repair->getRawLyrics();
                            }else{
                                Translation *rtrans = new Translation(lang, repair);
                                rtrans->setArr(getSecret("GEMINI_API_KEY"));
                                text = joinLines(rtrans->getLines());
                                delete rtrans;
                            }
                            delete repair;
                            ofstream file(entry.path());

                            if (!file.is_open())
                            {
                                cerr << "Error while writing file.\n";
                                cerr << "errno: " << errno << "\n";
                                cerr << "Description: "
                                << strerror(errno) << "\n";
                                exit(EXIT_FAILURE);
                            }
                            file << text;
                            cout << "Added to cache: " << fname <<endl;
                            file.close();
                        }


                }
                break;
            }
            default: cout << "Illegal input" << endl;
        }
        return;

}


void Cache::writeIndex()
{   if(!fs::exists(".cache")) {fs::create_directory(".cache");}
    ofstream file(indexPath);
    if (!file.is_open())
    {
        cerr << "Error while writing file.\n";
        cerr << "errno: " << errno << "\n";
        cerr << "Description: "
        << strerror(errno) << "\n";
        exit(EXIT_FAILURE);
    }
    file << index.dump(2);
    cout << "Created or updated indexfile: " << indexPath <<endl;
    file.close();
}
void Cache::addToCache(CacheItem a)
{
    if(!fs::exists(".cache")) {fs::create_directory(".cache");}
    Songinfo *original = a.original;
    Translation* translation = a.translation;
    const fs::path foriginal= ".cache/" + original->getArtist() + sep + original->getName() + sep + to_string(original->getDuration()) + sep + "original" + ".txt";
    const fs::path ftranslation = ".cache/"+original->getArtist() + sep + original->getName() + sep + to_string(original->getDuration()) + sep + translation->getLang() + ".txt";
    auto it = find_if(index.begin(), index.end(), [&](const json& e) { return e.value("file", "") == foriginal.filename().string(); });
    if (it == index.end()){
        auto n = parseFilename(foriginal.stem().string() ,sep ,foriginal.filename().string());
        if (!n)
        {
            cerr << "Error while creating index entry for: " << foriginal.filename().string()<< endl;
            return;
        }
        index.push_back(*n);
    }
    if ((!fs::exists(foriginal) || fs::file_size(foriginal) == 0) && !original->getRawLyrics().empty())
    {
        ofstream file(foriginal);
        if (!file.is_open())
        {
            cerr << "Error while writing file.\n";
            cerr << "errno: " << errno << "\n";
            cerr << "Description: "
            << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }
        file << original->getRawLyrics();
        cout << "Added original lyrics to cache: " <<foriginal.filename().string() <<endl;
        file.close();
    }
    it = find_if(index.begin(), index.end(), [&](const json& e) { return e.value("file", "") == ftranslation.filename().string(); });
    if (it == index.end()){
        auto n = parseFilename(ftranslation.stem().string() ,sep ,ftranslation.filename().string());
        if (!n)
        {
            cerr << "Error while creating index entry for: " << ftranslation.filename().string()<< endl;
            return;
        }
        index.push_back(*n);
    }
    if ((!fs::exists(ftranslation) || fs::file_size(ftranslation) == 0) && !translation->getLines().empty())
    {
        ofstream file(ftranslation);
        if (!file.is_open())
        {
            cerr << "Error while writing file.\n";
            cerr << "errno: " << errno << "\n";
            cerr << "Description: "
            << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }
        file << joinLines(translation->getLines());
        cout << "Added translation for: "<< foriginal.filename().string()<< " to cache: " <<ftranslation.filename().string() <<endl;
        file.close();
    }
    writeIndex();

}
CacheItem Cache::searchCache(string name, string artist, double duration, string lang)
{
    Songinfo *originalsong = nullptr;
    Translation *translation = nullptr;
    bool changed = false;
    auto find = [&](const string &language) {
        return find_if(index.begin(), index.end(), [&](const json& e) {
            return e.value("title", "") == name &&
                   e.value("artist", "") == artist &&
                   e.value("duration", 0.0) == duration &&
                   e.value("language", "") == language;});
    };

    originalsong = new Songinfo(name, artist, duration);
    auto it = find("original");
    if (it == index.end() || !fs::exists(fs::path(".cache") / it->value("file", string{})))
    {
        originalsong->setArr();
        changed = true;
    } else {
        originalsong->ArrFromCache(readFile(fs::path(".cache") / it->value("file", string{})));
        if (originalsong->getLines().empty()) { originalsong->setArr(); changed = true; }
    }

    translation = new Translation(lang, originalsong);
    auto itt = find(lang);
    if (itt == index.end() || !fs::exists(fs::path(".cache") / itt->value("file", string{})))
    {
        translation->setArr(getSecret("GEMINI_API_KEY"));
        changed = true;
    } else {
        translation->ArrFromCache(readFile(fs::path(".cache") / itt->value("file", string{})));
        if (translation->getLines().empty()) { translation->setArr(getSecret("GEMINI_API_KEY")); changed = true; }
    }

    CacheItem found = {originalsong, translation};
    if (changed) addToCache(found);
    return found;
}
