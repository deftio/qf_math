/**
 *  @file qf_math.c - QF_MATH: Quick Float Math Library implementation
 *
 *  Fast approximate math on IEEE 754 float32 (table trig; polynomial log2 / Horner pow2 paths).
 *  Same algorithmic approach as FR_math.c but operating natively on floats.
 *
 *  @author M A Chatterjee <deftio [at] deftio [dot] com>
 *
 *  Copyright (c) 2002-2026, M. A. Chatterjee
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "qf_math.h"

/*=======================================================
 * Lookup tables
 *
 * Native qf constants computed at full float32 precision (~7 digits).
 * No int-to-float conversion needed at lookup time.
 */

/* Full-cycle trig tables: 512 entries per turn, 128 BAM sub-steps per table interval.
 * Forward sin/cos/tan lookup uses index = bam >> 7 and frac = bam & 0x7f. */
#define QF_TRIG_CYCLE_BITS 9
#define QF_TRIG_CYCLE_SIZE (1 << QF_TRIG_CYCLE_BITS)  /* 512 */
#define QF_TRIG_TABLE_SIZE (QF_TRIG_CYCLE_SIZE + 1)    /* sentinel repeats entry 0 */
#define QF_TRIG_CYCLE_MASK (QF_TRIG_CYCLE_SIZE - 1)
#define QF_TRIG_FRAC_BITS  7
#define QF_BAM_UNIT_FRAC_BITS 7
#define QF_BAM_UNIT_SCALE 128.0f
#define QF_INV_BAM_UNIT_SCALE 0.0078125f
#define QF_TRIG_PHASE_FRAC_BITS (QF_TRIG_FRAC_BITS + QF_BAM_UNIT_FRAC_BITS)
#define QF_TRIG_PHASE_FRAC_MASK ((1u << QF_TRIG_PHASE_FRAC_BITS) - 1u)
#define QF_TRIG_PHASE_FRAC_SCALE 0.00006103515625f     /* exact 2^-14 */
#define QF_BAM_PHASE_MASK  ((1u << (16 + QF_BAM_UNIT_FRAC_BITS)) - 1u)
#define SIN_QUAD_SIZE      129
#define SIN_STEP_RAD       0.01227185f                /* (pi / 2) / 128 */

#define QF_THREE_HALF_PI 4.71238898f
#define QF_INV_TWO_PI    0.15915494f
#define QF_BAM_CYCLE     65536.0f
#define QF_INV_BAM_CYCLE 0.0000152587890625f
#define QF_BAM_PER_RAD   10430.37835f                 /* 65536 / (2*pi) */
#define QF_BAM_PER_DEG   182.04444444f
#define QF_RAD_PER_BAM   0.00009587379924f
#define QF_INV_32768     0.000030517578125f
#define QF_INV_16384     0.00006103515625f

