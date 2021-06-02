#ifdef LTN_TEXTURE_SYSTEM_EXPORT
#define LTN_TEXTURE_SYSTEM_API __declspec(dllexport)
#else
#define LTN_TEXTURE_SYSTEM_API __declspec(dllimport)
#endif