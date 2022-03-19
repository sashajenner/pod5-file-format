#pragma once

#include "mkr_format/mkr_format_export.h"
#include "mkr_format/result.h"
#include "mkr_format/signal_table_schema.h"

#include <arrow/io/type_fwd.h>
#include <boost/uuid/uuid.hpp>
#include <gsl/gsl-lite.hpp>

namespace arrow {
class Schema;
namespace io {
class OutputStream;
}
namespace ipc {
class RecordBatchWriter;
}
}  // namespace arrow

namespace mkr {

class MKR_FORMAT_EXPORT SignalTableWriter {
public:
    SignalTableWriter(std::shared_ptr<arrow::ipc::RecordBatchWriter>&& writer,
                      std::shared_ptr<arrow::Schema>&& schema,
                      SignalTableSchemaDescription const& field_locations,
                      arrow::MemoryPool* pool);
    SignalTableWriter(SignalTableWriter&&);
    SignalTableWriter& operator=(SignalTableWriter&&);
    SignalTableWriter(SignalTableWriter const&) = delete;
    SignalTableWriter& operator=(SignalTableWriter const&) = delete;
    ~SignalTableWriter();

    /// \brief Add an array of reads to the signal table, as a batch.
    /// \param read_id The read id for the read entry
    /// \param signal The signal for the read entry
    Result<std::size_t> add_read(boost::uuids::uuid const& read_id,
                                 gsl::span<std::int16_t> const& signal);

    /// \brief Flush buffered data into the writer as a record batch.
    Status flush();

    /// \brief Close this writer, signaling no further data will be written to the writer.
    Status close();

private:
    arrow::MemoryPool* m_pool = nullptr;
    std::shared_ptr<arrow::Schema> m_schema;
    SignalTableSchemaDescription m_field_locations;

    std::shared_ptr<arrow::io::OutputStream> m_sink;
    std::shared_ptr<arrow::ipc::RecordBatchWriter> m_writer;

    std::unique_ptr<arrow::FixedSizeBinaryBuilder> m_read_id_builder;
    std::shared_ptr<arrow::Int16Builder> m_signal_data_builder;
    std::unique_ptr<arrow::LargeListBuilder> m_signal_builder;
    std::unique_ptr<arrow::UInt32Builder> m_samples_builder;

    std::size_t m_flushed_row_count = 0;
    std::size_t m_current_batch_row_count = 0;
};

/// \brief Make a new writer for a signal table.
/// \param sink Sink to be used for output of the table.
/// \param metadata Metadata to be applied to the table schema.
/// \param pool Pool to be used for building table in memory.
/// \returns The writer for the new table.
Result<SignalTableWriter> make_signal_table_writer(
        std::shared_ptr<arrow::io::OutputStream> const& sink,
        std::shared_ptr<const arrow::KeyValueMetadata> const& metadata,
        arrow::MemoryPool* pool);

}  // namespace mkr