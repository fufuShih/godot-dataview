#include "dataview.h"

#include "data_source_factory.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

String DataView::data_type_name(dataview::DataType p_type) {
	switch (p_type) {
		case dataview::DataType::NIL:
			return "nil";
		case dataview::DataType::BOOLEAN:
			return "boolean";
		case dataview::DataType::INTEGER:
			return "integer";
		case dataview::DataType::FLOAT:
			return "float";
		case dataview::DataType::STRING:
			return "string";
	}
	return "unknown";
}

void DataView::set_result(const dataview::DataResult &p_result) {
	last_error = p_result.is_ok() ? String() : p_result.message;
}

bool DataView::open(const String &p_path, const Dictionary &p_options) {
	close();

	dataview::DataSourceOptions options;
	if (p_options.has("delimiter")) {
		options.csv_delimiter = p_options["delimiter"];
	}
	if (p_options.has("has_header")) {
		options.csv_has_header = p_options["has_header"];
	}

	dataview::DataResult result;
	std::unique_ptr<dataview::IDataSource> candidate =
			dataview::DataSourceFactory::create(p_path, options, result);
	if (!result.is_ok() || candidate == nullptr) {
		set_result(result);
		return false;
	}

	result = candidate->open(p_path);
	if (!result.is_ok()) {
		set_result(result);
		return false;
	}

	source = std::move(candidate);
	set_result(result);
	return true;
}

void DataView::close() {
	if (source != nullptr) {
		source->close();
		source.reset();
	}
	last_error = "";
}

bool DataView::is_open() const {
	return source != nullptr && source->is_open();
}

int64_t DataView::get_capabilities() const {
	return source != nullptr ? source->get_capabilities() : dataview::CAPABILITY_NONE;
}

bool DataView::has_capability(int64_t p_capability) const {
	if (source == nullptr || p_capability < 0) {
		return false;
	}
	return (source->get_capabilities() & static_cast<uint32_t>(p_capability)) != 0;
}

PackedStringArray DataView::get_tables() const {
	PackedStringArray result;
	if (source == nullptr) {
		return result;
	}

	const Vector<String> tables = source->get_tables();
	for (const String &table : tables) {
		result.push_back(table);
	}
	return result;
}

Dictionary DataView::get_schema(const String &p_table) {
	Dictionary result;
	if (source == nullptr) {
		last_error = "No data source is open.";
		return result;
	}

	dataview::DataSchema schema;
	const dataview::DataResult operation = source->get_schema(p_table, schema);
	set_result(operation);
	if (!operation.is_ok()) {
		return result;
	}

	Array columns;
	for (const dataview::DataColumn &column : schema.columns) {
		Dictionary column_data;
		column_data["name"] = column.name;
		column_data["type"] = data_type_name(column.type);
		column_data["type_id"] = static_cast<int64_t>(column.type);
		columns.push_back(column_data);
	}

	result["table"] = schema.table_name;
	result["row_count"] = schema.row_count;
	result["columns"] = columns;
	return result;
}

Dictionary DataView::fetch(const Dictionary &p_query) {
	Dictionary result;
	if (source == nullptr) {
		last_error = "No data source is open.";
		return result;
	}

	if ((p_query.has("filter") && !Dictionary(p_query["filter"]).is_empty()) ||
			(p_query.has("sort") && !Array(p_query["sort"]).is_empty())) {
		last_error = "Filtering and sorting are not implemented for this data source.";
		return result;
	}

	dataview::DataQuery query;
	if (p_query.has("table")) {
		query.table_name = p_query["table"];
	}
	if (p_query.has("offset")) {
		query.offset = p_query["offset"];
	}
	if (p_query.has("limit")) {
		query.limit = p_query["limit"];
	}

	dataview::DataPage page;
	const dataview::DataResult operation = source->fetch(query, page);
	set_result(operation);
	if (!operation.is_ok()) {
		return result;
	}

	PackedStringArray column_names;
	for (const dataview::DataColumn &column : page.columns) {
		column_names.push_back(column.name);
	}

	Array rows;
	for (const dataview::DataRow &source_row : page.rows) {
		Array row;
		for (const Variant &value : source_row.values) {
			row.push_back(value);
		}
		rows.push_back(row);
	}

	result["table"] = page.table_name;
	result["offset"] = page.offset;
	result["count"] = page.rows.size();
	result["total_count"] = page.total_count;
	result["columns"] = column_names;
	result["rows"] = rows;
	return result;
}

bool DataView::save() {
	if (source == nullptr) {
		last_error = "No data source is open.";
		return false;
	}

	const dataview::DataResult operation = source->save();
	set_result(operation);
	return operation.is_ok();
}

String DataView::get_last_error() const {
	return last_error;
}

void DataView::_bind_methods() {
	ClassDB::bind_method(D_METHOD("open", "path", "options"), &DataView::open, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("close"), &DataView::close);
	ClassDB::bind_method(D_METHOD("is_open"), &DataView::is_open);
	ClassDB::bind_method(D_METHOD("get_capabilities"), &DataView::get_capabilities);
	ClassDB::bind_method(D_METHOD("has_capability", "capability"), &DataView::has_capability);
	ClassDB::bind_method(D_METHOD("get_tables"), &DataView::get_tables);
	ClassDB::bind_method(D_METHOD("get_schema", "table"), &DataView::get_schema, DEFVAL(String()));
	ClassDB::bind_method(D_METHOD("fetch", "query"), &DataView::fetch, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("save"), &DataView::save);
	ClassDB::bind_method(D_METHOD("get_last_error"), &DataView::get_last_error);

	ClassDB::bind_integer_constant(get_class_static(), "Capability", "READ", READ);
	ClassDB::bind_integer_constant(get_class_static(), "Capability", "WRITE", WRITE);
	ClassDB::bind_integer_constant(get_class_static(), "Capability", "MULTI_TABLE", MULTI_TABLE);
	ClassDB::bind_integer_constant(get_class_static(), "Capability", "QUERY", QUERY);
	ClassDB::bind_integer_constant(get_class_static(), "Capability", "TRANSACTION", TRANSACTION);
	ClassDB::bind_integer_constant(get_class_static(), "Capability", "SCHEMA_EDIT", SCHEMA_EDIT);
}
