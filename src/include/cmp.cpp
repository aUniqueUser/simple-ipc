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

void
CatMemoryPool::statistics(pool_info& info) const
{
    memset(&info, 0, sizeof(pool_info));
    pool_block *current = (pool_block *) base_;
    while (current)
    {
        if (!current->free)
        {
            info.blocks_alloc++;
            info.bytes_alloc += current->size;
        }
        info.bytes_alloc += sizeof(pool_block);
        info.blocks_total++;
        if (current->next == (void *) -1)
            break;
        current = real_pointer<pool_block>(current->next);
    }
    info.bytes_free = size_ - info.bytes_alloc;
    info.blocks_free = info.blocks_total - info.blocks_alloc;
    info.bytes_total = size_;
}

void
CatMemoryPool::rebase(void *base, size_t size)
{
    base_ = base;
    size_ = size;
}

void
CatMemoryPool::init()
{
    memset(base_, 0, size_);
    pool_block zeroth_block;
    zeroth_block.free = true;
    zeroth_block.next = (void *) -1;
    zeroth_block.prev = (void *) -1;
    zeroth_block.size = size_;
    memcpy(base_, &zeroth_block, sizeof(pool_block));
}

CatMemoryPool::pool_block*
CatMemoryPool::_find(size_t size)
{
    pool_block *current = (pool_block *) base_;
    while (current)
    {
        if (current->free)
        {
            if (current->size >= size)
                return current;
        }
        if (current->next == (void *) -1)
            break;
        current = real_pointer<pool_block>(current->next);
    }
    return nullptr;
}

void *
CatMemoryPool::alloc(size_t size)
{
    pool_block *block = _find(size);
    if (block == nullptr)
    {
        return nullptr;
    }
    _chip(block, size);
    block->free = false;
    return (void*) ((uintptr_t) (block) + sizeof(pool_block));
}

void
CatMemoryPool::free(void *object)
{
    pool_block *block = (pool_block*) ((uintptr_t) object - sizeof(pool_block));
    block->free = true;
    _mend(block);
}

void
CatMemoryPool::_chip(pool_block *block, size_t size)
{
    if (block->size - sizeof(pool_block) > size)
    {
        unsigned old_size = block->size;
        block->size = size;
        pool_block new_block;
        new_block.prev = pool_pointer<void>(block);
        new_block.next = block->next;
        new_block.free = true;
        new_block.size = old_size - (size + sizeof(pool_block));
        void *p_new_block = (void*) ((unsigned) pool_pointer<void>(block)
                + sizeof(pool_block) + block->size);
        if (block->next != (void *) -1)
        {
            real_pointer<pool_block>(block->next)->prev = p_new_block;
        }
        block->next = p_new_block;
        memcpy(real_pointer<void>(p_new_block), &new_block, sizeof(pool_block));
    }
}

void
CatMemoryPool::_mend(pool_block* block)
{
    if (block->prev != (void *) -1)
    {
        pool_block *cur_prev = real_pointer<pool_block>(block->prev);
        if (cur_prev->free)
        {
            _mend(cur_prev);
            return;
        }
    }
    if (block->next != (void *) -1)
    {
        pool_block *cur_next = real_pointer<pool_block>(block->next);
        while (cur_next->free)
        {
            block->size += sizeof(pool_block) + cur_next->size;
            _delete(cur_next);
            if (block->next != (void *) -1)
            {
                cur_next = real_pointer<pool_block>(block->next);
            }
            else
                break;
        }
    }
}

void
CatMemoryPool::_delete(pool_block* block)
{
    if (block->next != (void *) -1)
        real_pointer<pool_block>(block->next)->prev = block->prev;
    if (block->prev != (void *) -1)
        real_pointer<pool_block>(block->prev)->next = block->next;
}
