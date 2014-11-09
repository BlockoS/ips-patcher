#include <iostream>
#include "ips.h"

int main()
{
    IPS::Patch patch;
    bool ret;
    ret = patch.add(IPS::Record(5, 4, (uint8_t)0));
    if(false == ret) { std::cerr << "A" << std::endl; }

    ret = patch.add(IPS::Record(1, 2, (uint8_t)0));
    if(false == ret) { std::cerr << "B" << std::endl; }

    ret = patch.add(IPS::Record(11, 3, (uint8_t)0));
    if(false == ret) { std::cerr << "C" << std::endl; }

    ret = patch.add(IPS::Record(6, 1, (uint8_t)0));
    if(true == ret) { std::cerr << "D" << std::endl; }

    ret = patch.add(IPS::Record(3, 8, (uint8_t)0));
    if(true == ret) { std::cerr << "E" << std::endl; }

    ret = patch.add(IPS::Record(8, 21, (uint8_t)0));
    if(true == ret) { std::cerr << "F" << std::endl; }

    ret = patch.remove(1);

    return 0;
}
