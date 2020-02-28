/***********************************************************************************************************************
 * Copyright [2015-2017] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 * 
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * File Name    : r_crc.h
 * Description  : HAL API header file for the CRC module
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup CRC CRC
 * @brief Driver for the CRC Calculator (CRC).
 *
 * This module supports the CRC Calculator (CRC).  It implements
 * the following interface:
 *   - @ref CRC_API
 * @{
 **********************************************************************************************************************/

#ifndef R_CRC_H
#define R_CRC_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_crc_cfg.h"
#include "r_crc_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define CRC_CODE_VERSION_MAJOR (1U)
#define CRC_CODE_VERSION_MINOR (5U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** Driver instance control structure. */
typedef struct st_crc_instance_ctrl
{
    R_CRC_Type      * p_reg;        ///< Pointer to register base address
    uint32_t          open;         ///< Whether or not channel is open
    crc_polynomial_t  polynomial;   ///< CRC Generating Polynomial Switching (GPS).
    crc_bit_order_t   bit_order;    ///< CRC Calculation Switching (LMS).
    bool              fifo_mode;    ///< FIFO Mode selection for sci_uart in CRC snoop operation.
} crc_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const crc_api_t g_crc_on_crc;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_CRC_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup CRC)
 **********************************************************************************************************************/
