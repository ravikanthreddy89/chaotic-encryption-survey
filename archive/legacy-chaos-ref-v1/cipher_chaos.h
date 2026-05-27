#pragma once
/**
 * cipher_chaos.h
 * Bucketed chaos-cipher interface for the benchmark suite.
 *
 * Buckets:
 *  - 1D discrete-map baseline: Fridrich-1998
 *  - 2D/transcendental coupled-map family: YeHuang-2018, LSCM-2020
 */

#include "chaos/chaos_common.h"
#include "chaos/chaos_fridrich.h"
#include "chaos/chaos_multid.h"