static const qf gSIN_TAB[QF_TRIG_TABLE_SIZE] = {
    0.00000000f,    0.01227154f,    0.02454123f,    0.03680722f,    0.04906767f,    0.06132074f,    0.07356456f,    0.08579731f,
    0.09801714f,    0.11022221f,    0.12241068f,    0.13458071f,    0.14673047f,    0.15885814f,    0.17096189f,    0.18303989f,
    0.19509032f,    0.20711138f,    0.21910124f,    0.23105811f,    0.24298018f,    0.25486566f,    0.26671276f,    0.27851969f,
    0.29028468f,    0.30200595f,    0.31368174f,    0.32531029f,    0.33688985f,    0.34841868f,    0.35989504f,    0.37131719f,
    0.38268343f,    0.39399204f,    0.40524131f,    0.41642956f,    0.42755509f,    0.43861624f,    0.44961133f,    0.46053871f,
    0.47139674f,    0.48218377f,    0.49289819f,    0.50353838f,    0.51410274f,    0.52458968f,    0.53499762f,    0.54532499f,
    0.55557023f,    0.56573181f,    0.57580819f,    0.58579786f,    0.59569930f,    0.60551104f,    0.61523159f,    0.62485949f,
    0.63439328f,    0.64383154f,    0.65317284f,    0.66241578f,    0.67155895f,    0.68060100f,    0.68954054f,    0.69837625f,
    0.70710678f,    0.71573083f,    0.72424708f,    0.73265427f,    0.74095113f,    0.74913639f,    0.75720885f,    0.76516727f,
    0.77301045f,    0.78073723f,    0.78834643f,    0.79583690f,    0.80320753f,    0.81045720f,    0.81758481f,    0.82458930f,
    0.83146961f,    0.83822471f,    0.84485357f,    0.85135519f,    0.85772861f,    0.86397286f,    0.87008699f,    0.87607009f,
    0.88192126f,    0.88763962f,    0.89322430f,    0.89867447f,    0.90398929f,    0.90916798f,    0.91420976f,    0.91911385f,
    0.92387953f,    0.92850608f,    0.93299280f,    0.93733901f,    0.94154407f,    0.94560733f,    0.94952818f,    0.95330604f,
    0.95694034f,    0.96043052f,    0.96377607f,    0.96697647f,    0.97003125f,    0.97293995f,    0.97570213f,    0.97831737f,
    0.98078528f,    0.98310549f,    0.98527764f,    0.98730142f,    0.98917651f,    0.99090264f,    0.99247953f,    0.99390697f,
    0.99518473f,    0.99631261f,    0.99729046f,    0.99811811f,    0.99879546f,    0.99932238f,    0.99969882f,    0.99992470f,
    1.00000000f,    0.99992470f,    0.99969882f,    0.99932238f,    0.99879546f,    0.99811811f,    0.99729046f,    0.99631261f,
    0.99518473f,    0.99390697f,    0.99247953f,    0.99090264f,    0.98917651f,    0.98730142f,    0.98527764f,    0.98310549f,
    0.98078528f,    0.97831737f,    0.97570213f,    0.97293995f,    0.97003125f,    0.96697647f,    0.96377607f,    0.96043052f,
    0.95694034f,    0.95330604f,    0.94952818f,    0.94560733f,    0.94154407f,    0.93733901f,    0.93299280f,    0.92850608f,
    0.92387953f,    0.91911385f,    0.91420976f,    0.90916798f,    0.90398929f,    0.89867447f,    0.89322430f,    0.88763962f,
    0.88192126f,    0.87607009f,    0.87008699f,    0.86397286f,    0.85772861f,    0.85135519f,    0.84485357f,    0.83822471f,
    0.83146961f,    0.82458930f,    0.81758481f,    0.81045720f,    0.80320753f,    0.79583690f,    0.78834643f,    0.78073723f,
    0.77301045f,    0.76516727f,    0.75720885f,    0.74913639f,    0.74095113f,    0.73265427f,    0.72424708f,    0.71573083f,
    0.70710678f,    0.69837625f,    0.68954054f,    0.68060100f,    0.67155895f,    0.66241578f,    0.65317284f,    0.64383154f,
    0.63439328f,    0.62485949f,    0.61523159f,    0.60551104f,    0.59569930f,    0.58579786f,    0.57580819f,    0.56573181f,
    0.55557023f,    0.54532499f,    0.53499762f,    0.52458968f,    0.51410274f,    0.50353838f,    0.49289819f,    0.48218377f,
    0.47139674f,    0.46053871f,    0.44961133f,    0.43861624f,    0.42755509f,    0.41642956f,    0.40524131f,    0.39399204f,
    0.38268343f,    0.37131719f,    0.35989504f,    0.34841868f,    0.33688985f,    0.32531029f,    0.31368174f,    0.30200595f,
    0.29028468f,    0.27851969f,    0.26671276f,    0.25486566f,    0.24298018f,    0.23105811f,    0.21910124f,    0.20711138f,
    0.19509032f,    0.18303989f,    0.17096189f,    0.15885814f,    0.14673047f,    0.13458071f,    0.12241068f,    0.11022221f,
    0.09801714f,    0.08579731f,    0.07356456f,    0.06132074f,    0.04906767f,    0.03680722f,    0.02454123f,    0.01227154f,
    0.00000000f,    -0.01227154f,    -0.02454123f,    -0.03680722f,    -0.04906767f,    -0.06132074f,    -0.07356456f,    -0.08579731f,
    -0.09801714f,    -0.11022221f,    -0.12241068f,    -0.13458071f,    -0.14673047f,    -0.15885814f,    -0.17096189f,    -0.18303989f,
    -0.19509032f,    -0.20711138f,    -0.21910124f,    -0.23105811f,    -0.24298018f,    -0.25486566f,    -0.26671276f,    -0.27851969f,
    -0.29028468f,    -0.30200595f,    -0.31368174f,    -0.32531029f,    -0.33688985f,    -0.34841868f,    -0.35989504f,    -0.37131719f,
    -0.38268343f,    -0.39399204f,    -0.40524131f,    -0.41642956f,    -0.42755509f,    -0.43861624f,    -0.44961133f,    -0.46053871f,
    -0.47139674f,    -0.48218377f,    -0.49289819f,    -0.50353838f,    -0.51410274f,    -0.52458968f,    -0.53499762f,    -0.54532499f,
    -0.55557023f,    -0.56573181f,    -0.57580819f,    -0.58579786f,    -0.59569930f,    -0.60551104f,    -0.61523159f,    -0.62485949f,
    -0.63439328f,    -0.64383154f,    -0.65317284f,    -0.66241578f,    -0.67155895f,    -0.68060100f,    -0.68954054f,    -0.69837625f,
    -0.70710678f,    -0.71573083f,    -0.72424708f,    -0.73265427f,    -0.74095113f,    -0.74913639f,    -0.75720885f,    -0.76516727f,
    -0.77301045f,    -0.78073723f,    -0.78834643f,    -0.79583690f,    -0.80320753f,    -0.81045720f,    -0.81758481f,    -0.82458930f,
    -0.83146961f,    -0.83822471f,    -0.84485357f,    -0.85135519f,    -0.85772861f,    -0.86397286f,    -0.87008699f,    -0.87607009f,
    -0.88192126f,    -0.88763962f,    -0.89322430f,    -0.89867447f,    -0.90398929f,    -0.90916798f,    -0.91420976f,    -0.91911385f,
    -0.92387953f,    -0.92850608f,    -0.93299280f,    -0.93733901f,    -0.94154407f,    -0.94560733f,    -0.94952818f,    -0.95330604f,
    -0.95694034f,    -0.96043052f,    -0.96377607f,    -0.96697647f,    -0.97003125f,    -0.97293995f,    -0.97570213f,    -0.97831737f,
    -0.98078528f,    -0.98310549f,    -0.98527764f,    -0.98730142f,    -0.98917651f,    -0.99090264f,    -0.99247953f,    -0.99390697f,
    -0.99518473f,    -0.99631261f,    -0.99729046f,    -0.99811811f,    -0.99879546f,    -0.99932238f,    -0.99969882f,    -0.99992470f,
    -1.00000000f,    -0.99992470f,    -0.99969882f,    -0.99932238f,    -0.99879546f,    -0.99811811f,    -0.99729046f,    -0.99631261f,
    -0.99518473f,    -0.99390697f,    -0.99247953f,    -0.99090264f,    -0.98917651f,    -0.98730142f,    -0.98527764f,    -0.98310549f,
    -0.98078528f,    -0.97831737f,    -0.97570213f,    -0.97293995f,    -0.97003125f,    -0.96697647f,    -0.96377607f,    -0.96043052f,
    -0.95694034f,    -0.95330604f,    -0.94952818f,    -0.94560733f,    -0.94154407f,    -0.93733901f,    -0.93299280f,    -0.92850608f,
    -0.92387953f,    -0.91911385f,    -0.91420976f,    -0.90916798f,    -0.90398929f,    -0.89867447f,    -0.89322430f,    -0.88763962f,
    -0.88192126f,    -0.87607009f,    -0.87008699f,    -0.86397286f,    -0.85772861f,    -0.85135519f,    -0.84485357f,    -0.83822471f,
    -0.83146961f,    -0.82458930f,    -0.81758481f,    -0.81045720f,    -0.80320753f,    -0.79583690f,    -0.78834643f,    -0.78073723f,
    -0.77301045f,    -0.76516727f,    -0.75720885f,    -0.74913639f,    -0.74095113f,    -0.73265427f,    -0.72424708f,    -0.71573083f,
    -0.70710678f,    -0.69837625f,    -0.68954054f,    -0.68060100f,    -0.67155895f,    -0.66241578f,    -0.65317284f,    -0.64383154f,
    -0.63439328f,    -0.62485949f,    -0.61523159f,    -0.60551104f,    -0.59569930f,    -0.58579786f,    -0.57580819f,    -0.56573181f,
    -0.55557023f,    -0.54532499f,    -0.53499762f,    -0.52458968f,    -0.51410274f,    -0.50353838f,    -0.49289819f,    -0.48218377f,
    -0.47139674f,    -0.46053871f,    -0.44961133f,    -0.43861624f,    -0.42755509f,    -0.41642956f,    -0.40524131f,    -0.39399204f,
    -0.38268343f,    -0.37131719f,    -0.35989504f,    -0.34841868f,    -0.33688985f,    -0.32531029f,    -0.31368174f,    -0.30200595f,
    -0.29028468f,    -0.27851969f,    -0.26671276f,    -0.25486566f,    -0.24298018f,    -0.23105811f,    -0.21910124f,    -0.20711138f,
    -0.19509032f,    -0.18303989f,    -0.17096189f,    -0.15885814f,    -0.14673047f,    -0.13458071f,    -0.12241068f,    -0.11022221f,
    -0.09801714f,    -0.08579731f,    -0.07356456f,    -0.06132074f,    -0.04906767f,    -0.03680722f,    -0.02454123f,    -0.01227154f,
    0.00000000f
};

