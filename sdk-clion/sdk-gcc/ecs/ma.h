//
// Created by ruiy on 18-3-18.
//

#ifndef SDK_GCC_MA_H
#define SDK_GCC_MA_H

#include "type_def.h"
#include <vector>

std::vector<Double> ma(std::vector<Double> data, int window_size, int move_size=1);

#endif //SDK_GCC_MA_H
