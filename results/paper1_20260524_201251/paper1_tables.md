# Paper 1 Tables

Largest-image subset bytes=196608

## SIMD Build Speedup (largest image)

| case | scalar enc MB/s | simd enc MB/s | enc speedup | scalar dec MB/s | simd dec MB/s | dec speedup |
|---|---:|---:|---:|---:|---:|---:|
| map_fridrich+scalar_chain | 80.6 | 113.2 | 1.40x | 88.7 | 123.1 | 1.39x |
| map_fridrich+scan_exact | 113.3 | 130.8 | 1.16x | 105.2 | 134.6 | 1.28x |
| sort_logistic+scalar_chain | 25.5 | 28.4 | 1.11x | 26.9 | 30.3 | 1.13x |
| sort_logistic+scan_exact | 31.7 | 33.2 | 1.05x | 32.7 | 32.2 | 0.98x |

## Full Results

| build_mode | case | simd_backend | shape | enc MB/s | dec MB/s | correct |
|---|---|---|---|---:|---:|---:|
| scalar | map_fridrich+scalar_chain | scalar | 256x256x3 | 80.6 | 88.7 | 1 |
| scalar | map_fridrich+scan_exact | scalar | 256x256x3 | 113.3 | 105.2 | 1 |
| scalar | sort_logistic+scalar_chain | scalar | 256x256x3 | 25.5 | 26.9 | 1 |
| scalar | sort_logistic+scan_exact | scalar | 256x256x3 | 31.7 | 32.7 | 1 |
| simd | map_fridrich+scalar_chain | scalar | 256x256x3 | 113.2 | 123.1 | 1 |
| simd | map_fridrich+scan_exact | scalar | 256x256x3 | 130.8 | 134.6 | 1 |
| simd | sort_logistic+scalar_chain | scalar | 256x256x3 | 28.4 | 30.3 | 1 |
| simd | sort_logistic+scan_exact | scalar | 256x256x3 | 33.2 | 32.2 | 1 |
