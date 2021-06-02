#pragma once

// core
#ifdef LTN_CORE_EXPORT
#define LTN_CORE_API __declspec(dllexport)
#else
#define LTN_CORE_API __declspec(dllimport)
#endif

// application
#ifdef LTN_APPLICATION_EXPORT
#define LTN_APP_API __declspec(dllexport)
#else
#define LTN_APP_API __declspec(dllimport)
#endif