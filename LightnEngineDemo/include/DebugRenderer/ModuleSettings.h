#ifdef LTN_DEBUG_RENDERER_EXPORT
#define LTN_DEBUG_RENDERER_API __declspec(dllexport)
#else
#define LTN_DEBUG_RENDERER_API __declspec(dllimport)
#endif