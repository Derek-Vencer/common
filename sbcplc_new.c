#include <math.h>
#include "sbcplc.h"
#include <stdlib.h>
#include <string.h>

/* Updated for 32-sample smoothness */
#undef OLAL
#define OLAL 32 

/* 32-point Raised Cosine Table for 2ms Crossfade */
static const float rcos[OLAL] = {
    0.997592f, 0.990382f, 0.978423f, 0.961817f, 0.940714f, 0.915308f, 0.885841f, 0.852591f,
    0.815871f, 0.776023f, 0.733432f, 0.688509f, 0.641697f, 0.593461f, 0.544300f, 0.494726f,
    0.445274f, 0.396539f, 0.348303f, 0.301491f, 0.256568f, 0.214129f, 0.174129f, 0.137409f,
    0.104159f, 0.074692f, 0.049286f, 0.028183f, 0.011577f, 0.002408f, 0.000000f, 0.000000f
};

/* Helper: Amplitude Matching */
float AmplitudeMatch(short *y, short bestmatch) {
    float sumx = 0.0f, sumy = 1e-6f;
    for (int i = 0; i < FS; i++) {
        sumx += (float)abs(y[LHIST - FS + i]);
        sumy += (float)abs(y[bestmatch + i]);
    }
    float sf = sumx / sumy;
    if (sf < MIN_SF) sf = MIN_SF;
    if (sf > MAX_SF) sf = MAX_SF;
    return sf;
}

/* Helper: Cross Correlation */
float CrossCorrelation(short *x, short *y) {
    float num = 0, x2 = 0, y2 = 0;
    for (int m = 0; m < M; m++) {
        num += ((float)x[m]) * y[m];
        x2  += ((float)x[m]) * x[m];
        y2  += ((float)y[m]) * y[m];
    }
    float den = (float)sqrt(x2 * y2);
    return (den < 1e-6) ? 0.0f : (num / den);
}

/* Updated PatternMatch with Slope Alignment for SMOOTHNESS */
int PatternMatch(short *y) {
    int n, bestmatch = 0;
    float maxCn = -1e10f;
    
    // Determine the direction of the wave at the end of the good audio
    int target_slope = (y[LHIST-1] >= y[LHIST-2]) ? 1 : -1;

    for (n = 0; n < N; n++) {
        float Cn = CrossCorrelation(&y[LHIST-M], &y[n]);
        if (Cn > maxCn) {
            // Only consider the match if the waveform direction (slope) matches
            int match_slope = (y[n+M-1] >= y[n+M-2]) ? 1 : -1;
            if (match_slope == target_slope) {
                maxCn = Cn;
                bestmatch = n;
            }
        }
    }
    
    // Final Phase Alignment: Snap to nearest upward zero-crossing
    for (int offset = -4; offset <= 4; offset++) {
        int idx = bestmatch + offset;
        if (idx > 0 && idx < N && y[idx-1] <= 0 && y[idx] > 0) return idx;
    }
    return bestmatch;
}

void InitPLC(struct PLC_State *plc_state) {
    memset(plc_state, 0, sizeof(struct PLC_State));
}

void PLC_bad_frame(struct PLC_State *plc_state, short *ZIRbuf, short *out) {
    int i;
    float val, sf, attenuation = 1.0f;
    plc_state->nbf++;

    if (plc_state->nbf > 3) attenuation = pow(NOISE_ATTENUATION_FACTOR, plc_state->nbf - 3);

    if (plc_state->nbf == 1) {
        plc_state->bestlag = PatternMatch(plc_state->hist) + M;
        sf = AmplitudeMatch(plc_state->hist, plc_state->bestlag);

        // OLA blending ZIR tail into the concealment loop
        for (i = 0; i < OLAL; i++) {
            val = ZIRbuf[i] * rcos[i] + sf * plc_state->hist[plc_state->bestlag + i] * rcos[OLAL - 1 - i];
            plc_state->hist[LHIST + i] = (short)fminf(fmaxf(val, -32768), 32767);
        }
        for (; i < FS + SBCRT + OLAL; i++) {
            val = sf * plc_state->hist[plc_state->bestlag + i] * attenuation;
            plc_state->hist[LHIST + i] = (short)fminf(fmaxf(val, -32768), 32767);
        }
    } else {
        for (i = 0; i < FS + SBCRT + OLAL; i++) {
            val = plc_state->hist[plc_state->bestlag + i] * attenuation;
            plc_state->hist[LHIST + i] = (short)fminf(fmaxf(val, -32768), 32767);
        }
    }

    memcpy(out, &plc_state->hist[LHIST], FS * sizeof(short));
    memmove(plc_state->hist, &plc_state->hist[FS], (LHIST + SBCRT + OLAL) * sizeof(short));
}

void PLC_good_frame(struct PLC_State *plc_state, short *in, short *out) {
    int i = 0;
    if (plc_state->nbf > 0) {
        for (i = 0; i < SBCRT; i++) out[i] = plc_state->hist[LHIST + i];
        for (; i < SBCRT + OLAL; i++) {
            int idx = i - SBCRT;
            out[i] = (short)(plc_state->hist[LHIST + i] * rcos[idx] + in[i] * rcos[OLAL - 1 - idx]);
        }
    }
    for (; i < FS; i++) out[i] = in[i];
    memcpy(&plc_state->hist[LHIST], out, FS * sizeof(short));
    memmove(plc_state->hist, &plc_state->hist[FS], LHIST * sizeof(short));
    plc_state->nbf = 0;
}
