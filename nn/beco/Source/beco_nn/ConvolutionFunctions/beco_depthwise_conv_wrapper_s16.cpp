/***************************************************************************
 *
 * Copyright 2015-2025 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/


#include "beco_nn/beco_nnfunctions.h"

int32_t beco_depthwise_conv_wrapper_s16_get_buffer_size(const cmsis_nn_dims *input_dims,
                                                        const cmsis_nn_dims *output_dims,
                                                        const cmsis_nn_dw_conv_params *dw_conv_params,
                                                        const cmsis_nn_dims *filter_dims)
{
    if ((filter_dims->w == 3) && (dw_conv_params->padding.w == 1) &&
        (output_dims->w % 4 == 0) && (dw_conv_params->stride.w == 1 || dw_conv_params->stride.w == 2) &&
        (dw_conv_params->dilation.h == 1) && (dw_conv_params->dilation.w == 1)) {
        return beco_depthwise_conv_nx3_s16_get_buffer_size(input_dims,
                                                           output_dims,
                                                           dw_conv_params,
                                                           filter_dims);
    } else {
        return beco_depthwise_conv_s16_get_buffer_size(input_dims,
                                                       output_dims,
                                                       dw_conv_params,
                                                       filter_dims);
    }
}

arm_cmsis_nn_status beco_depthwise_conv_wrapper_s16(const cmsis_nn_context *ctx,
                                                    const cmsis_nn_dw_conv_params *dw_conv_params,
                                                    const cmsis_nn_per_channel_quant_params *quant_params,
                                                    const cmsis_nn_dims *input_dims,
                                                    const int16_t *input,
                                                    const cmsis_nn_dims *filter_dims,
                                                    const int8_t *kernel,
                                                    const cmsis_nn_dims *bias_dims,
                                                    const int32_t *bias,
                                                    const cmsis_nn_dims *output_dims,
                                                    int16_t *output)
{
    if ((filter_dims->w == 3) && (dw_conv_params->padding.w == 1) &&
        (output_dims->w % 4 == 0) && (dw_conv_params->stride.w == 1 || dw_conv_params->stride.w == 2) &&
        (dw_conv_params->dilation.h == 1) && (dw_conv_params->dilation.w == 1)) {

        return beco_depthwise_conv_nx3_s16(ctx,
                                           dw_conv_params,
                                           quant_params,
                                           input_dims,
                                           input,
                                           filter_dims,
                                           kernel,
                                           bias_dims,
                                           bias,
                                           output_dims,
                                           output);
    } else {
        return beco_depthwise_conv_s16(ctx,
                                       dw_conv_params,
                                       quant_params,
                                       input_dims,
                                       input,
                                       filter_dims,
                                       kernel,
                                       bias_dims,
                                       bias,
                                       output_dims,
                                       output);
    }
}
