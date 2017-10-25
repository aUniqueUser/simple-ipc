#include "xshmem.hpp"
#include "xshmutex.hpp"

#include <stdio.h>

int main()
{
    printf("XShMem + XShMutex demonstration\n");
    xshmutex::xshmutex mtx("xshm_demo");
    xshmem::xshmem mem("xshm_demo_mem", true);
}