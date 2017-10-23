#pragma once

#include <string>

// Cross-Platform Shared Memory

namespace xshmem
{

class xshmem
{
public:
    xshmem(std::string name, unsigned length);
    ~xshmem();
    
    int  init();
    void destroy();
    int  connect();
    
    void *get();
protected:
    void *data_ { nullptr };
    std::string name_;
    unsigned size_;
};

}