static const qf gTAN_TAB[QF_TRIG_TABLE_SIZE] = {
    0.00000000f,    0.01227246f,    0.02454862f,    0.03683218f,    0.04912685f,    0.06143635f,    0.07376443f,    0.08611485f,
    0.09849140f,    0.11089791f,    0.12333824f,    0.13581628f,    0.14833599f,    0.16090136f,    0.17351646f,    0.18618540f,
    0.19891237f,    0.21170162f,    0.22455751f,    0.23748445f,    0.25048696f,    0.26356966f,    0.27673727f,    0.28999463f,
    0.30334668f,    0.31679853f,    0.33035538f,    0.34402260f,    0.35780572f,    0.37171042f,    0.38574257f,    0.39990820f,
    0.41421356f,    0.42866511f,    0.44326951f,    0.45803368f,    0.47296478f,    0.48807021f,    0.50335770f,    0.51883523f,
    0.53451114f,    0.55039406f,    0.56649300f,    0.58281737f,    0.59937693f,    0.61618193f,    0.63324302f,    0.65057136f,
    0.66817864f,    0.68607707f,    0.70427946f,    0.72279925f,    0.74165055f,    0.76084816f,    0.78040766f,    0.80034545f,
    0.82067879f,    0.84142588f,    0.86260593f,    0.88423922f,    0.90634717f,    0.92895247f,    0.95207915f,    0.97575265f,
    1.00000000f,    1.02484989f,    1.05033285f,    1.07648134f,    1.10332998f,    1.13091569f,    1.15927791f,    1.18845880f,
    1.21850353f,    1.24946047f,    1.28138158f,    1.31432270f,    1.34834391f,    1.38351001f,    1.41989090f,    1.45756220f,
    1.49660576f,    1.53711039f,    1.57917257f,    1.62289733f,    1.66839921f,    1.71580337f,    1.76524687f,    1.81688009f,
    1.87086841f,    1.92739416f,    1.98665879f,    2.04888553f,    2.11432236f,    2.18324555f,    2.25596385f,    2.33282340f,
    2.41421356f,    2.50057389f,    2.59240252f,    2.69026624f,    2.79481277f,    2.90678576f,    3.02704320f,    3.15658033f,
    3.29655821f,    3.44833976f,    3.61353568f,    3.79406340f,    3.99222378f,    4.21080203f,    4.45320222f,    4.72362933f,
    5.02733949f,    5.37099044f,    5.76314201f,    6.21498777f,    6.74145241f,    7.36288764f,    8.10778580f,    9.01730236f,
    10.15317039f,    11.61239886f,    13.55666924f,    16.27700796f,    20.35546762f,    27.15017067f,    40.73548387f,    81.48324021f,
    QF_TAN_MAX,    -81.48324021f,    -40.73548387f,    -27.15017067f,    -20.35546762f,    -16.27700796f,    -13.55666924f,    -11.61239886f,
    -10.15317039f,    -9.01730236f,    -8.10778580f,    -7.36288764f,    -6.74145241f,    -6.21498777f,    -5.76314201f,    -5.37099044f,
    -5.02733949f,    -4.72362933f,    -4.45320222f,    -4.21080203f,    -3.99222378f,    -3.79406340f,    -3.61353568f,    -3.44833976f,
    -3.29655821f,    -3.15658033f,    -3.02704320f,    -2.90678576f,    -2.79481277f,    -2.69026624f,    -2.59240252f,    -2.50057389f,
    -2.41421356f,    -2.33282340f,    -2.25596385f,    -2.18324555f,    -2.11432236f,    -2.04888553f,    -1.98665879f,    -1.92739416f,
    -1.87086841f,    -1.81688009f,    -1.76524687f,    -1.71580337f,    -1.66839921f,    -1.62289733f,    -1.57917257f,    -1.53711039f,
    -1.49660576f,    -1.45756220f,    -1.41989090f,    -1.38351001f,    -1.34834391f,    -1.31432270f,    -1.28138158f,    -1.24946047f,
    -1.21850353f,    -1.18845880f,    -1.15927791f,    -1.13091569f,    -1.10332998f,    -1.07648134f,    -1.05033285f,    -1.02484989f,
    -1.00000000f,    -0.97575265f,    -0.95207915f,    -0.92895247f,    -0.90634717f,    -0.88423922f,    -0.86260593f,    -0.84142588f,
    -0.82067879f,    -0.80034545f,    -0.78040766f,    -0.76084816f,    -0.74165055f,    -0.72279925f,    -0.70427946f,    -0.68607707f,
    -0.66817864f,    -0.65057136f,    -0.63324302f,    -0.61618193f,    -0.59937693f,    -0.58281737f,    -0.56649300f,    -0.55039406f,
    -0.53451114f,    -0.51883523f,    -0.50335770f,    -0.48807021f,    -0.47296478f,    -0.45803368f,    -0.44326951f,    -0.42866511f,
    -0.41421356f,    -0.39990820f,    -0.38574257f,    -0.37171042f,    -0.35780572f,    -0.34402260f,    -0.33035538f,    -0.31679853f,
    -0.30334668f,    -0.28999463f,    -0.27673727f,    -0.26356966f,    -0.25048696f,    -0.23748445f,    -0.22455751f,    -0.21170162f,
    -0.19891237f,    -0.18618540f,    -0.17351646f,    -0.16090136f,    -0.14833599f,    -0.13581628f,    -0.12333824f,    -0.11089791f,
    -0.09849140f,    -0.08611485f,    -0.07376443f,    -0.06143635f,    -0.04912685f,    -0.03683218f,    -0.02454862f,    -0.01227246f,
    -0.00000000f,    0.01227246f,    0.02454862f,    0.03683218f,    0.04912685f,    0.06143635f,    0.07376443f,    0.08611485f,
    0.09849140f,    0.11089791f,    0.12333824f,    0.13581628f,    0.14833599f,    0.16090136f,    0.17351646f,    0.18618540f,
    0.19891237f,    0.21170162f,    0.22455751f,    0.23748445f,    0.25048696f,    0.26356966f,    0.27673727f,    0.28999463f,
    0.30334668f,    0.31679853f,    0.33035538f,    0.34402260f,    0.35780572f,    0.37171042f,    0.38574257f,    0.39990820f,
    0.41421356f,    0.42866511f,    0.44326951f,    0.45803368f,    0.47296478f,    0.48807021f,    0.50335770f,    0.51883523f,
    0.53451114f,    0.55039406f,    0.56649300f,    0.58281737f,    0.59937693f,    0.61618193f,    0.63324302f,    0.65057136f,
    0.66817864f,    0.68607707f,    0.70427946f,    0.72279925f,    0.74165055f,    0.76084816f,    0.78040766f,    0.80034545f,
    0.82067879f,    0.84142588f,    0.86260593f,    0.88423922f,    0.90634717f,    0.92895247f,    0.95207915f,    0.97575265f,
    1.00000000f,    1.02484989f,    1.05033285f,    1.07648134f,    1.10332998f,    1.13091569f,    1.15927791f,    1.18845880f,
    1.21850353f,    1.24946047f,    1.28138158f,    1.31432270f,    1.34834391f,    1.38351001f,    1.41989090f,    1.45756220f,
    1.49660576f,    1.53711039f,    1.57917257f,    1.62289733f,    1.66839921f,    1.71580337f,    1.76524687f,    1.81688009f,
    1.87086841f,    1.92739416f,    1.98665879f,    2.04888553f,    2.11432236f,    2.18324555f,    2.25596385f,    2.33282340f,
    2.41421356f,    2.50057389f,    2.59240252f,    2.69026624f,    2.79481277f,    2.90678576f,    3.02704320f,    3.15658033f,
    3.29655821f,    3.44833976f,    3.61353568f,    3.79406340f,    3.99222378f,    4.21080203f,    4.45320222f,    4.72362933f,
    5.02733949f,    5.37099044f,    5.76314201f,    6.21498777f,    6.74145241f,    7.36288764f,    8.10778580f,    9.01730236f,
    10.15317039f,    11.61239886f,    13.55666924f,    16.27700796f,    20.35546762f,    27.15017067f,    40.73548387f,    81.48324021f,
    QF_TAN_MAX,    -81.48324021f,    -40.73548387f,    -27.15017067f,    -20.35546762f,    -16.27700796f,    -13.55666924f,    -11.61239886f,
    -10.15317039f,    -9.01730236f,    -8.10778580f,    -7.36288764f,    -6.74145241f,    -6.21498777f,    -5.76314201f,    -5.37099044f,
    -5.02733949f,    -4.72362933f,    -4.45320222f,    -4.21080203f,    -3.99222378f,    -3.79406340f,    -3.61353568f,    -3.44833976f,
    -3.29655821f,    -3.15658033f,    -3.02704320f,    -2.90678576f,    -2.79481277f,    -2.69026624f,    -2.59240252f,    -2.50057389f,
    -2.41421356f,    -2.33282340f,    -2.25596385f,    -2.18324555f,    -2.11432236f,    -2.04888553f,    -1.98665879f,    -1.92739416f,
    -1.87086841f,    -1.81688009f,    -1.76524687f,    -1.71580337f,    -1.66839921f,    -1.62289733f,    -1.57917257f,    -1.53711039f,
    -1.49660576f,    -1.45756220f,    -1.41989090f,    -1.38351001f,    -1.34834391f,    -1.31432270f,    -1.28138158f,    -1.24946047f,
    -1.21850353f,    -1.18845880f,    -1.15927791f,    -1.13091569f,    -1.10332998f,    -1.07648134f,    -1.05033285f,    -1.02484989f,
    -1.00000000f,    -0.97575265f,    -0.95207915f,    -0.92895247f,    -0.90634717f,    -0.88423922f,    -0.86260593f,    -0.84142588f,
    -0.82067879f,    -0.80034545f,    -0.78040766f,    -0.76084816f,    -0.74165055f,    -0.72279925f,    -0.70427946f,    -0.68607707f,
    -0.66817864f,    -0.65057136f,    -0.63324302f,    -0.61618193f,    -0.59937693f,    -0.58281737f,    -0.56649300f,    -0.55039406f,
    -0.53451114f,    -0.51883523f,    -0.50335770f,    -0.48807021f,    -0.47296478f,    -0.45803368f,    -0.44326951f,    -0.42866511f,
    -0.41421356f,    -0.39990820f,    -0.38574257f,    -0.37171042f,    -0.35780572f,    -0.34402260f,    -0.33035538f,    -0.31679853f,
    -0.30334668f,    -0.28999463f,    -0.27673727f,    -0.26356966f,    -0.25048696f,    -0.23748445f,    -0.22455751f,    -0.21170162f,
    -0.19891237f,    -0.18618540f,    -0.17351646f,    -0.16090136f,    -0.14833599f,    -0.13581628f,    -0.12333824f,    -0.11089791f,
    -0.09849140f,    -0.08611485f,    -0.07376443f,    -0.06143635f,    -0.04912685f,    -0.03683218f,    -0.02454862f,    -0.01227246f,
    0.00000000f
};

