#ifdef LTN_MATERIAL_SYSTEM_EXPORT
#define LTN_MATERIAL_SYSTEM_API __declspec(dllexport)
#else
#define LTN_MATERIAL_SYSTEM_API __declspec(dllimport)
#endif