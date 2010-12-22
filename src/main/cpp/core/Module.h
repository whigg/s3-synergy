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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * File:   Module.h
 * Author: ralf
 *
 * Created on December 21, 2010, 10:24 AM
 */

#ifndef MODULE_H
#define	MODULE_H

#include "Context.h"

/**
 * Interface representing a processing module.
 */
class Module {
public:

    /**
     * Destructor.
     */
    virtual ~Module() {
    };

    /**
     * Returns the module ID.
     * @return the module ID.
     */
    virtual const string& getId() const = 0;

    /**
     * Returns the module version.
     * @return the module version.
     */
    virtual const string& getVersion() const = 0;

    /**
     * Acquires and initializes resources the module needs for processing.
     * Resources may be added to the context as objects using the
     * {@code context.addObject()} method.
     * @param context The context of the module.
     */
    virtual void start(Context& context) = 0;

    /**
     * Removes objects this module has added to the context and releases
     * resources this module has acquired.
     * @param context The context of the module.
     */
    virtual void stop(Context& context) = 0;

    /**
     * Processes segment data, which are provided by a context. The contract
     * to be satisfies by implementing modules is:
     *
     * 1. The implementing module must use the context to create segment data
     * 2. The implementing module shall use the context to determine in which
     *    row the processing of segment data has to be started
     * 3. The implementing modules shall carry out the processing of segment
     *    data
     * 4. The implementing module shall communicate to the context up to which
     *    row the processing of segment data has been completed
     *
     * @param context The context.
     */
    virtual void process(Context& context) = 0;

    /**
     * Returns the index of the minimum row, which is required for processing
     * a certain block of rows.
     * @param l The row index of the first row in the block of rows to be
     *          processed.
     * @return the index of the minimum row required for processing a block
     *         of rows the first row of which has the row index {@code l}.
     */
    virtual size_t getMinLRequired(size_t l) const = 0;
};

#endif	/* MODULE_H */