/*=======================================================
 * Internal helpers
 */

/* Construct 2^n as a float using IEEE 754 bit layout.
 * Valid for n in [-126, 127]. */
static qf make_pow2i(int32_t n)
{
    if (n < -126) return 0.0f;
    if (n >  127) return 3.4028235e+38f;
    union { qf f; uint32_t u; } v;
    v.u = (uint32_t)(n + 127) << 23;
    return v.f;
}

/* Floor toward negative infinity (no libm). */
static int32_t qf_ifloor(qf x)
{
    int32_t i = (int32_t)x;
    return (x < (qf)i) ? (i - 1) : i;
}

/* Positive reciprocal without emitting a floating-point divide. */
static qf qf_inv_pos(qf x)
{
    union { qf f; uint32_t u; } v;
    v.f = x;
    v.u = 0x7EF311C3u - v.u;

    qf y = v.f;
    y = y * (2.0f - x * y);
    y = y * (2.0f - x * y);
    y = y * (2.0f - x * y);
    return y;
}

/* Good enough for atan argument reduction; the cubic approximation dominates error. */
static qf qf_inv_pos_atan(qf x)
{
    union { qf f; uint32_t u; } v;
    v.f = x;
    v.u = 0x7EF311C3u - v.u;

    qf y = v.f;
    y = y * (2.0f - x * y);
    y = y * (2.0f - x * y);
    return y;
}

static qf qf_inv_pos_atan_fast(qf x)
{
    union { qf f; uint32_t u; } v;
    v.f = x;
    v.u = 0x7EF311C3u - v.u;

    qf y = v.f;
    y = y * (2.0f - x * y);
    return y;
}

#if !QF_MATH_LEAN_BUILD
static qf qf_inv_pos_up(qf x)
{
    union { qf f; uint32_t u; } v;
    v.f = qf_inv_pos(x);
    if (v.u < 0x7F7FFFFFu)
        v.u++;
    return v.f;
}
#endif

#if defined(QF_MATH_COVERAGE)
static qf qf_wrap_bam_units(qf b)
{
    if (b >= QF_BAM_CYCLE) {
        b -= (qf)((int32_t)(b * QF_INV_BAM_CYCLE)) * QF_BAM_CYCLE;
        if (b >= QF_BAM_CYCLE) b -= QF_BAM_CYCLE;
    } else if (b < 0.0f) {
        b += (qf)((int32_t)(-b * QF_INV_BAM_CYCLE) + 1) * QF_BAM_CYCLE;
        if (b >= QF_BAM_CYCLE) b -= QF_BAM_CYCLE;
        if (b < 0.0f) b += QF_BAM_CYCLE;
    }
    return b;
}

