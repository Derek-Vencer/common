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

int32_t beco_transpose_conv_s16_get_buffer_size(
                                const cmsis_nn_dims *input_dims,
                                const cmsis_nn_dims *output_dims,
                                const cmsis_nn_transpose_conv_params *conv_params,
                                const cmsis_nn_dims *filter_dims)
{
    const uint16_t input_x = input_dims->w;
    const uint16_t input_y = input_dims->h;
    const uint16_t input_ch = input_dims->c;
    const uint16_t stride_x = conv_params->stride.w;
    const uint16_t stride_y = conv_params->stride.h;
    const uint16_t dilation_y = conv_params->dilation.h;
    const uint16_t kernel_x = filter_dims->w;
    const uint16_t kernel_y = filter_dims->h;
    const uint16_t output_padding_x = conv_params->padding_offsets.w;
    const uint16_t output_padding_y = conv_params->padding_offsets.h;

    uint16_t input_x_new = (input_x - 1) * stride_x + output_padding_x + 1;
    uint16_t input_y_new = (input_y - 1) * stride_y + output_padding_y + 1;
    int32_t  input_buffer_len = input_ch * input_x_new * input_y_new * sizeof(int16_t);

    uint16_t dilated_kernel_y = kernel_y + (kernel_y-1) * (dilation_y-1);
    const uint16_t output_x = output_dims->w;

    int32_t im2col_buffer_len = dilated_kernel_y * input_ch * kernel_x * 4 * sizeof(int16_t);
    int32_t output_buffer_len = 0;
#if defined(ARM_MATH_MVEI)
    output_buffer_len += 32 * sizeof(int32_t);
#endif
    if (output_x % 8 != 0) {
        output_buffer_len += 32 * sizeof(int16_t);
    }

    return ALIGN(input_buffer_len, 4) + im2col_buffer_len + output_buffer_len;
}

arm_cmsis_nn_status beco_transpose_conv_s16(
                                const cmsis_nn_context *ctx,
                                const cmsis_nn_transpose_conv_params *conv_params,
                                const cmsis_nn_per_channel_quant_params *quant_params,
                                const cmsis_nn_dims *input_dims,
                                const int16_t *input_data,
                                const cmsis_nn_dims *filter_dims,
                                const int8_t *filter_data,
                                const cmsis_nn_dims *bias_dims,
                                const int32_t *bias_data,
                                const cmsis_nn_dims *output_dims,
                                int16_t *output_data)
{
    (void)bias_dims;

    const uint16_t input_x = input_dims->w;
    const uint16_t input_y = input_dims->h;
    const uint16_t input_ch = input_dims->c;
    const uint16_t output_x = output_dims->w;
    const uint16_t output_y = output_dims->h;
    const uint16_t output_ch = output_dims->c;
    const uint16_t kernel_x = filter_dims->w;
    const uint16_t kernel_y = filter_dims->h;
    const uint16_t kernel_ch = filter_dims->c;
    const uint16_t pad_x = conv_params->padding.w;
    const uint16_t pad_y = conv_params->padding.h;
    const uint16_t output_padding_x = conv_params->padding_offsets.w;
    const uint16_t output_padding_y = conv_params->padding_offsets.h;
    const uint16_t stride_x = conv_params->stride.w;
    const uint16_t stride_y = conv_params->stride.h;
    const uint16_t dilation_x = conv_params->dilation.w;
    const uint16_t dilation_y = conv_params->dilation.h;

    const uint16_t groups = input_ch / kernel_ch;

    if (groups != 1 || output_ch % 8 != 0) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }
    if (kernel_x <= pad_x || kernel_y <= pad_y) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }

    uint16_t expected_output_x = (input_x - 1) * stride_x - 2 * pad_x + dilation_x * (kernel_x - 1) + output_padding_x + 1;
    uint16_t expected_output_y = (input_y - 1) * stride_y - 2 * pad_y + dilation_y * (kernel_y - 1) + output_padding_y + 1;
    if (output_x != expected_output_x || output_y != expected_output_y) {
        return ARM_CMSIS_NN_ARG_ERROR;
    }

    int16_t *input_buffer = (int16_t *)ctx->buf;
    uint16_t expanded_input_x = (input_x - 1) * stride_x + output_padding_x + 1;
    uint16_t expanded_input_y = (input_y - 1) * stride_y + output_padding_y + 1;
    int32_t input_buffer_len = input_ch * expanded_input_x * expanded_input_y;
    memset(input_buffer, 0, input_buffer_len * sizeof(int16_t));
    // apply 0 to input
    int16_t *padded_input = input_buffer;
    for (int ich = 0; ich < input_ch; ich++) {
        int16_t *cur_padded_in = padded_input;
        int16_t *cur_in = (int16_t *)input_data;
        for (int iy = 0; iy < input_y; iy++) {
            for (int ix = 0; ix < input_x; ix++) {
                cur_padded_in[ix*stride_x] = cur_in[ix];
            }
            cur_padded_in += expanded_input_x * stride_y;
            cur_in += input_x;
        }
        padded_input += expanded_input_x * expanded_input_y;
        input_data += input_x * input_y;
    }

    uint16_t padding_x_new = dilation_x * (kernel_x - 1) - pad_x;
    uint16_t padding_y_new = dilation_y * (kernel_y - 1) - pad_y;
    int16_t *buffer = input_buffer + ALIGN(input_buffer_len, 4);
    if (output_x % 4 == 0) {
        beco_convolve_q15(input_buffer, expanded_input_x, expanded_input_y, input_ch,
                          filter_data, output_ch, kernel_x, kernel_y,
                          padding_x_new, padding_y_new, 1, 1, bias_data,
                          quant_params->multiplier, quant_params->shift, output_data,
                          output_x, output_y, dilation_x, dilation_y, groups,
                          conv_params->activation.min, conv_params->activation.max, buffer);
    } else {
        beco_convolve_norm_q15(input_buffer, expanded_input_x, expanded_input_y, input_ch,
                               filter_data, output_ch, kernel_x, kernel_y,
                               padding_x_new, padding_y_new, 1, 1, bias_data,
                               quant_params->multiplier, quant_params->shift, output_data,
                               output_x, output_y, dilation_x, dilation_y, groups,
                               conv_params->activation.min, conv_params->activation.max, buffer);
    }

    return ARM_CMSIS_NN_SUCCESS;
}

