/*
 * cmp.cpp
 *
 *  Created on: Mar 18, 2017
 *      Author: nullifiedcat
 */

#include "cmp.hpp"

#include <stdint.h>
#include <memory.h>
#include <stdio.h>

CatMemoryPool::CatMemoryPool(void* base, size_t size) : base(base), size(size) {
	printf("creating memory pool with size 0x%08x at 0x%08x\n", size, base);
}


void CatMemoryPool::statistics(pool_info_s& info) {
	memset(&info, 0, sizeof(pool_info_s));
	pool_block_s* current = (pool_block_s*)base;
	while (current) {
		if (current->free) {
			info.freeblk++;
			info.free += current->size;
		}
		info.blkcnt++;
		if (current->next == (void*)-1) break;
		current = real_pointer<pool_block_s>(current->next);
	}
	info.alloc = size - info.free;
	info.allocblk = info.blkcnt - info.freeblk;
}

void CatMemoryPool::init() {
	memset(base, 0, size);
	pool_block_s zeroth_block;
	zeroth_block.free = true;
	zeroth_block.next = (pool_block_s*)-1;
	zeroth_block.prev = (pool_block_s*)-1;
	zeroth_block.size = size;
	memcpy(base, &zeroth_block, sizeof(pool_block_s));
}

CatMemoryPool::pool_block_s* CatMemoryPool::FindBlock(size_t size) {
	pool_block_s* current = (pool_block_s*)base;
	while (current) {
		if (current->free) {
			if (current->size >= size) return current;
		}
		if (current->next == (void*)-1) break;
		current = real_pointer<pool_block_s>(current->next);
	}
	return (pool_block_s*)-1;
}

void* CatMemoryPool::alloc(size_t size) {
	pool_block_s* block = FindBlock(size);
	if (block == (pool_block_s*)-1) {
		printf("not enough memory to allocate block of size 0x%08x\n", size);
		return (void*)0;
	}
	printf("allocating block of size 0x%08x at 0x%08x\n", size, pool_pointer<void>(block));
	ChipBlock(block, size);
	block->free = false;
	return (void*)((uintptr_t)(block) + sizeof(pool_block_s));
}

void CatMemoryPool::free(void* object) {
	pool_block_s* block = (pool_block_s*)((uintptr_t)object - sizeof(pool_block_s));
	printf("freeing block of size 0x%08x at 0x%08x\n", block->size, pool_pointer<void>(block));
	block->free = true;
	MendBlock(block);
}

void CatMemoryPool::ChipBlock(pool_block_s* block, size_t size) {
	if (block->size - sizeof(pool_block_s) > size) {
		unsigned old_size = block->size;
		block->size = size;
		pool_block_s new_block;
		new_block.prev = pool_pointer<void>(block);
		new_block.next = block->next;
		new_block.free = 1;
		new_block.size = old_size - (size + sizeof(pool_block_s));
		void* p_new_block = (void*)(pool_pointer<void>(block) + sizeof(pool_block_s) + block->size);
		if (block->next != (void*)-1) {
					real_pointer<pool_block_s>(block->next)->prev = p_new_block;
		}
		block->next = p_new_block;
		printf("chipping block at 0x%08x with old size 0x%08x, made new block at 0x%08x with size 0x%08x\n", pool_pointer<void>(block), old_size, p_new_block, new_block.size);
		memcpy(real_pointer<void>(p_new_block), &new_block, sizeof(pool_block_s));
	}
}

void CatMemoryPool::print() {
	pool_block_s* current = (pool_block_s*)base;
	while (true) {
		printf("] memory block at 0x%08x (0x%08x) size 0x%08x next 0x%08x prev 0x%08x free? %i. \n", pool_pointer<void>(current), current, current->size, current->next, current->prev, current->free);
		if (current->next == (void*)-1) break;
		current = real_pointer<pool_block_s>(current->next);
	}
}

void CatMemoryPool::MendBlock(pool_block_s* block) {
	if (block->next == (void*)-1) return;
	pool_block_s* cur_next = real_pointer<pool_block_s>(block->next);
	if (cur_next->free) {
		block->size += sizeof(pool_block_s) + cur_next->size;
		printf("mended block at 0x%08x new size 0x%08x\n", pool_pointer<void>(block), block->size);
		DeleteBlock(cur_next);
		if (block->prev != (void*)-1) {
			pool_block_s* cur_prev = real_pointer<pool_block_s>(block->prev);
			if (cur_prev->free) MendBlock(cur_prev);
		}
	}
}

void CatMemoryPool::DeleteBlock(pool_block_s* block) {
	printf("deleted block at 0x%08x size 0x%08x\n", pool_pointer<void>(block), block->size);
	if (block->next != (void*)-1) real_pointer<pool_block_s>(block->next)->prev = block->prev;
	if (block->prev != (void*)-1) real_pointer<pool_block_s>(block->prev)->next = block->next;
}
