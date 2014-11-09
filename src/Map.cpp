//
//  Map.cpp
//  AlienLander
//
//  Created by Andrew Morton on 11/7/14.
//
//

#include "Map.h"

void Map::setup(DataSourceRef img)
{
    try {
        mChannel = Channel32f(loadImage(img));
    }
    catch( ... ) {
//        console() << "unable to load the texture file!" << std::endl;
        // fall back to perlin noise
        Channel32f::Iter iter = mChannel.getIter();
        while( iter.line() ) {
            while( iter.pixel() ) {
                float noise = (0.5 + mPerlin.fBm(iter.x() * 0.1, iter.y() * 0.1, 0.52));
                iter.v() = noise;
            }
        }
    };

}