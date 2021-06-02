#pragma once

// gfx framework
#ifdef LTN_GFX_FRAMEWORK_EXPORT
#define LTN_GFX_FRAMEWORK_API __declspec(dllexport)
#else
#define LTN_GFX_FRAMEWORK_API __declspec(dllimport)
#endif

// gfx core
#ifdef LTN_GFX_CORE_EXPORT
#define LTN_GFX_CORE_API __declspec(dllexport)
#else
#define LTN_GFX_CORE_API __declspec(dllimport)
#endif

// d3d12 api
#ifdef LTN_GFX_API_EXPORT
#define LTN_GFX_API __declspec(dllexport)
#else
#define LTN_GFX_API __declspec(dllimport)
#endif

// mesh renderer
#ifdef LTN_MESH_RENDERER_EXPORT
#define LTN_MESH_RENDERER_API __declspec(dllexport)
#else
#define LTN_MESH_RENDERER_API __declspec(dllimport)
#endif