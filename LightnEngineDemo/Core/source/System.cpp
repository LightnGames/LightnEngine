#include <Core/System.h>
#include "../third_party/XxHash/xxhash.h"

u32 StrLength(const char* str) {
	u32 result = 0;
	while (*(str + result) != '\0') {
		result++;
	}
	return result;
}
u64 StrHash(const char* str) {
	return XXH64(str, StrLength(str), 0);
}

u32 StrHash32(const char* str) {
	return XXH32(str, StrLength(str), 0);
}

u64 BinHash(const void* bin, u32 length) {
	return XXH64(bin, length, 0);
}
