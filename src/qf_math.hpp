/**
 *  @file qf_math.hpp - C++ convenience wrapper for qf_math.
 *
 *  Header-only facade over the C API in qf_math.h. It does not add storage,
 *  allocation, exceptions, RTTI, or new runtime dependencies.
 */

#ifndef QF_MATH_HPP
#define QF_MATH_HPP

#include "qf_math.h"

namespace qf_math {

using scalar = ::qf;
#if !QF_MATH_LEAN_BUILD
using adsr_t = ::qf_adsr_t;
#endif

static constexpr const char *version = QF_MATH_VERSION;
static constexpr int version_hex = QF_MATH_VERSION_HEX;

static constexpr scalar pi = QF_PI;
static constexpr scalar two_pi = QF_TWO_PI;
static constexpr scalar half_pi = QF_HALF_PI;
static constexpr scalar e = QF_E;
static constexpr scalar inv_e = QF_INV_E;
static constexpr scalar inv_pi = QF_INV_PI;
static constexpr scalar deg2rad_k = QF_DEG2RAD_K;
static constexpr scalar rad2deg_k = QF_RAD2DEG_K;
static constexpr scalar log2e = QF_LOG2E;
static constexpr scalar ln2 = QF_LN2;
static constexpr scalar sqrt2 = QF_SQRT2;
static constexpr scalar tan_max = QF_TAN_MAX;
static constexpr scalar domain_error = QF_DOMAIN_ERROR;

inline scalar from_int(int x) noexcept { return static_cast<scalar>(x); }
inline int to_int(scalar x) noexcept { return static_cast<int>(x); }
inline int round_to_int(scalar x) noexcept { return QF_ROUND(x); }

inline scalar abs(scalar x) noexcept { return QF_ABS(x); }
inline scalar sign(scalar x) noexcept { return QF_SGN(x); }
inline scalar min(scalar a, scalar b) noexcept { return QF_MIN(a, b); }
inline scalar max(scalar a, scalar b) noexcept { return QF_MAX(a, b); }
inline scalar clamp(scalar x, scalar lo, scalar hi) noexcept { return QF_CLAMP(x, lo, hi); }
inline scalar lerp(scalar a, scalar b, scalar t) noexcept { return QF_INTERP(a, b, t); }

inline scalar deg_to_rad(scalar deg) noexcept { return QF_DEG_TO_RAD(deg); }
inline scalar rad_to_deg(scalar rad) noexcept { return QF_RAD_TO_DEG(rad); }
inline scalar bam_to_rad(uint16_t bam) noexcept { return QF_BAM_TO_RAD(bam); }
inline uint16_t rad_to_bam(scalar rad) noexcept { return QF_RAD_TO_BAM(rad); }
inline scalar bam_to_deg(uint16_t bam) noexcept { return QF_BAM_TO_DEG(bam); }
inline uint16_t deg_to_bam(scalar deg) noexcept { return QF_DEG_TO_BAM(deg); }
inline int32_t to_fr(scalar x, int radix) noexcept { return QF_TO_FR(x, radix); }
inline int32_t to_fr_round(scalar x, int radix) noexcept { return QF_TO_FR_RND(x, radix); }
inline scalar from_fr(int32_t x, int radix) noexcept { return FR_TO_QF(x, radix); }

#if !QF_MATH_LEAN_BUILD
inline scalar sin_bam(uint16_t bam) noexcept { return ::qf_sin_bam(bam); }
inline scalar cos_bam(uint16_t bam) noexcept { return ::qf_cos_bam(bam); }
inline scalar tan_bam(uint16_t bam) noexcept { return ::qf_tan_bam(bam); }
#endif
inline scalar sin(scalar rad) noexcept { return ::qf_sin(rad); }
inline scalar cos(scalar rad) noexcept { return ::qf_cos(rad); }
inline scalar tan(scalar rad) noexcept { return ::qf_tan(rad); }
#if !QF_MATH_LEAN_BUILD
inline scalar sin_deg(scalar deg) noexcept { return ::qf_sin_deg(deg); }
inline scalar cos_deg(scalar deg) noexcept { return ::qf_cos_deg(deg); }
inline scalar tan_deg(scalar deg) noexcept { return ::qf_tan_deg(deg); }
#endif

inline scalar acos(scalar x) noexcept { return ::qf_acos(x); }
inline scalar asin(scalar x) noexcept { return ::qf_asin(x); }
inline scalar atan(scalar x) noexcept { return ::qf_atan(x); }
inline scalar atan2(scalar y, scalar x) noexcept { return ::qf_atan2(y, x); }

inline scalar log2(scalar x) noexcept { return ::qf_log2(x); }
inline scalar ln(scalar x) noexcept { return ::qf_ln(x); }
#if !QF_MATH_LEAN_BUILD
inline scalar log10(scalar x) noexcept { return ::qf_log10(x); }
#endif
inline scalar pow2(scalar x) noexcept { return ::qf_pow2(x); }
inline scalar exp(scalar x) noexcept { return ::qf_exp(x); }
inline scalar pow(scalar x, scalar y) noexcept { return ::qf_pow(x, y); }
#if !QF_MATH_LEAN_BUILD
inline scalar pow10(scalar x) noexcept { return ::qf_pow10(x); }
#endif

inline scalar sqrt(scalar x) noexcept { return ::qf_sqrt(x); }
#if !QF_MATH_LEAN_BUILD
inline scalar hypot(scalar x, scalar y) noexcept { return ::qf_hypot(x, y); }
inline scalar hypot_fast2(scalar x, scalar y) noexcept { return ::qf_hypot_fast2(x, y); }
#endif
inline scalar hypot_fast8(scalar x, scalar y) noexcept { return ::qf_hypot_fast8(x, y); }

#if !QF_MATH_LEAN_BUILD
inline uint16_t hz_to_bam_inc(uint32_t hz, uint32_t sample_rate) noexcept
{
    return static_cast<uint16_t>((hz * 65536UL) / sample_rate);
}

inline scalar wave_sqr(uint16_t phase) noexcept { return ::qf_wave_sqr(phase); }
inline scalar wave_pwm(uint16_t phase, uint16_t duty) noexcept { return ::qf_wave_pwm(phase, duty); }
inline scalar wave_tri(uint16_t phase) noexcept { return ::qf_wave_tri(phase); }
inline scalar wave_saw(uint16_t phase) noexcept { return ::qf_wave_saw(phase); }
inline scalar wave_tri_morph(uint16_t phase, uint16_t break_point) noexcept
{
    return ::qf_wave_tri_morph(phase, break_point);
}
inline scalar wave_noise(uint32_t &state) noexcept { return ::qf_wave_noise(&state); }

inline void adsr_init(adsr_t &env,
                      uint32_t attack_samples,
                      uint32_t decay_samples,
                      scalar sustain_level,
                      uint32_t release_samples) noexcept
{
    ::qf_adsr_init(&env, attack_samples, decay_samples, sustain_level, release_samples);
}

inline void adsr_trigger(adsr_t &env) noexcept { ::qf_adsr_trigger(&env); }
inline void adsr_release(adsr_t &env) noexcept { ::qf_adsr_release(&env); }
inline scalar adsr_step(adsr_t &env) noexcept { return ::qf_adsr_step(&env); }

class adsr {
public:
    adsr(uint32_t attack_samples,
         uint32_t decay_samples,
         scalar sustain_level,
         uint32_t release_samples) noexcept
    {
        adsr_init(env_, attack_samples, decay_samples, sustain_level, release_samples);
    }

    void trigger() noexcept { adsr_trigger(env_); }
    void release() noexcept { adsr_release(env_); }
    scalar step() noexcept { return adsr_step(env_); }
    adsr_t &state() noexcept { return env_; }
    const adsr_t &state() const noexcept { return env_; }

private:
    adsr_t env_{};
};
#endif /* !QF_MATH_LEAN_BUILD */

} // namespace qf_math

#endif /* QF_MATH_HPP */
