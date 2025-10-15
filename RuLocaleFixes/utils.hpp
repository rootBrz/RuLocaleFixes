#pragma once

#include "DbgSymbols.hpp"
#include <cstdio>
#include <string>

unsigned int TranslateKeyToCP1251(unsigned int key);
void ConvertStdStringUtf8ToCp1251(std::string &strPtr);
int __cdecl FastToUpper(int c);
int __cdecl FastToLower(int c);
void ResolveAndHook(DbgSymbols &resolver, const char *funcName, void *hook, void **trump);

inline void *instance = nullptr;

class Logger
{
  private:
    FILE *logFileHandle = nullptr;

    Logger(const char *logFile)
    {
        if (fopen_s(&logFileHandle, logFile, "w") != 0 || !logFileHandle)
            fprintf(stderr, "ERROR: Could not open log file %s\n", logFile);
    }

    ~Logger()
    {
        if (logFileHandle) fclose(logFileHandle);
    }

  public:
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    static Logger &GetInstance()
    {
        if (!instance) instance = new Logger("RuLocaleFixes.log");
        return *reinterpret_cast<Logger *>(instance);
    }

    static void DestroyInstance()
    {
        if (instance)
        {
            delete reinterpret_cast<Logger *>(instance);
            instance = nullptr;
        }
    }

    void Log(const char *fmt, ...)
    {
        if (!logFileHandle) return;

        va_list args;

        va_start(args, fmt);
        vfprintf(logFileHandle, fmt, args);
        va_end(args);
        fflush(logFileHandle);

        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        fflush(stdout);
    }
};
