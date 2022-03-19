#include "mkr_format/signal_table_schema.h"

#include "mkr_format/types.h"

#include <arrow/type.h>

namespace mkr {

std::shared_ptr<arrow::Schema> make_signal_table_schema(
        std::shared_ptr<const arrow::KeyValueMetadata> const& metadata,
        SignalTableSchemaDescription* field_locations) {
    auto const uuid_type = uuid();

    if (field_locations) {
        *field_locations = {};
    }

    return arrow::schema(
            {
                    arrow::field("read_id", uuid_type),
                    arrow::field("signal", arrow::large_list(arrow::int16())),
                    arrow::field("samples", arrow::uint32()),
            },
            metadata);
}

Result<SignalTableSchemaDescription> read_signal_table_schema(
        std::shared_ptr<arrow::Schema> const& schema) {
    auto read_id_field_idx = schema->GetFieldIndex("read_id");
    {
        if (read_id_field_idx == -1) {
            return Status::TypeError("Schema missing field 'read_id'");
        }
        auto read_id_field = schema->field(read_id_field_idx);
        auto read_id_type = read_id_field->type();
        if (read_id_type->id() != arrow::Type::EXTENSION) {
            return Status::TypeError("Schema field 'read_id' is incorrect type: '",
                                     read_id_type->name(), "'");
        }
        auto read_id_extension_field = static_cast<arrow::ExtensionType*>(read_id_type.get());
        if (read_id_extension_field->extension_name() != "minknow.uuid") {
            return Status::TypeError("Schema field 'read_id' is incorrect extension type");
        }
    }

    auto signal_field_idx = schema->GetFieldIndex("signal");
    {
        if (signal_field_idx == -1) {
            return Status::TypeError("Schema missing field 'signal'");
        }
        auto signal_field = schema->field(signal_field_idx);
        auto signal_type = signal_field->type();
        if (signal_type->id() != arrow::Type::LARGE_LIST) {
            return Status::TypeError("Schema field 'signal' is incorrect type: '",
                                     signal_type->name(), "'");
        }
        auto signal_list_field = static_cast<arrow::LargeListType*>(signal_field->type().get());
        if (signal_list_field->value_type()->id() != arrow::Type::INT16) {
            return Status::TypeError("Schema field 'signal' list value type is incorrect type");
        }
    }

    auto samples_field_idx = schema->GetFieldIndex("samples");
    {
        if (samples_field_idx == -1) {
            return Status::TypeError("Schema missing field 'samples'");
        }
        auto samples_field = schema->field(samples_field_idx);
        auto samples_type = samples_field->type();
        if (samples_field->type()->id() != arrow::Type::UINT32) {
            return Status::TypeError("Schema field 'samples' is incorrect type: '",
                                     samples_type->name(), "'");
        }
    }

    return SignalTableSchemaDescription{read_id_field_idx, signal_field_idx, samples_field_idx};
}

}  // namespace mkr