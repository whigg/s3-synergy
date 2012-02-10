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
 * File:   Constants.h
 * Author: Ralf Quast
 *
 * Created on December 15, 2010, 5:39 PM
 */

#ifndef CONSTANTS_H
#define	CONSTANTS_H

#include <boost/cstdint.hpp>
#include <limits>
#include <string>

using std::numeric_limits;
using std::string;

using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;

namespace Constants {

#include "Config.h"

/**
 * Product types.
 */
const string PRODUCT_SY1 = "SY1";
const string PRODUCT_SY2 = "SY2";
const string PRODUCT_VGP = "VGP";
const string PRODUCT_VGS = "VGS";

/**
 * Segment names.
 */
const string SEGMENT_GEO = "GEO";
const string SEGMENT_OLC = "OLC";
const string SEGMENT_OLC_TP = "OLC_TP";
const string SEGMENT_OLC_INFO = "OLC_INFO";
const string SEGMENT_OLC_TIME = "OLC_TIME";
const string SEGMENT_SLN = "SLN";
const string SEGMENT_SLO = "SLO";
const string SEGMENT_SLN_TP = "SLN_TP";
const string SEGMENT_SLO_TP = "SLO_TP";
const string SEGMENT_SLN_INFO = "SLN_INFO";
const string SEGMENT_SLO_INFO = "SLO_INFO";
const string SEGMENT_SYN_COLLOCATED = "SYN_COLLOCATED";
const string SEGMENT_SYN_AVERAGED = "SYN_AVERAGED";

const string SEGMENT_VGT = "VGT";
const string SEGMENT_VGT_TP = "VGT_TP";

const string SEGMENT_VGT_LAT = "VGT_LAT";
const string SEGMENT_VGT_LAT_TP = "VGT_LAT_TP";
const string SEGMENT_VGT_LON = "VGT_LON";
const string SEGMENT_VGT_LON_TP = "VGT_LON_TP";

const string SEGMENT_VGT_LAT_BNDS = "VGT_LAT_BNDS";
const string SEGMENT_VGT_LON_BNDS = "VGT_LON_BNDS";
const string SEGMENT_VGT_LAT_TP_BNDS = "VGT_LAT_TP_BNDS";
const string SEGMENT_VGT_LON_TP_BNDS = "VGT_LON_TP_BNDS";

/**
 * Dimension names.
 */
const string DIMENSION_N_CAM = "N_CAM";
const string DIMENSION_N_DET_CAM = "N_DET_CAM";
const string DIMENSION_N_LINE_OLC = "N_LINE_OLC";

/**
 * Dimension sizes.
 */
const long N_CAM = 5;
const long N_DET_CAM = 740;

/**
 * Data types.
 */
const int TYPE_BYTE = 1;
const int TYPE_CHAR = 2;
const int TYPE_SHORT = 3;
const int TYPE_INT = 4;
const int TYPE_FLOAT = 5;
const int TYPE_DOUBLE = 6;
const int TYPE_UBYTE = 7;
const int TYPE_USHORT = 8;
const int TYPE_UINT = 9;
const int TYPE_LONG = 10;
const int TYPE_ULONG = 11;
const int TYPE_STRING = 12;

/**
 * Log levels
 */
const string LOG_LEVEL_INFO = "INFO";
const string LOG_LEVEL_PROGRESS = "PROGRESS";
const string LOG_LEVEL_DEBUG = "DEBUG";
const string LOG_LEVEL_WARNING = "WARNING";
const string LOG_LEVEL_ERROR = "ERROR";

/**
 * Auxiliary data IDs.
 */
const string AUX_ID_SYCPAX = "SYCPAX";
const string AUX_ID_SYRTAX = "SYRTAX";
const string AUX_ID_VPCPAX = "VPCPAX";
const string AUX_ID_VPSRAX = "VPSRAX";
const string AUX_ID_VSRTAX = "VSRTAX";
const string AUX_ID_VPRTAX = "VPRTAX";
const string AUX_ID_VSCPAX = "VSCPAX";

/**
 * The mask for the SY1 OLCI land flag.
 */
const uint32_t SY1_OLCI_LAND_FLAG = 2147483648U;
/**
 * The mask for the SY1 SLSTR cloud summary flag.
 */
const uint8_t SY1_SLSTR_CLOUD_FLAG = 64U;
/**
 * The mask for the SY2 cloud flag.
 */
const uint16_t SY2_CLOUD_FLAG = 1U;
/**
 * The mask for the SY2 snow-risk flag.
 */
const uint16_t SY2_SNOW_RISK_FLAG = 2U;
/**
 * The mask for the SY2 shadow-risk flag.
 */
const uint16_t SY2_SHADOW_RISK_FLAG = 4U;
/**
 * The mask for the SY2 cloud-filled flag.
 */
const uint16_t SY2_CLOUD_FILLED_FLAG = 8U;
/**
 * The mask for the SY2 land flag.
 */
const uint16_t SY2_LAND_FLAG = 16U;
/**
 * The mask for the SY2 no-OLC flag.
 */
const uint16_t SY2_NO_OLC_FLAG = 32U;
/**
 * The mask for the SY2 no-SLN flag.
 */
const uint16_t SY2_NO_SLN_FLAG = 64U;
/**
 * The mask for the SY2 no-SLO flag.
 */
const uint16_t SY2_NO_SLO_FLAG = 128U;
/**
 * The mask for the SY2 partly-cloudy flag.
 */
const uint16_t SY2_PARTLY_CLOUDY_FLAG = 256U;
/**
 * The mask for the SY2 partly-water flag.
 */
const uint16_t SY2_PARTLY_WATER_FLAG = 512U;
/**
 * The mask for the SY2 border flag.
 */
const uint16_t SY2_BORDER_FLAG = 1024U;
/**
 * The mask for the SY2 aerosol-filled flag.
 */
const uint16_t SY2_AEROSOL_FILLED_FLAG = 2048U;
/**
 * The mask for the SY2 aerosol-success flag.
 */
const uint16_t SY2_AEROSOL_SUCCESS_FLAG = 4096U;
/**
 * The mask for the SY2 aerosol-negative-curvature flag.
 */
const uint16_t SY2_AEROSOL_NEGATIVE_CURVATURE_FLAG = 8192U;
/**
 * The mask for the SY2 aerosol-too-low flag.
 */
const uint16_t SY2_AEROSOL_TOO_LOW_FLAG = 16384U;
/**
 * The mask for the SY2 aerosol-high-error flag.
 */
const uint16_t SY2_AEROSOL_HIGH_ERROR_FLAG = 32768U;
/**
 * The mask for any flags set by the AVE module.
 */
const uint16_t SY2_AVERAGE_FLAG_MASK = SY2_PARTLY_CLOUDY_FLAG | SY2_PARTLY_WATER_FLAG | SY2_BORDER_FLAG | SY2_AEROSOL_FILLED_FLAG | SY2_AEROSOL_HIGH_ERROR_FLAG | SY2_AEROSOL_NEGATIVE_CURVATURE_FLAG | SY2_AEROSOL_SUCCESS_FLAG | SY2_AEROSOL_TOO_LOW_FLAG;

/**
 * The 'clear sky' value of the VGT cloud flags.
 */
const uint8_t VGT_CLEAR_VALUE = 252U;
/**
 * The 'cloud shadow' value of the VGT cloud flags.
 */
const uint8_t VGT_SHADOW_VALUE = 1U;
/**
 * The 'uncertain' value of the VGT cloud flags.
 */
const uint8_t VGT_UNCERTAIN_VALUE = 2U;
/**
 * The 'cloudy' value of the VGT cloud flags.
 */
const uint8_t VGT_CLOUD_VALUE = 3U;
/**
 * The mask for the VGT ice or snow flag.
 */
const uint8_t VGT_ICE_OR_SNOW_FLAG = 4U;
/**
 * The mask for the VGT land flag.
 */
const uint8_t VGT_LAND_FLAG = 8U;
/**
 * The mask for the VGT MIR-good flag.
 */
const uint8_t VGT_MIR_GOOD_FLAG = 16U;
/**
 * The mask for the VGT B3-good flag.
 */
const uint8_t VGT_B3_GOOD_FLAG = 32U;
/**
 * The mask for the VGT B2-good flag.
 */
const uint8_t VGT_B2_GOOD_FLAG = 64U;
/**
 * The mask for the VGT B0-good flag.
 */
const uint8_t VGT_B0_GOOD_FLAG = 128U;

/**
 * The default fill value used for 'double' type variables.
 */
const double FILL_VALUE_DOUBLE = -numeric_limits<double>::max();
/**
 * The name of the SYN SAFE manifest file.
 */
const string SAFE_MANIFEST_NAME_SYN = "manifest_SYN";
/**
 * The name of the VGT-P SAFE manifest file.
 */
const string SAFE_MANIFEST_NAME_VGT_P = "manifest_VGP";
/**
 * The name of the VGT-S SAFE manifest file.
 */
const string SAFE_MANIFEST_NAME_VGT_S = "manifest_VGS";
}

#endif	/* CONSTANTS_H */

