#include "qf_math.hpp"

int main()
{
    qf_math::scalar angle = qf_math::deg_to_rad(45.0f);
    qf_math::scalar s = qf_math::sin(angle);
    qf_math::scalar c = qf_math::cos(angle);
    qf_math::scalar t = qf_math::tan(angle);
    qf_math::scalar a = qf_math::atan2(s, c);
    qf_math::scalar h = qf_math::hypot_fast2(3.0f, 4.0f);
    qf_math::scalar p = qf_math::pow(2.0f, 3.0f);
    uint16_t bam = qf_math::deg_to_bam(90.0f);

    qf_math::adsr env(1, 1, 0.5f, 1);
    env.trigger();
    qf_math::scalar e = env.step();

    return (s > 0.0f && c > 0.0f && t > 0.0f && a > 0.0f && h > 0.0f && p > 7.9f &&
            bam == 16384u && e >= 0.0f)
               ? 0
               : 1;
}
