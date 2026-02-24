// log.hpp
// Simple logging macros for FileMgr application
// 
// Provides INFO and ERROR logging macros that output to console only when
// g_ConsoleOutput flag is enabled (via --console command line argument).
// 
// Usage:
//   LOG_INFO("Loading file: %s", filename);
//   LOG_ERROR("Failed to open: %s", path);
// 
#pragma once

#include <cstdio>

// Global flag to enable/disable console output
extern bool g_ConsoleOutput;

// Log an informational message to stdout
#define LOG_INFO(...) do { \
    if (g_ConsoleOutput) { \
        printf("[INFO] "); \
        printf(__VA_ARGS__); \
        printf("\n"); \
    } \
} while(0)

// Log an error message to stderr  
#define LOG_ERROR(...) do { \
    if (g_ConsoleOutput) { \
        fprintf(stderr, "[ERROR] "); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } \
} while(0)
