#pragma once

#include <iostream>

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#else
    #include <clocale>
    #include <cstring>
    #include <langinfo.h>
#endif

inline void utfcheck()
{
#ifdef _WIN32
    if (GetConsoleOutputCP() == CP_UTF8)
        return;

    std::cerr << "Warning: Your Terminal doesn't use UTF-8. (active Codepage: "
              << GetConsoleOutputCP() << ").\n"
              << "Special characters will be displayed incorectly.\n"
              << "Run 'chcp 65001' in your terminal.\n";
#else
    std::setlocale(LC_ALL, "");
    std::setlocale(LC_NUMERIC, "C");
    const char* codeset = nl_langinfo(CODESET);

    if (codeset && (std::strcmp(codeset, "UTF-8") == 0 ||
                    std::strcmp(codeset, "utf8") == 0))
        return;

    std::cerr << "Warning: Your Terminal doesn't use UTF-8. Your Codeset: "
              << (codeset ? codeset : "unknown") << ").\n"
              << "Special characters will be displayed incorectly.\n"
              << "Run 'export LANG=de_DE.UTF-8' in your terminal.\n";
#endif
}
