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
    rebase(void *base, size_t size);

    void
    init();
    void *
    alloc(size_t size);
    void
    free(void *block);

    void
    statistics(pool_info& info) const;

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
    _find(size_t size);
    void
    _chip(pool_block *block, size_t size);
    void
    _mend(pool_block *block);
    void
    _delete(pool_block *block);
protected:
    void *base_ { nullptr };
    size_t size_ { 0 };
};

#endif /* CMP_HPP_ */
