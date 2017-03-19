/*
 * cmp.hpp
 *
 *  Created on: Mar 18, 2017
 *      Author: nullifiedcat
 */

#ifndef CMP_HPP_
#define CMP_HPP_

#include <stddef.h>
#include <stdint.h>

class CatMemoryPool {
public:
	CatMemoryPool(void* base, size_t size);

	struct pool_block_s {
		bool free;
		size_t size;
		void* prev;
		void* next;
	};

	struct pool_info_s {
		unsigned long free;
		unsigned long alloc;
		unsigned freeblk;
		unsigned allocblk;
		unsigned blkcnt;
	};

	void  init();
	void* alloc(size_t size);
	void  free(void*);

	void statistics(pool_info_s& info);

	void print();

	template<typename T>
	T* real_pointer(void* pointer) const { return reinterpret_cast<T*>((uintptr_t)base + (uintptr_t)pointer); }

	template<typename T>
	void* pool_pointer(T* pointer) const { return (void*)((uintptr_t)pointer - (uintptr_t)base); }

	void* base;
	const size_t size;
protected:
	pool_block_s* FindBlock(size_t size);
	void ChipBlock(pool_block_s* block, size_t size);
	void MendBlock(pool_block_s*);
	void DeleteBlock(pool_block_s* block);
};

#endif /* CMP_HPP_ */