/* Coverage/test hook keeps the public old reducer behavior observable. */
static qf reduce_to_twopi(qf r)
{
    return qf_wrap_bam_units(r * QF_BAM_PER_RAD) * (QF_TWO_PI * QF_INV_BAM_CYCLE);
}
#endif

#define QF_SIN_LOOKUP_FROM_PHASE(out_, phase_)                  \
    do {                                                        \
        uint32_t qf_phase_ = (phase_) & QF_BAM_PHASE_MASK;      \
        uint32_t qf_idx_ =                                  \
            (qf_phase_ >> QF_TRIG_PHASE_FRAC_BITS) & QF_TRIG_CYCLE_MASK; \
        uint32_t qf_frac_ = qf_phase_ & QF_TRIG_PHASE_FRAC_MASK; \
        qf qf_lo_ = gSIN_TAB[qf_idx_++];                        \
        qf qf_hi_ = gSIN_TAB[qf_idx_];                          \
        (out_) = qf_lo_ + (qf_hi_ - qf_lo_) *                   \
                 ((qf)qf_frac_ * QF_TRIG_PHASE_FRAC_SCALE);     \
    } while (0)

static inline qf qf_tan_bam_phase(uint32_t phase)
{
    if (phase > (14336u << QF_BAM_UNIT_FRAC_BITS) && phase < (18432u << QF_BAM_UNIT_FRAC_BITS)) {
        qf d = (qf)((int32_t)phase - (int32_t)(16384u << QF_BAM_UNIT_FRAC_BITS)) * QF_INV_BAM_UNIT_SCALE;
        if (d == 0.0f) return QF_TAN_MAX;
        qf ad = (d < 0.0f) ? -d : d;
        qf a = ad * QF_RAD_PER_BAM;
        qf raw = qf_inv_pos(a) - a * 0.33333334f;
        if (raw > QF_TAN_MAX) raw = QF_TAN_MAX;
        return (d < 0.0f) ? raw : -raw;
    }

    if (phase > (47104u << QF_BAM_UNIT_FRAC_BITS) && phase < (51200u << QF_BAM_UNIT_FRAC_BITS)) {
        qf d = (qf)((int32_t)phase - (int32_t)(49152u << QF_BAM_UNIT_FRAC_BITS)) * QF_INV_BAM_UNIT_SCALE;
        if (d == 0.0f) return QF_TAN_MAX;
        qf ad = (d < 0.0f) ? -d : d;
        qf a = ad * QF_RAD_PER_BAM;
        qf raw = qf_inv_pos(a) - a * 0.33333334f;
        if (raw > QF_TAN_MAX) raw = QF_TAN_MAX;
        return (d < 0.0f) ? raw : -raw;
    }

    uint32_t idx  = (phase >> QF_TRIG_PHASE_FRAC_BITS) & QF_TRIG_CYCLE_MASK;
    uint32_t frac = phase & QF_TRIG_PHASE_FRAC_MASK;
    qf lo = gTAN_TAB[idx++];
    qf hi = gTAN_TAB[idx];
    return lo + (hi - lo) * ((qf)frac * QF_TRIG_PHASE_FRAC_SCALE);
}

/*=======================================================
 * BAM-native trig
 */

qf qf_sin_bam(uint16_t bam)
{
    qf out;
    QF_SIN_LOOKUP_FROM_PHASE(out, (uint32_t)bam << QF_BAM_UNIT_FRAC_BITS);
    return out;
}

qf qf_cos_bam(uint16_t bam)
{
    qf out;
    QF_SIN_LOOKUP_FROM_PHASE(out, ((uint32_t)bam + 16384u) << QF_BAM_UNIT_FRAC_BITS);
    return out;
}

qf qf_tan_bam(uint16_t bam)
{
    return qf_tan_bam_phase((uint32_t)bam << QF_BAM_UNIT_FRAC_BITS);
}

/*=======================================================
 * Radian-input trig
 */

qf qf_sin(qf rad)
{
    qf out;
    uint32_t phase = (uint32_t)((int32_t)(rad * QF_BAM_PER_RAD * QF_BAM_UNIT_SCALE));
    QF_SIN_LOOKUP_FROM_PHASE(out, phase);
    return out;
}

qf qf_cos(qf rad)
{
    qf out;
    uint32_t phase = (uint32_t)((int32_t)(rad * QF_BAM_PER_RAD * QF_BAM_UNIT_SCALE)) +
                     (16384u << QF_BAM_UNIT_FRAC_BITS);
    QF_SIN_LOOKUP_FROM_PHASE(out, phase);
    return out;
}

qf qf_tan(qf rad)
{
    uint32_t phase = (uint32_t)((int32_t)(rad * QF_BAM_PER_RAD * QF_BAM_UNIT_SCALE)) & QF_BAM_PHASE_MASK;
    return qf_tan_bam_phase(phase);
}

/*=======================================================
 * Degree-input trig
 */

qf qf_sin_deg(qf deg)
{
    qf out;
    uint32_t phase = (uint32_t)((int32_t)(deg * QF_BAM_PER_DEG * QF_BAM_UNIT_SCALE));
    QF_SIN_LOOKUP_FROM_PHASE(out, phase);
    return out;
}

qf qf_cos_deg(qf deg)
{
    qf out;
    uint32_t phase = (uint32_t)((int32_t)(deg * QF_BAM_PER_DEG * QF_BAM_UNIT_SCALE)) +
                     (16384u << QF_BAM_UNIT_FRAC_BITS);
    QF_SIN_LOOKUP_FROM_PHASE(out, phase);
    return out;
}

qf qf_tan_deg(qf deg)
{
    uint32_t phase = (uint32_t)((int32_t)(deg * QF_BAM_PER_DEG * QF_BAM_UNIT_SCALE)) & QF_BAM_PHASE_MASK;
    return qf_tan_bam_phase(phase);
}

/*=======================================================
 * Inverse trig
 *
 * asin(|x|): three C¹ cubic Hermite spans on [0, 3/4] (knots 0, 1/4, 1/2, 3/4);
 * on (3/4, 1) use asin(ax)=atan(ax/sqrt(1-ax²)) via sqrt((1-ax)*(1+ax)) (avoids Hermite on
 * a span where asin'(u) explodes toward u=1 — cubic overshoot otherwise). Above 0.9975 still
 * use pi/2 - sqrt(2(1-|x|)) with acos semantics; tiny |x|: ~x. acos folds sign + HALF_PI - asin(|x|).
 * atan: six quadratic spans on [0,1]; reciprocal reduction atan(x)=pi/2-atan(1/x) when x>1.
 */

#define ASIN_SMALL_ABSX 0.084f
#define ASIN_TAIL_THR   0.9975f
#define ASIN_ATAN_CROSS  0.75f

/* atan unit poly ---------------------------------------------------------*/

static inline qf qf_atan_unit(qf x)
{
    qf x2 = x * x;
    qf p = 0.02083510f;
    p = p * x2 - 0.08513300f;
    p = p * x2 + 0.18014100f;
    p = p * x2 - 0.33029950f;
    p = p * x2 + 0.99986600f;
    return x * p;
}

