/* 
 * Copyright (C) 2011 by Brockmann Consult (info@brockmann-consult.de)
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * File:   NullLogging.cpp
 * Author: ralf
 * 
 * Created on January 19, 2011, 3:40 PM
 */

#include "NullLogging.h"

NullLogging::NullLogging() : Logging() {
}

NullLogging::NullLogging(const NullLogging& orig) : Logging() {
}

NullLogging::~NullLogging() {
}

shared_ptr<Logging> NullLogging::instance = shared_ptr<Logging>(new NullLogging());
