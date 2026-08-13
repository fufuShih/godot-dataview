#ifndef DATAVIEW_DATA_TYPES_H
#define DATAVIEW_DATA_TYPES_H

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <cstdint>

namespace godot::dataview {

enum class DataType : int32_t {
	NIL,
	BOOLEAN,
	INTEGER,
	FLOAT,
	STRING,
};

enum DataSourceCapability : uint32_t {
	CAPABILITY_NONE = 0,
	CAPABILITY_READ = 1U << 0,
	CAPABILITY_WRITE = 1U << 1,
	CAPABILITY_MULTI_TABLE = 1U << 2,
	CAPABILITY_QUERY = 1U << 3,
	CAPABILITY_TRANSACTION = 1U << 4,
	CAPABILITY_SCHEMA_EDIT = 1U << 5,
};

using CapabilityMask = uint32_t;
using DataValue = Variant;

struct DataResult {
	Error error = OK;
	String message;

	bool is_ok() const {
		return error == OK;
	}

	static DataResult success() {
		return {};
	}

	static DataResult failure(Error p_error, const String &p_message) {
		DataResult result;
		result.error = p_error;
		result.message = p_message;
		return result;
	}
};

struct DataColumn {
	String name;
	DataType type = DataType::STRING;
};

struct DataSchema {
	String table_name;
	Vector<DataColumn> columns;
	int64_t row_count = 0;
};

struct DataRow {
	Vector<DataValue> values;
};

struct DataTable {
	DataSchema schema;
	Vector<DataRow> rows;
};

struct DataQuery {
	String table_name;
	int64_t offset = 0;
	int64_t limit = 100;
};

struct DataPage {
	String table_name;
	int64_t offset = 0;
	int64_t total_count = 0;
	Vector<DataColumn> columns;
	Vector<DataRow> rows;
};

struct DataSourceOptions {
	String csv_delimiter = ",";
	bool csv_has_header = true;
};

inline bool has_capability(CapabilityMask p_capabilities, DataSourceCapability p_capability) {
	return (p_capabilities & static_cast<CapabilityMask>(p_capability)) != 0;
}

} // namespace godot::dataview

#endif
