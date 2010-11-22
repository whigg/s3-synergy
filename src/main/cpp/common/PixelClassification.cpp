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
 * File:   PixelClassification.cpp
 * Author: thomass
 * 
 * Created on November 10, 2010, 4:26 PM
 */

#include "PixelClassification.h"

PixelClassification::PixelClassification() : Module() {
}

PixelClassification::~PixelClassification() {
}

void PixelClassification::start() {

}

void PixelClassification::stop() {

}

Segment* PixelClassification::processSegment(ProcessorContext& context) {
    Segment* source = context.getSource("ID");
    for (size_t k = 0; k < source->getMaxK(); k++) {
        // TODO - parallelize
        for (size_t l = 0; l < source->getMaxL(); l++) {
            for (size_t m = 0; m < source->getMaxM(); m++) {
                const size_t pos = m + l * source->getMaxM() + k * source->getMaxL();
                int olcFlags = source->getSampleInt("F_OLC", pos);
                int slnFlags = source->getSampleInt("F_SLN", pos);
                int sloFlags = source->getSampleInt("F_SLO", pos);
                bool olcLand = (olcFlags & 0x1000) != 0;
                bool slnLand = (slnFlags & 0x0800) != 0;
                bool sloLand = (sloFlags & 0x0800) != 0;
                bool slnCloud = (slnFlags & 0x00400000) != 0;
                bool sloCloud = (sloFlags & 0x00400000) != 0;
                // TODO - OLCI L2 Pixel classification
                int synFlags = 0;
                if (olcLand && slnLand && sloLand) {
                    synFlags |= 0x0020;
                }
                if (slnCloud || sloCloud) {
                    synFlags |= 0x0001;
                }
                source->setSampleInt("F_SYN", pos, synFlags);
            }
        }
    }
    return source;
}