/* nonnegative asin(ax), ax in [0, 1]; shared by acos and asin. */
static inline qf qf_asin_pos_kernel(qf ax)
{
    if (ax <= 0.0f)
        return 0.0f;
    if (ax >= 1.0f)
        return QF_HALF_PI;
    /* Same tail as legacy acos: asin(ax) ~= pi/2 - sqrt(2(1-ax)), ax→1^- */
    if (ax > ASIN_TAIL_THR)
        return QF_HALF_PI - qf_sqrt(2.0f * (1.0f - ax));
    if (ax < ASIN_SMALL_ABSX)
        return ax;

    if (ax > ASIN_ATAN_CROSS) {
        /*
         * asin(ax) = atan(ax / sqrt(1-ax²)), nonnegative branch.
         * 1-ax² = (1-ax)*(1+ax) — stable away from catastrophic cancel in (1-x*x) alone near 1,
         * and fast atan(...) replaces ill-conditioned Hermite when asin'(u) is large for u≈3/4..1.
         */
        qf w = qf_sqrt((1.0f - ax) * (1.0f + ax));
        qf scaled = ax * qf_inv_pos(w);
        return qf_atan(scaled);
    }

    qf a, da, y0, m0, y1, m1;

    if (ax <= 0.25f) {
        a = 0.0f;
        da = 0.25f;
        y0 = 0.0f;
        m0 = 1.0f;
        y1 = 0.25268024f;
        m1 = 1.03279555f;
    } else if (ax <= 0.5f) {
        a = 0.25f;
        da = 0.25f;
        y0 = 0.25268024f;
        m0 = 1.03279555f;
        y1 = 0.52359879f;
        m1 = 1.15470052f;
    } else {
        /* (0.5, 0.75] */
        a = 0.5f;
        da = 0.25f;
        y0 = 0.52359879f;
        m0 = 1.15470052f;
        y1 = 0.84806210f;
        m1 = 1.51185787f;
    }

    qf t = (ax - a) * qf_inv_pos(da);
    qf t2 = t * t;
    qf t3 = t2 * t;
    qf dm0 = da * m0;
    qf dm1 = da * m1;
    qf h00 = (2.0f * t3 - 3.0f * t2 + 1.0f);
    qf h10 = (t3 - 2.0f * t2 + t);
    qf h01 = (-2.0f * t3 + 3.0f * t2);
    qf h11 = (t3 - t2);
    return h00 * y0 + h10 * dm0 + h01 * y1 + h11 * dm1;
}

qf qf_acos(qf x)
{
    if (x >=  1.0f) return 0.0f;
    if (x <= -1.0f) return QF_PI;

    int sign = (x < 0.0f) ? 1 : 0;
    qf ax = sign ? -x : x;

    qf asin_angle = qf_asin_pos_kernel(ax);
    qf result = QF_HALF_PI - asin_angle;

    return sign ? (QF_PI - result) : result;
}

qf qf_asin(qf x)
{
    if (x >=  1.0f) return  QF_HALF_PI;
    if (x <= -1.0f) return -QF_HALF_PI;

    int neg = x < 0.0f;
    qf ax = neg ? -x : x;
    qf r = qf_asin_pos_kernel(ax);
    return neg ? -r : r;
}

qf qf_atan(qf x)
{
    if (x == 0.0f)
        return x;

    int32_t neg = x < 0.0f;
    if (neg)
        x = -x;

    if (x >= QF_TAN_MAX)
        return neg ? -QF_HALF_PI : QF_HALF_PI;

    int32_t recip = x > 1.0f;
    if (recip)
        x = qf_inv_pos_atan(x);

    qf result = qf_atan_unit(x);
    if (recip)
        result = QF_HALF_PI - result;
    return neg ? -result : result;
}

qf qf_atan2(qf y, qf x)
{
    /* Axis cases */
    if (x == 0.0f) {
        if (y > 0.0f) return  QF_HALF_PI;
        if (y < 0.0f) return -QF_HALF_PI;
        return 0.0f;
    }
    if (y == 0.0f)
        return (x > 0.0f) ? 0.0f : QF_PI;

    qf ax = (x < 0.0f) ? -x : x;
    qf ay = (y < 0.0f) ? -y : y;

    qf q1_angle;
    if (ay == ax) {
        q1_angle = QF_PI * 0.25f;
    } else if (ay < ax) {
        q1_angle = qf_atan_unit(ay * qf_inv_pos_atan_fast(ax));
    } else {
        q1_angle = QF_HALF_PI - qf_atan_unit(ax * qf_inv_pos_atan_fast(ay));
    }

    if (x > 0.0f)
        return (y > 0.0f) ? q1_angle : -q1_angle;
    return (y > 0.0f) ? (QF_PI - q1_angle) : (q1_angle - QF_PI);
}

/*=======================================================
 * Logarithms
 *
 * qf_log2: exponent bits for integer part; fractional part log2(1+t) with
 * t = (m-1) from IEEE mantissa (normal: t = M/2^23) via a degree-7 polynomial
 * in t (zero constant — exact at t=0). Denormals / NaN / Inf use the same
 * float reconstruction of m as the legacy FR_log2 path, then the same poly.
 */

/* 1 / 2^23 — t = (qf)M * QF_INV_POW23 for normal finite x. */
#define QF_INV_POW23  1.19209290e-7f

/* log2(1+t) on t in [0, 1); Chebyshev-weighted LS, ~4.5e-7 max error vs float ref. */
static inline qf qf_log2_1p_poly(qf t)
{
    qf r = 0.0151275283f;
    r = r * t - 0.0781569619f;
    r = r * t + 0.1923834968f;
    r = r * t - 0.3246154315f;
    r = r * t + 0.4731130529f;
    r = r * t - 0.7205154732f;
    r = r * t + 1.4426640445f;
    return t * r;
}

qf qf_log2(qf x)
{
    if (x <= 0.0f) return QF_DOMAIN_ERROR;

    union { qf f; uint32_t u; } v;
    v.f = x;
    uint32_t ux = v.u;
    uint32_t e_biased = ux & 0x7F800000u;
    int32_t exp_raw = (int32_t)((ux >> 23) & 0xFF) - 127;
    uint32_t M = ux & 0x007FFFFFu;

    /* Normal finite: exponent field non-zero — t = M/2^23, log2(m) = log2(1+t). */
    if (e_biased != 0 && e_biased < 0x7F800000u) {
        qf t = (qf)M * QF_INV_POW23;
        return (qf)exp_raw + qf_log2_1p_poly(t);
    }

    /* Legacy path: subnormals, ±0 residue, NaN, Inf — same reconstruction as FR_log2. */
    v.u = (ux & 0x007FFFFFu) | 0x3F800000u;
    qf m = v.f;

    qf m_frac = m - 1.0f;
    return (qf)exp_raw + qf_log2_1p_poly(m_frac);
}

