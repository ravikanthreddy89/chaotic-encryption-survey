# Finite-Precision Orbit Probe

Probe settings:

- Warmup: 500
- Samples: 16
- Max steps per sample: 1,000,000
- Quantization level: 16 bits

## Summary

- `logistic_double`: 0/16 exact repeats within the 1,000,000-step horizon.
- `logistic_q16`: 16/16 quantized repeats within the horizon.
- `fixed_point_tent`: 0/16 exact repeats within the horizon.

## Quantized logistic results

- First repeat step range: 35 to 549
- Median first repeat step: 161
- Cycle length range: 5 to 245
- Median cycle length: 60

Interpretation:

- Exact double precision gives only a lower bound on orbit length at the current horizon.
- Coarse 16-bit quantization collapses the effective orbit quickly and consistently.
- The current fixed-point tent probe did not repeat within the same bound, so the report should stay conservative about that path.
