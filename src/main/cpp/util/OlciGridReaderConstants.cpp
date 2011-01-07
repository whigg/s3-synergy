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
 * File:   ReaderConstants.cpp
 * Author: thomass
 * 
 * Created on January 3, 2011, 1:57 PM
 */

#include "OlciGridReaderConstants.h"

OlciGridReaderConstants::OlciGridReaderConstants() {
}

OlciGridReaderConstants::~OlciGridReaderConstants() {
}

const string OlciGridReaderConstants::L_1 = "L_1";
const string OlciGridReaderConstants::L_2 = "L_2";
const string OlciGridReaderConstants::L_3 = "L_3";
const string OlciGridReaderConstants::L_4 = "L_4";
const string OlciGridReaderConstants::L_5 = "L_5";
const string OlciGridReaderConstants::L_6 = "L_6";
const string OlciGridReaderConstants::L_7 = "L_7";
const string OlciGridReaderConstants::L_8 = "L_8";
const string OlciGridReaderConstants::L_9 = "L_9";
const string OlciGridReaderConstants::L_10= "L_10";
const string OlciGridReaderConstants::L_11 = "L_11";
const string OlciGridReaderConstants::L_12 = "L_12";
const string OlciGridReaderConstants::L_13 = "L_13";
const string OlciGridReaderConstants::L_14 = "L_14";
const string OlciGridReaderConstants::L_15 = "L_15";
const string OlciGridReaderConstants::L_16 = "L_16";
const string OlciGridReaderConstants::L_17 = "L_17";
const string OlciGridReaderConstants::L_18 = "L_18";
const string OlciGridReaderConstants::L_19 = "L_19";
const string OlciGridReaderConstants::L_20 = "L_20";
const string OlciGridReaderConstants::L_21 = "L_21";
const string OlciGridReaderConstants::L_22 = "L_22";
const string OlciGridReaderConstants::L_23 = "L_23";
const string OlciGridReaderConstants::L_24 = "L_24";
const string OlciGridReaderConstants::L_25 = "L_25";
const string OlciGridReaderConstants::L_26 = "L_26";
const string OlciGridReaderConstants::L_27 = "L_27";
const string OlciGridReaderConstants::L_28 = "L_28";
const string OlciGridReaderConstants::L_29 = "L_29";
const string OlciGridReaderConstants::L_30 = "L_30";

const vector<string> OlciGridReaderConstants::getVariablesToRead() {
    /*
        Important note:
            The first variable to be added here defines the OLCI-grid!
     */
    vector<string> varsToRead;
    varsToRead.push_back(L_1);
//    varsToRead.push_back(L_2);
//    varsToRead.push_back(L_3);
//    varsToRead.push_back(L_4);
//    varsToRead.push_back(L_5);
//    varsToRead.push_back(L_6);
//    varsToRead.push_back(L_7);
//    varsToRead.push_back(L_8);
//    varsToRead.push_back(L_9);
//    varsToRead.push_back(L_10);
//    varsToRead.push_back(L_11);
//    varsToRead.push_back(L_12);
//    varsToRead.push_back(L_13);
//    varsToRead.push_back(L_14);
//    varsToRead.push_back(L_15);
//    varsToRead.push_back(L_16);
//    varsToRead.push_back(L_17);
//    varsToRead.push_back(L_18);
//    varsToRead.push_back(L_19);
//    varsToRead.push_back(L_20);
//    varsToRead.push_back(L_21);
//    varsToRead.push_back(L_22);
//    varsToRead.push_back(L_23);
//    varsToRead.push_back(L_24);
//    varsToRead.push_back(L_25);
//    varsToRead.push_back(L_26);
//    varsToRead.push_back(L_27);
//    varsToRead.push_back(L_28);
//    varsToRead.push_back(L_29);
//    varsToRead.push_back(L_30);
    return varsToRead;
}