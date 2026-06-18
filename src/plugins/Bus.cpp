#include "Bus.h"

Bus *Bus::Instance()
{
    static Bus instance;
    return &instance;
}