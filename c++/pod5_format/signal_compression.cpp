/*
#include "pod5_format/signal_compression.h"

#include "pod5_format/svb16/decode.hpp"
#include "pod5_format/svb16/encode.hpp"

#include <arrow/buffer.h>
*/
#include <zstd.h>

/*namespace pod5 {*/

size_t compressed_signal_max_size(size_t sample_count) {
    const uint32_t max_svb_size = svb16_max_encoded_length(sample_count);
    const size_t zstd_compressed_max_size = ZSTD_compressBound(max_svb_size);
    return zstd_compressed_max_size;
}

uint8_t *compress_signal(
        const int16_t *samples,
	size_t count,
        size_t *n)
{
    // First compress the data using svb:
    const uint32_t max_size = svb16_max_encoded_length(count);
    /*ARROW_ASSIGN_OR_RAISE(auto intermediate, arrow::AllocateResizableBuffer(max_size, pool));*/
    uint8_t *intermediate = malloc(max_size * sizeof(*intermediate));

    bool UseDelta = true;
    bool UseZigzag = true;
    const size_t encoded_count = svb16::encode<int16_t, UseDelta, UseZigzag>(
            samples, intermediate, count);
    /*ARROW_RETURN_NOT_OK(intermediate->Resize(encoded_count));*/

    // Now compress the svb data using zstd:
    size_t const zstd_compressed_max_size = ZSTD_compressBound(max_size);
    if (ZSTD_isError(zstd_compressed_max_size)) {
        /*return pod5::Status::Invalid("Failed to find zstd max size for data");*/
	free(intermediate);
        return NULL;
    }

    /* Compress.
     * If you are doing many compressions, you may want to reuse the context.
     * See the multiple_simple_compression.c example.
     */
    uint8_t *destination = malloc(zstd_compressed_max_size);
    const size_t compressed_size = ZSTD_compress(destination, zstd_compressed_max_size,
                                                 intermediate, encoded_count, SLOW5_ZSTD_COMPRESS_LEVEL);
    if (ZSTD_isError(compressed_size)) {
        /*return pod5::Status::Invalid("Failed to compress data");*/
	free(intermediate);
	free(destination);
        return NULL;
    }

    *n = compression_size;
    return destination;
}

/*
arrow::Result<std::shared_ptr<arrow::Buffer>> compress_signal(const int16_t *samples) {
    ARROW_ASSIGN_OR_RAISE(
        std::shared_ptr<arrow::ResizableBuffer> out,
        arrow::AllocateResizableBuffer(compressed_signal_max_size(samples.size()), pool));

    ARROW_ASSIGN_OR_RAISE(
        auto final_size,
        compress_signal(samples, pool, gsl::make_span(out->mutable_data(), out->size())));

    ARROW_RETURN_NOT_OK(out->Resize(final_size));
    return out;
}
*/

int16_t *decompress_signal(
        const uint8_t *compressed_bytes,
        size_t count,
        size_t *n)
{
    // First decompress the data using zstd:
    unsigned long long const decompressed_zstd_size =
            ZSTD_getFrameContentSize(compressed_bytes, count);
    if (ZSTD_isError(decompressed_zstd_size)) {
        /*return pod5::Status::Invalid("Input data not compressed by zstd");*/
        return NULL;
    }

    /*
    ARROW_ASSIGN_OR_RAISE(auto intermediate,
                          arrow::AllocateResizableBuffer(decompressed_zstd_size, pool));
    */
    uint8_t *intermediate = malloc(decompressed_zstd_size);
    size_t const decompress_res =
            ZSTD_decompress(intermediate, decompressed_zstd_size,
                            compressed_bytes, count);
    if (ZSTD_isError(decompress_res)) {
        /*return pod5::Status::Invalid("Input data failed to compress using zstd");*/
	free(intermediate);
        return NULL;
    }

    /* TODO get the length of destination somehow as input or read from intermediate */
    int16_t *destination;

    // Now decompress the data using svb:
    bool UseDelta = true;
    bool UseZigzag = true;
    size_t consumed_count = svb16::decode<int16_t, UseDelta, UseZigzag>(
            destination, intermediate, destination);

    if (consumed_count != decompressed_zstd_size) {
        /*return pod5::Status::Invalid("Remaining data at end of signal buffer");*/
	free(intermediate);
	free(destination);
	return NULL;
    }

    return destination;
}

/*
arrow::Result<std::shared_ptr<arrow::Buffer>> decompress_signal(
        gsl::span<std::uint8_t const> const& compressed_bytes,
        std::uint32_t samples_count,
        arrow::MemoryPool* pool) {
    ARROW_ASSIGN_OR_RAISE(std::shared_ptr<arrow::ResizableBuffer> out,
                          arrow::AllocateResizableBuffer(samples_count * sizeof(int16_t), pool));

    auto signal_span = gsl::make_span(out->mutable_data(), out->size()).as_span<std::int16_t>();

    ARROW_RETURN_NOT_OK(decompress_signal(compressed_bytes, pool, signal_span));
    return out;
}
k  // namespace pod5*/
