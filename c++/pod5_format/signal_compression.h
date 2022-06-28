#ifndef SIGNAL_COMPRESSION_H
#define SIGNAL_COMPRESSION_H
/*
#pragma once

#include "pod5_format/pod5_format_export.h"
#include "pod5_format/result.h"

#include <gsl/gsl-lite.hpp>

namespace arrow {
class MemoryPool;
class Buffer;
}  // namespace arrow

namespace pod5 {

using SampleType = std::int16_t;
*/

size_t compressed_signal_max_size(size_t sample_count);

uint8_t *compress_signal(
        const int16_t *samples,
	size_t count,
        size_t *n);

int16_t *decompress_signal(
        const uint8_t *compressed_bytes,
        size_t count,
        size_t *n);

/*}  // namespace pod5*/
#endif /* signal_compression.h */
