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


int32_t beco_convolve_wrapper_s8_get_buffer_size(const cmsis_nn_dims *input_dims,
                                                 const cmsis_nn_dims *output_dims,
                                                 const cmsis_nn_conv_params *conv_params,
                                                 const cmsis_nn_dims *filter_dims)
{
    if ((filter_dims->h == 1) && (filter_dims->w == 1) &&
        (conv_params->padding.h == 0) && (conv_params->padding.w == 0) &&
        (conv_params->stride.h == 1) && (conv_params->stride.w == 1) &&
        (conv_params->dilation.h == 1) && (conv_params->dilation.w == 1)) {
        return beco_convolve_1x1_s8_get_buffer_size(output_dims, filter_dims);
    } else {
        return beco_convolve_s8_get_buffer_size(input_dims, output_dims, conv_params, filter_dims);
    }
}

arm_cmsis_nn_status beco_convolve_wrapper_s8(const cmsis_nn_context *ctx,
                                             const cmsis_nn_conv_params *conv_params,
                                             const cmsis_nn_per_channel_quant_params *quant_params,
                                             const cmsis_nn_dims *input_dims,
                                             const int8_t *input_data,
                                             const cmsis_nn_dims *filter_dims,
                                             const int8_t *filter_data,
                                             const cmsis_nn_dims *bias_dims,
                                             const int32_t *bias_data,
                                             const cmsis_nn_dims *output_dims,
                                             int8_t *output_data)
{
    if ((filter_dims->h == 1) && (filter_dims->w == 1) &&
        (conv_params->padding.h == 0) && (conv_params->padding.w == 0) &&
        (conv_params->stride.h == 1) && (conv_params->stride.w == 1) &&
        (conv_params->dilation.h == 1) && (conv_params->dilation.w == 1)) {

        return beco_convolve_1x1_s8(ctx,
                                    conv_params,
                                    quant_params,
                                    input_dims,
                                    input_data,
                                    filter_dims,
                                    filter_data,
                                    bias_dims,
                                    bias_data,
                                    output_dims,
                                    output_data);
    } else {
        return beco_convolve_s8(ctx,
                                conv_params,
                                quant_params,
                                input_dims,
                                input_data,
                                filter_dims,
                                filter_data,
                                bias_dims,
                                bias_data,
                                output_dims,
                                output_data);
    }
}
