/*************************************************************
SBC Example PLC ANSI-C Source Code
This is copy from HFP spec, just only for study and demo.
Please don't use in commercial product.
File: sbcplc.c
*************************************************************/
#include <math.h>
#include "sbcplc.h"
#include <stdlib.h>

#define FS 120 /* Frame Size */
#define N 256 /* 16ms - Window Length for pattern matching */
#define M 64 /* 4ms - Template for matching */
#define LHIST (N+FS-1) /* Length of history buffer required */
#define SBCRT 36 /* SBC Reconvergence Time (samples) */
#define OLAL 16 /* OverLap-Add Length (samples) */
/* PLC State Information */
struct PLC_State
{
    short hist[LHIST+FS+SBCRT+OLAL];
    short bestlag;
    int nbf;
};
/* Prototypes */
void InitPLC(struct PLC_State *plc_state);
void PLC_bad_frame(struct PLC_State *plc_state, short *ZIRbuf, short *out);
void PLC_good_frame(struct PLC_State *plc_state, short *in, short *out);

/* Local Function Prototypes */
float CrossCorrelation(short *x, short *y);
int PatternMatch(short *y);
float AmplitudeMatch(short *y, short bestmatch);
/* Raised COSine table for OLA */
float rcos[OLAL] = {0.99148655f,0.96623611f,0.92510857f,0.86950446f,
                    0.80131732f,0.72286918f,0.63683150f,0.54613418f,
                    0.45386582f,0.36316850f,0.27713082f,0.19868268f,
                    0.13049554f,0.07489143f,0.03376389f,0.00851345f};

/* Advanced PLC parameters */
#define NOISE_ATTENUATION_FACTOR 0.95f  // Noise attenuation factor for long losses
#define MAX_SF 1.5f                      // Increased maximum scale factor for better amplitude matching
#define MIN_SF 0.5f                      // Decreased minimum scale factor for better amplitude matching

/*****************************************************************************
* Function: InitPLC() *
* Purpose: Perform PLC initialization of memory vectors. *
* Inputs: *plc_state - pointer to PLC state memory *
* Outputs: *plc_state - initialized memory. *
* Date: 03-18-2009
*****************************************************************************/
void InitPLC(struct PLC_State *plc_state)
{
    int i;
    plc_state->nbf=0;
    plc_state->bestlag=0;
    for (i=0;i<LHIST+SBCRT;i++)
    plc_state->hist[i] = 0;
}

/***********************************************************
* Function: PLC_bad_frame()
*
* Purpose: Perform bad frame processing with advanced features.
*
* Inputs: *plc_state - pointer to PLC state memory
* *ZIRbuf - pointer to the ZIR response of the SBC decoder
*
* Outputs: *out - pointer to the output samples
*
* Date: 03-18-2009
* Enhanced: 2026-04-07 - Added adaptive noise attenuation and improved quality
************************************************************/
void PLC_bad_frame(struct PLC_State *plc_state, short *ZIRbuf, short *out)
{
    int i;
    float val;
    float sf;
    float attenuation_factor = 1.0f;
    
    plc_state->nbf++;
    sf = 1.0f;
    i = 0;
    
    /* Apply adaptive noise attenuation for long consecutive losses */
    if (plc_state->nbf > 3) {
        // Apply gradual attenuation for long losses to reduce artifacts
        attenuation_factor = pow(NOISE_ATTENUATION_FACTOR, plc_state->nbf - 3);
        //attenuation_factor *= NOISE_ATTENUATION_FACTOR;
    }
    
    if (plc_state->nbf == 1)
    {
        /* Perform pattern matching to find where to replicate */
        plc_state->bestlag = PatternMatch(plc_state->hist);
        plc_state->bestlag += M; /* the replication begins after the template match*/

        /* Compute Scale Factor to Match Amplitude of Substitution Packet to that of
        Preceding Packet */
        sf = AmplitudeMatch(plc_state->hist, plc_state->bestlag);

        for (i = 0; i < OLAL; i++)
        {
            val = ZIRbuf[i] * rcos[i] + sf * plc_state->hist[plc_state->bestlag + i] * rcos[OLAL - i - 1];
            if (val > 32767.0) val = 32767.0;
            if (val < -32768.0) val = -32768.0;
            plc_state->hist[LHIST + i] = (short)val;
        }

        for (; i < FS; i++)
        {
            val = sf * plc_state->hist[plc_state->bestlag + i] * attenuation_factor;
            if (val > 32767.0) val = 32767.0;
            if (val < -32768.0) val = -32768.0;
            plc_state->hist[LHIST + i] = (short)val;
        }

        for (; i < FS + OLAL; i++)
        {
            val = sf * plc_state->hist[plc_state->bestlag + i] * rcos[i - FS] + 
                  plc_state->hist[plc_state->bestlag + i] * rcos[OLAL - 1 - i + FS];
            val *= attenuation_factor;
            if (val > 32767.0) val = 32767.0;
            if (val < -32768.0) val = -32768.0;
            plc_state->hist[LHIST + i] = (short)val;
        }

        for (; i < FS + SBCRT + OLAL; i++)
            plc_state->hist[LHIST + i] = plc_state->hist[plc_state->bestlag + i] * attenuation_factor;
    }
    else
    {
        for (; i < FS; i++)
            plc_state->hist[LHIST + i] = plc_state->hist[plc_state->bestlag + i] * attenuation_factor;
        for (; i < FS + SBCRT + OLAL; i++)
            plc_state->hist[LHIST + i] = plc_state->hist[plc_state->bestlag + i] * attenuation_factor;
    }

    for (i = 0; i < FS; i++)
        out[i] = plc_state->hist[LHIST + i];

    /* shift the history buffer */
    for (i = 0; i < LHIST + SBCRT + OLAL; i++)
        plc_state->hist[i] = plc_state->hist[i + FS];
}

