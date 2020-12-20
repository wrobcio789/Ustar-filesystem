#include "ustar_base.h"

uint64_t oct2bin(char* string){
    uint64_t result = 0;
    while(string){
        result *= 8;
        result += (*string) - '0';
        string++;
    }
    return result;
}