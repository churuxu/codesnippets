#pragma once


/** �������ʽ hash���� */

constexpr size_t HASH(const char* str, size_t last = 0) {
	return *str ? HASH(str + 1, (last * 31 + *str)) : last;
}


