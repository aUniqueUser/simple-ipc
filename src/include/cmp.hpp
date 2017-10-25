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
#include <memory.h>
#include <stdio.h>
#include <stdexcept>

class CatMemoryPool {
public:
    typedef unsigned poolptr_t;
    constexpr static poolptr_t invalid_pool_pointer = poolptr_t(-1);
    struct pool_block {
        bool      free;
        size_t    size;
        poolptr_t prev;
        poolptr_t next;
    };
    struct pool_info {
        size_t bytes_free;
        size_t bytes_alloc;
        size_t bytes_total;
        size_t blocks_free;
        size_t blocks_alloc;
        size_t blocks_total;
    };
public:
    inline
    CatMemoryPool()
    {
    }
    inline
    CatMemoryPool(void *base, size_t size) :
            base_(base), size_(size)
    {
    }
    CatMemoryPool(const CatMemoryPool&) = delete;

    void
    rebase(void *base, size_t size)
    {
        base_ = base;
        size_ = size;
    }

    void
    init()
    {
        memset(base_, 0, size_);
        pool_block zeroth_block;
        zeroth_block.free = true;
        zeroth_block.next = invalid_pool_pointer;
        zeroth_block.prev = invalid_pool_pointer;
        zeroth_block.size = size_;
        memcpy(base_, &zeroth_block, sizeof(pool_block));
    }
    void *
    alloc(size_t size)
    {
        pool_block *block = _find(size);
        if (block == nullptr)
        {
            throw std::bad_alloc();
        }
        _chip(block, size);
        block->free = false;
        return (void*) ((uintptr_t) (block) + sizeof(pool_block));
    }
    void
    free(void *object)
    {
        pool_block *block = (pool_block*) ((uintptr_t) object - sizeof(pool_block));
        block->free = true;
        _mend(block);
    }

    void
    statistics(pool_info& info) const
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
            if (current->next == invalid_pool_pointer)
                break;
            current = real_pointer<pool_block>(current->next);
        }
        info.bytes_free = size_ - info.bytes_alloc;
        info.blocks_free = info.blocks_total - info.blocks_alloc;
        info.bytes_total = size_;
    }

    template<typename T>
        T*
        real_pointer(poolptr_t pointer) const
        {
            return reinterpret_cast<T*>((uintptr_t) base_ + (uintptr_t) pointer);
        }

    template<typename T>
        poolptr_t
        pool_pointer(T* pointer) const
        {
            return (poolptr_t) ((uintptr_t) pointer - (uintptr_t) base_);
        }
protected:
    pool_block *
    _find(size_t size)
    {
        pool_block *current = (pool_block *) base_;
        while (current)
        {
            if (current->free)
            {
                if (current->size >= size)
                    return current;
            }
            if (current->next == invalid_pool_pointer)
                break;
            current = real_pointer<pool_block>(current->next);
        }
        return nullptr;
    }
    void
    _chip(pool_block *block, size_t size)
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
            poolptr_t p_new_block = (pool_pointer<void>(block) + sizeof(pool_block) + block->size);
            if (block->next != invalid_pool_pointer)
            {
                real_pointer<pool_block>(block->next)->prev = p_new_block;
            }
            block->next = p_new_block;
            memcpy(real_pointer<void>(p_new_block), &new_block, sizeof(pool_block));
        }
    }
    void
    _mend(pool_block *block)
    {
        if (block->prev != invalid_pool_pointer)
        {
            pool_block *cur_prev = real_pointer<pool_block>(block->prev);
            if (cur_prev->free)
            {
                _mend(cur_prev);
                return;
            }
        }
        if (block->next != invalid_pool_pointer)
        {
            pool_block *cur_next = real_pointer<pool_block>(block->next);
            while (cur_next->free)
            {
                block->size += sizeof(pool_block) + cur_next->size;
                _delete(cur_next);
                if (block->next != invalid_pool_pointer)
                {
                    cur_next = real_pointer<pool_block>(block->next);
                }
                else
                    break;
            }
        }
    }
    void
    _delete(pool_block *block)
    {
        if (block->next != invalid_pool_pointer)
            real_pointer<pool_block>(block->next)->prev = block->prev;
        if (block->prev != invalid_pool_pointer)
            real_pointer<pool_block>(block->prev)->next = block->next;
    }
protected:
    void *base_ { nullptr };
    size_t size_ { 0 };
};

#endif /* CMP_HPP_ */
