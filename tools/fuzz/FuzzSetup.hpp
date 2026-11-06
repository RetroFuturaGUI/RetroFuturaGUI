#pragma once
#include <cstdlib>

#if defined(_MSC_VER)
#include <crtdbg.h>
#include <cstdio>
#include <cwchar>
#include <stdlib.h>
#endif

//Included by every fuzz target only once. A Debug build's standard library checks ranges itself (a subspan past the end, an index
//out of range) and fails too fast for libFuzzer to keep the input. Such a failure says what it was and aborts, which libFuzzer catches and saves as a crash file.
extern "C" int LLVMFuzzerInitialize(int*, char***)
{
#if defined(_MSC_VER)
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _set_invalid_parameter_handler([](const wchar_t* expression, const wchar_t* function, const wchar_t* file, uint32_t line, uintptr_t)
        {
            std::fwprintf(stderr, L"invalid parameter: %ls in %ls (%ls:%u)\n", expression ? expression : L"?", function ? function : L"?", file ? file : L"?", line);
            std::abort();
        });
#endif
    return 0;
}