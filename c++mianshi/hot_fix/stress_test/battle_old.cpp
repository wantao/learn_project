#include <iostream>

extern "C" int calc_damage(int hp, int atk) {
    std::cout <<" old_so calc_damage"<<std::endl;
    return hp - atk;
}