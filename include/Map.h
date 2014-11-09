//
//  Map.h
//  AlienLander
//
//  Created by Andrew Morton on 11/7/14.
//
//

#ifndef __AlienLander__Map__
#define __AlienLander__Map__

#include "cinder/Perlin.h"
#include "Resources.h"

using namespace ci;
using namespace std;

class Map {
public:
    void setup(DataSourceRef img);
    void update();

    Perlin mPerlin = Perlin(16);
    Channel32f mChannel = Channel32f(1024, 1024);
};

#endif /* defined(__AlienLander__Map__) */