qf qf_ln(qf x)
{
    qf r = qf_log2(x);
    if (r == QF_DOMAIN_ERROR) return QF_DOMAIN_ERROR;
    return r * QF_LN2;
}

#if !QF_MATH_LEAN_BUILD
qf qf_log10(qf x)
{
    qf r = qf_log2(x);
    if (r == QF_DOMAIN_ERROR) return QF_DOMAIN_ERROR;
    return r * QF_LOG10_2;
}
#endif

/*=======================================================
 * Exponentials
 *
 * qf_pow2: round-to-nearest split x = n + f with |f| <= 0.5 via the float
 * mantissa-offset trick (1.5*2^23), degree-5 Horner approximation to 2^f on
 * [-0.5, 0.5], then add n to IEEE exponent bits when in range — otherwise
 * p * make_pow2i(n). If the split is invalid (e.g. pathological floats), falls
 * back to floor + 2^[0,1) via same poly scaled for [0.5, 1).
 */

#define QF_POW2_MAGIC    12582912.0f          /* float32(3 * 2^22): round-to-nearest int */
#define QF_POW2_MAGIC_U  0x4B400000u         /* bitwise of QF_POW2_MAGIC */

/* MiniMax-style 2^f on [-0.5, 0.5]; max error ~2.5e-6 vs double 2^f. */
static inline qf qf_exp2_poly_half(qf f)
{
    qf p = 0.001333355f;
    p = p * f + 0.009618129f;
    p = p * f + 0.055504109f;
    p = p * f + 0.240226507f;
    p = p * f + 0.693147181f;
    return p * f + 1.0f;
}

/* Fast 2^frac for frac in [0, 1), using symmetry about 0.5. */
static qf qf_exp2_frac_01(qf frac)
{
    if (frac > 0.5f)
        return 2.0f * qf_exp2_poly_half(frac - 1.0f);
    return qf_exp2_poly_half(frac);
}

qf qf_pow2(qf x)
{
    union { qf f; uint32_t u; } vx;
    vx.f = x;
    uint32_t xbits = vx.u;
    uint32_t exp_x = xbits & 0x7F800000u;
    uint32_t mant_x = xbits & 0x007FFFFFu;

    /* NaN: unchanged. Inf: pow2(-inf)=0, pow2(+inf)=+inf */
    if (exp_x == 0x7F800000u) {
        if (mant_x != 0)
            return x;
        return (int32_t)(xbits >> 31) ? 0.0f : x;
    }

    union { qf f; uint32_t u; } vn;
    vn.f = x + QF_POW2_MAGIC;
    int32_t n = (int32_t)(vn.u - QF_POW2_MAGIC_U);
    qf f = x - (qf)n;

    qf p;
    if (QF_ABS(f) > 0.502f) {
        /* Magic unreliable (e.g. huge |x|); floor-split + frac in [0,1). */
        n = qf_ifloor(x);
        qf frac = x - (qf)n;
        p = qf_exp2_frac_01(frac);
    } else {
        p = qf_exp2_poly_half(f);
    }

    union { qf f; uint32_t u; } vp;
    vp.f = p;
    uint32_t pb = vp.u;
    uint32_t exp_p = (pb >> 23) & 0xFFu;

    /* Fast path: add n to IEEE exponent if the result stays in normal range.
     * Defensive: also catches subnormal / inf / zero p (exp_p==0 or >=255). */
    int32_t ne = (int32_t)exp_p + n;
    if (ne < 1 || ne > 254)
        return p * make_pow2i(n);

    vp.u = (pb & 0x807FFFFFu) | ((uint32_t)ne << 23);
    return vp.f;
}

qf qf_exp(qf x)
{
    return qf_pow2(x * QF_LOG2E);
}

qf qf_pow(qf x, qf y)
{
    if (x <= 0.0f) return QF_DOMAIN_ERROR;
    return qf_pow2(y * qf_log2(x));
}

#if !QF_MATH_LEAN_BUILD
qf qf_pow10(qf x)
{
    return qf_pow2(x * QF_LOG2_10);
}
#endif

/*=======================================================
 * Square root
 *
 * Uses the classic fast inverse square root (Quake III style)
 * followed by two Newton-Raphson iterations.
 */

qf qf_sqrt(qf x)
{
    if (x < 0.0f) return QF_DOMAIN_ERROR;
    if (x == 0.0f) return 0.0f;

    union { qf f; uint32_t u; } conv;
    conv.f = x;
    conv.u = 0x5F375A86u - (conv.u >> 1);  /* initial 1/sqrt(x) estimate */
    qf y = conv.f;

    /* Two Newton-Raphson iterations for 1/sqrt(x) */
    y = y * (1.5f - 0.5f * x * y * y);
    y = y * (1.5f - 0.5f * x * y * y);

    return x * y;  /* sqrt(x) = x * (1/sqrt(x)) */
}

/*=======================================================
 * Hypot
 */

qf qf_hypot(qf x, qf y)
{
    return qf_sqrt(x * x + y * y);
}

/*=======================================================
 * FR_hypot_fast8 — 8-segment piecewise-linear magnitude approximation.
 *
 * Same algorithm as FR_hypot_fast8 in FR_math.c. The shift-only
 * coefficients are expressed as float constants. ~0.10% peak error.
 *
 * Based on US Patent 6,567,777 B1 (Chatterjee, expired).
 *
 * Coefficient derivation from the shift expressions:
 *   Segment (beta > 0.875):   a = 1 - 1/4 - 1/64 - 1/256       = 0.73046875
 *                              b = 1 - 1/4 - 1/16 - 1/256       = 0.68359375
 *   Segment (0.75, 0.875]:    a = 1 - 1/4 + 1/32 - 1/1024      = 0.78027344
 *                              b = 1/2 + 1/8 + 1/1024 + 1/4096  = 0.62622070
 *   Segment (0.625, 0.75]:    a = 1 - 1/4 + 1/16 + 1/64        = 0.828125
 *                              b = 1/2 + 1/16 + 1/2048          = 0.56298828
 *   Segment (0.5, 0.625]:     a = 1 - 1/8 - 1/512 - 1/4096     = 0.87280273
 *                              b = 1/2 - 1/64 + 1/256 + 1/1024  = 0.48925781
 *   Segment (0.375, 0.5]:     a = 1 - 1/16 - 1/64 - 1/256      = 0.91796875
 *                              b = 1/2 - 1/8 + 1/32 - 1/128     = 0.3984375
 *   Segment (0.25, 0.375]:    a = 1 - 1/16 + 1/64 + 1/512      = 0.95507813
 *                              b = 1/4 + 1/16 - 1/64 + 1/512    = 0.29882813
 *   Segment (0.125, 0.25]:    a = 1 - 1/64 - 1/2048            = 0.98388672
 *                              b = 1/4 - 1/16 - 1/256 + 1/4096  = 0.18383789
 *   Segment [0, 0.125]:       a = 1 - 1/1024                    = 0.99902344
 *                              b = 1/16 - 1/2048                 = 0.06201172
 */
