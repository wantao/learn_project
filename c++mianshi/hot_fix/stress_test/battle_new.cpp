#include <iostream>

extern "C" int calc_damage(int hp, int atk) {
    int def = 5;
    int real = atk - def;
    std::cout <<" new_so calc_damage"<<std::endl;
    return real > 0 ? hp - real : hp;
}