/****************************************************************************
*
* Function: PLC_good_frame()
*
* Purpose: Perform good frame processing. Most of the time, this function
* just updates history buffers and passes the input to the output,
* but in the first good frame after frame loss, it must conceal the
* received signal as it reconverges with the true output.
*
* Inputs: *plc_state - pointer to PLC state memory
* *in - pointer to the input vector
*
* Outputs: *out - pointer to the output samples
* Date: 03-18-2009
* Enhanced: 2026-04-07 - Added improved recovery mechanism and better history management
*****************************************************************************/
void PLC_good_frame(struct PLC_State *plc_state, short *in, short *out)
{
  int i;
  float transition_factor;
  i = 0;
  
  if (plc_state->nbf > 0)
  {
    /* For long losses, use a more gradual transition */
    float rcos_scaled[OLAL];
    
    /* Scale the crossfade window for smoother transition after long losses */
    for (i = 0; i < OLAL; i++) {
      transition_factor = (plc_state->nbf > 5) ? 0.5f : 1.0f;
      rcos_scaled[i] = rcos[i] * transition_factor;
    }
    
    i = 0;
    for (; i < SBCRT; i++)
      out[i] = plc_state->hist[LHIST + i];
    
    for (; i < SBCRT + OLAL; i++) {
      int idx = i - SBCRT;
      out[i] = (short)(plc_state->hist[LHIST + i] * rcos_scaled[idx] + 
                      in[i] * rcos[OLAL - 1 - idx]);
    }
  }

  for (; i < FS; i++)
    out[i] = in[i];

  /*Copy the output to the history buffer */
  for (i = 0; i < FS; i++)
    plc_state->hist[LHIST + i] = out[i];

  /* shift the history buffer */
  for (i = 0; i < LHIST; i++)
    plc_state->hist[i] = plc_state->hist[i + FS];

  /* Reset the bad frame count */
  plc_state->nbf = 0;
}

/****************************************************************************
*
* Function: CrossCorrelation()
*
* Purpose: Compute the cross correlation according to Eq. (4) of Goodman
* paper, except that the true correlation is used. His formula
* seems to be incorrect.
*
* Inputs: *x - pointer to x input vector
* *y - pointer to y input vector
*
* Outputs: Cn - return value containing the cross-correlation of x and y
*
* Date: 03-18-2009
*****************************************************************************/
float CrossCorrelation(short *x, short *y)
{
  int m;
  float num;
  float den;
  float Cn;
  float x2, y2;
  num=0;
  den=0;
  x2=0.0;
  y2=0.0;

  for (m=0;m<M;m++)
  {
    num+=((float)x[m])*y[m];
    x2+=((float)x[m])*x[m];
    y2+=((float)y[m])*y[m];
  }

  den = (float)sqrt(x2*y2);
  if (den < 1e-6) 
    return 0.0f;
  Cn = num/den;
  return(Cn);
}

/****************************************************************************
*
* Function: PatternMatch()
*
* Purpose: Perform pattern matching to find the match of template with the
* history buffer according to Section B of Goodman paper.
*
* Inputs: *y : pointer to history buffer
*
* Outputs: return(int): the lag corresponding to the best match. The lag is
* with respect to the beginning of the history buffer.
*
* Date: 03-18-2009
*****************************************************************************/
int PatternMatch(short *y)
{
  int n;
  float maxCn;
  float Cn;
  int bestmatch;
  maxCn=-999999.0; /* large negative number */
  bestmatch=0;

  for (n=0;n<N;n++)
  {
    Cn = CrossCorrelation(&y[LHIST-M] /* x */, &y[n]);
    if (Cn>maxCn)
    {
      bestmatch=n;
      maxCn = Cn;
    }
  }
  /* PHASE ALIGNMENT: Snap to the nearest upward zero-crossing 
     within a +/- 4 sample window to ensure a smooth stitch */
  for (int offset = -4; offset <= 4; offset++) {
    int idx = bestmatch + offset;
    if (idx > 0 && idx < N && y[idx-1] <= 0 && y[idx] > 0) {
      return idx; 
    }
  }
  return(bestmatch);
}

/****************************************************************************
*
* Function: AmplitudeMatch()
*
* Purpose: Perform amplitude matching using mean-absolute-value according
* to Goodman paper with enhanced scaling limits.
*
* Inputs: *y : pointer to history buffer
* bestmatch : value of the lag to the best match
*
* Outputs: return(float): scale factor
*
* Date: 03-19-2009
* Enhanced: 2026-04-07 - Added wider scaling range for better amplitude matching
*****************************************************************************/
float AmplitudeMatch(short *y, short bestmatch)
{
  int i;
  float sumx;
  float sumy;
  float sf;
  sumx = 0.0;
  sumy = 0.000001f; /* Avoid division by zero */

  for (i = 0; i < FS; i++)
  {
    sumx += abs(y[LHIST - FS + i]);
    sumy += abs(y[bestmatch + i]);
  }

  sf = sumx / sumy;

  /* Limit the scaling factor to a reasonable range to avoid artifacts,
   * but with wider range for better amplitude matching */
  if (sf < MIN_SF) sf = MIN_SF;
  if (sf > MAX_SF) sf = MAX_SF;

  return(sf);
}