qf qf_hypot_fast2(qf x, qf y)
{
    qf hi, lo;

    if (x < 0.0f) x = -x;
    if (y < 0.0f) y = -y;

    if (x > y) { hi = x; lo = y; }
    else       { hi = y; lo = x; }

    if (hi == 0.0f) return 0.0f;

    if (hi * 0.5f < lo)
        return hi * 0.81376422f + lo * 0.59235915f;
    return hi * 0.98586827f + lo * 0.23606798f;
}

qf qf_hypot_fast8(qf x, qf y)
{
    qf hi, lo;

    if (x < 0.0f) x = -x;
    if (y < 0.0f) y = -y;

    if (x > y) { hi = x; lo = y; }
    else       { hi = y; lo = x; }

    if (hi == 0.0f) return 0.0f;

    if (hi * 0.5f < lo) {
        /* beta in (0.5, 1.0] */
        if (lo > hi * 0.75f) {
            if (lo > hi * 0.875f)
                return hi * 0.73046875f + lo * 0.68359375f;
            else
                return hi * 0.78027344f + lo * 0.62622070f;
        } else {
            if (lo > hi * 0.625f)
                return hi * 0.828125f   + lo * 0.56298828f;
            else
                return hi * 0.87280273f + lo * 0.48925781f;
        }
    } else {
        /* beta in [0, 0.5] */
        if (hi * 0.25f < lo) {
            if (hi * 0.375f < lo)
                return hi * 0.91796875f + lo * 0.3984375f;
            else
                return hi * 0.95507813f + lo * 0.29882813f;
        } else {
            if (hi * 0.125f < lo)
                return hi * 0.98388672f + lo * 0.18383789f;
            else
                return hi * 0.99902344f + lo * 0.06201172f;
        }
    }
}

#if !QF_MATH_LEAN_BUILD
/*=======================================================
 * Wave generators
 *
 * All waves take a u16 BAM phase and return qf in [-1.0, 1.0]
 * (except tri_morph which returns [0.0, 1.0]).
 */

qf qf_wave_sqr(uint16_t phase)
{
    return (phase < 0x8000u) ? 1.0f : -1.0f;
}

qf qf_wave_pwm(uint16_t phase, uint16_t duty)
{
    return (phase < duty) ? 1.0f : -1.0f;
}

qf qf_wave_saw(uint16_t phase)
{
    return ((qf)phase - 32768.0f) * QF_INV_32768;
}

qf qf_wave_tri(uint16_t phase)
{
    qf t;
    if (phase < 0x8000u) {
        if (phase < 0x4000u)
            t = (qf)phase * QF_INV_16384;
        else
            t = (qf)(0x8000u - phase) * QF_INV_16384;
        return (t > 1.0f) ? 1.0f : t;
    } else {
        if (phase < 0xC000u)
            t = (qf)(phase - 0x8000u) * QF_INV_16384;
        else
            t = (qf)(0x10000u - phase) * QF_INV_16384;
        return (t > 1.0f) ? -1.0f : -t;
    }
}

qf qf_wave_tri_morph(uint16_t phase, uint16_t break_point)
{
    if (break_point == 0) break_point = 1;

    if (phase < break_point) {
        return (qf)phase * qf_inv_pos((qf)break_point);
    } else {
        uint32_t span = (uint32_t)0xFFFFu - (uint32_t)break_point;
        if (span == 0) return 1.0f;
        return (qf)((uint32_t)0xFFFFu - (uint32_t)phase) * qf_inv_pos((qf)span);
    }
}

qf qf_wave_noise(uint32_t *state)
{
    uint32_t lsb;
    if (!state) return 0.0f;

    lsb = *state & 1u;
    *state >>= 1;
    if (lsb) *state ^= 0xD0000001u;

    /* Top 16 bits to float in [-1, 1) */
    return ((qf)((*state >> 16) & 0xFFFFu) - 32768.0f) * QF_INV_32768;
}

/*=======================================================
 * ADSR envelope generator
 *
 * Same lifecycle as FR_math's fr_adsr_t but using qf levels
 * in [0.0, 1.0].
 */

void qf_adsr_init(qf_adsr_t *env,
                   uint32_t attack_samples,
                   uint32_t decay_samples,
                   qf sustain_level,
                   uint32_t release_samples)
{
    if (!env) return;

    env->state = QF_ADSR_IDLE;
    env->level = 0.0f;

    if (sustain_level < 0.0f) sustain_level = 0.0f;
    if (sustain_level > 1.0f) sustain_level = 1.0f;
    env->sustain = sustain_level;

    env->attack_inc  = (attack_samples  > 0) ? qf_inv_pos_up((qf)attack_samples) : 1.0f;
    env->decay_dec   = (decay_samples   > 0) ? ((1.0f - sustain_level) * qf_inv_pos_up((qf)decay_samples)) : (1.0f - sustain_level);
    env->release_dec = (release_samples > 0) ? qf_inv_pos_up((qf)release_samples) : 1.0f;
}

void qf_adsr_trigger(qf_adsr_t *env)
{
    if (!env) return;
    env->state = QF_ADSR_ATTACK;
    env->level = 0.0f;
}

void qf_adsr_release(qf_adsr_t *env)
{
    if (!env) return;
    env->state = QF_ADSR_RELEASE;
}

qf qf_adsr_step(qf_adsr_t *env)
{
    if (!env) return 0.0f;

    switch (env->state) {
    case QF_ADSR_ATTACK:
        env->level += env->attack_inc;
        if (env->level >= 1.0f) {
            env->level = 1.0f;
            env->state = QF_ADSR_DECAY;
        }
        break;
    case QF_ADSR_DECAY:
        env->level -= env->decay_dec;
        if (env->level <= env->sustain) {
            env->level = env->sustain;
            env->state = QF_ADSR_SUSTAIN;
        }
        break;
    case QF_ADSR_SUSTAIN:
        env->level = env->sustain;
        break;
    case QF_ADSR_RELEASE:
        env->level -= env->release_dec;
        if (env->level <= 0.0f) {
            env->level = 0.0f;
            env->state = QF_ADSR_IDLE;
        }
        break;
    case QF_ADSR_IDLE:
    default:
        env->level = 0.0f;
        break;
    }

    return env->level;
}

#endif /* !QF_MATH_LEAN_BUILD */

#if defined(QF_MATH_COVERAGE)
qf qf_cov_reduce_to_twopi(qf r)
{
    return reduce_to_twopi(r);
}
qf qf_cov_make_pow2i(int32_t n) { return make_pow2i(n); }
int32_t qf_cov_ifloor(qf x) { return qf_ifloor(x); }
qf qf_cov_exp2_frac_01(qf f) { return qf_exp2_frac_01(f); }
qf qf_cov_asin_pos_kernel(qf ax) { return qf_asin_pos_kernel(ax); }
#endif
