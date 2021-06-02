#pragma once
#include <Core/System.h>
#include <GfxCore/GfxModuleSettings.h>
#include <fstream>

#define DEBUG_WINDOW_ENABLE 1
struct CommandList;
using ColorEditFlags = s32;

constexpr char DEBUG_WINDOW_STRUCTURE_FOLDER_PATH[] = "L:/LightnEngine/LightnEngineDemo/__Intermediate/debug_window/";
constexpr char DEBUG_WINDOW_STRUCTURE_EXT[] = ".dws";
template <typename T>
struct DebugWindowStructure :public T {
#if DEBUG_WINDOW_ENABLE
	DebugWindowStructure(const char* fileName) :_fileName(fileName) {
		char fullPath[256] = {};
		sprintf_s(fullPath, "%s%s%s", DEBUG_WINDOW_STRUCTURE_FOLDER_PATH, fileName, DEBUG_WINDOW_STRUCTURE_EXT);
		std::ifstream fin;
		fin.open(fullPath, std::ios::in | std::ios::binary);

		// ファイルサイズ取得
		fin.seekg(0, std::ios_base::end);
		s32 fileSize = static_cast<s32>(fin.tellg());
		fin.seekg(0, std::ios_base::beg);

		if(fin.fail()) {
			fin.open(fullPath, std::ios_base::app);
			T initData = T();
			memcpy(reinterpret_cast<T*>(this), &initData, sizeof(T));
		} else {
			fin.exceptions(std::ios::badbit);
			fin.read(reinterpret_cast<char*>(this), fileSize);
		}

		LTN_ASSERT(!fin.fail());
		fin.close();

		_data = static_cast<T>(*this);
	}

	~DebugWindowStructure() {
		if (!memcmp(&_data, static_cast<const T*>(this), sizeof(T))) {
			return;
		}

		char fullPath[256] = {};
		sprintf_s(fullPath, "%s%s%s", DEBUG_WINDOW_STRUCTURE_FOLDER_PATH, _fileName, DEBUG_WINDOW_STRUCTURE_EXT);
		std::ofstream fout;
		fout.open(fullPath, std::ios::out | std::ios::binary | std::ios::trunc);

		LTN_ASSERT(!fout.fail());
		fout.write(reinterpret_cast<char*>(this), sizeof(T));
		fout.close();
	}

	T _data;
	const char* _fileName = nullptr;
#else
	DebugWindowStructure(const char* fileName) {}
#endif
};

class LTN_GFX_CORE_API DebugWindow :private NonCopyable{
public:
	static void initialize();
	static void terminate();
	static void beginFrame();
	static void renderFrame(CommandList* commandList);

	static void StartWindow(const char* windowName) {
#if DEBUG_WINDOW_ENABLE
		Start(windowName);
#endif
	}

	template <typename U>
	static DebugWindowStructure<U> StartWindow(const char* windowName) {
		StartWindow(windowName);
		return DebugWindowStructure<U>(windowName);
	}
	static void End();

private:
	static void Start(const char* windowName);
};
