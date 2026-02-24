// log.h
#pragma once

#include <cstdio>

extern bool g_ConsoleOutput;   // 仅声明

#define LOG_INFO(...) do { if (g_ConsoleOutput) { printf("[INFO] "); printf(__VA_ARGS__); printf("\n"); } } while(0)
#define LOG_ERROR(...) do { if (g_ConsoleOutput) { fprintf(stderr, "[ERROR] "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } } while(0)