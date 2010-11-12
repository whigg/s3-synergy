/*
 * Copyright (C) 2010 by Brockmann Consult (info@brockmann-consult.de)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation. This program is distributed in the hope it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * File:   SegmentImpl.h
 * Author: thomass
 *
 * Created on November 11, 2010, 10:04 AM
 */

#ifndef SEGMENTIMPL_H
#define	SEGMENTIMPL_H

#include <valarray>
#include <map>
#include "Segment.h"
#include "PixelImpl.h"

class SegmentImpl : public Segment {
public:
    SegmentImpl(int xPos, int yPos, int width, int height) {
        this->xPos = xPos;
        this->yPos = yPos;
        this->width = width;
        this->height = height;
        numValues = width * height;
    }

//    template<class T> void getSamples(const std::string& varName, std::valarray<T> samples);
//    template<class T> void setSamples(const std::string& varName, std::valarray<T> samples);

    ~SegmentImpl();

    void getSamplesInt(const std::string& varName, std::vector<int>& samples);
    void setSamplesInt(const std::string& varName, std::vector<int>& samples);

    void addIntVariable(const std::string& varName);
    void remove(const std::string& varName);
    Pixel* getPixel(int k, int l, int m, Pixel* pixel);

private:
    int xPos;
    int yPos;
    int width;
    int height;
    int numValues;
    const static int NO_DATA_VALUE = -999;
    std::map<std::string, void* > dataBufferMap;
    int computeArrayPosition( int k, int l, int m );
};

#endif	/* SEGMENTIMPL_H */

