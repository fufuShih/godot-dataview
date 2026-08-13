#include "csv_source.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/char_string.hpp>

#include <csv.hpp>

#include <exception>
#include <memory>
#include <sstream>
#include <string>

using namespace godot;
using namespace godot::dataview;

CsvDataSource::CsvDataSource(const String &p_delimiter, bool p_has_header) :
		delimiter(p_delimiter),
		has_header(p_has_header) {
}

DataResult CsvDataSource::open(const String &p_path) {
	close();

	const CharString delimiter_utf8 = delimiter.utf8();
	if (delimiter.length() != 1 || delimiter_utf8.length() != 1) {
		return DataResult::failure(
				ERR_INVALID_PARAMETER,
				"CSV delimiter must be exactly one single-byte character.");
	}

	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
	if (file.is_null()) {
		return DataResult::failure(
				FileAccess::get_open_error(),
				"Unable to open CSV file: " + p_path);
	}

	const String contents = file->get_as_text();
	const CharString contents_utf8 = contents.utf8();
	std::string csv_text(contents_utf8.get_data(), static_cast<size_t>(contents_utf8.length()));

	try {
		csv::CSVFormat format;
		format.delimiter(delimiter_utf8[0]);
		format.variable_columns(csv::VariableColumnPolicy::THROW);
		format.threading(false);

		if (has_header) {
			format.header_row(0);
		} else {
			format.no_header();
		}

		auto stream = std::make_unique<std::istringstream>(std::move(csv_text));
		csv::CSVReader reader(std::move(stream), format);

		table.schema.table_name = p_path.get_file().get_basename();

		if (has_header) {
			const std::vector<std::string> &column_names = reader.get_col_names();
			if (column_names.empty()) {
				close();
				return DataResult::failure(ERR_FILE_CORRUPT, "CSV header is missing or empty.");
			}

			for (const std::string &column_name : column_names) {
				DataColumn column;
				column.name = String::utf8(column_name.data(), static_cast<int64_t>(column_name.size()));
				column.type = DataType::STRING;
				table.schema.columns.push_back(column);
			}
		}

		for (csv::CSVRow &csv_row : reader) {
			if (!has_header && table.schema.columns.is_empty()) {
				for (size_t column_index = 0; column_index < csv_row.size(); ++column_index) {
					DataColumn column;
					column.name = "Column " + String::num_int64(static_cast<int64_t>(column_index + 1));
					column.type = DataType::STRING;
					table.schema.columns.push_back(column);
				}
			}

			DataRow row;
			for (csv::CSVField field : csv_row) {
				const std::string value = field.get<std::string>();
				row.values.push_back(
						String::utf8(value.data(), static_cast<int64_t>(value.size())));
			}
			table.rows.push_back(row);
		}
	} catch (const std::exception &exception) {
		close();
		return DataResult::failure(
				ERR_PARSE_ERROR,
				"Unable to parse CSV: " + String::utf8(exception.what()));
	}

	if (table.schema.columns.is_empty()) {
		close();
		return DataResult::failure(ERR_FILE_CORRUPT, "CSV file does not contain any columns.");
	}

	table.schema.row_count = table.rows.size();
	opened = true;
	return DataResult::success();
}

void CsvDataSource::close() {
	opened = false;
	table = DataTable();
}

bool CsvDataSource::is_open() const {
	return opened;
}

CapabilityMask CsvDataSource::get_capabilities() const {
	return CAPABILITY_READ;
}

Vector<String> CsvDataSource::get_tables() const {
	Vector<String> tables;
	if (opened) {
		tables.push_back(table.schema.table_name);
	}
	return tables;
}

bool CsvDataSource::matches_table(const String &p_table) const {
	return p_table.is_empty() || p_table == table.schema.table_name;
}

DataResult CsvDataSource::get_schema(const String &p_table, DataSchema &r_schema) const {
	if (!opened) {
		return DataResult::failure(ERR_UNCONFIGURED, "No CSV file is open.");
	}
	if (!matches_table(p_table)) {
		return DataResult::failure(ERR_DOES_NOT_EXIST, "CSV table does not exist: " + p_table);
	}

	r_schema = table.schema;
	return DataResult::success();
}

DataResult CsvDataSource::fetch(const DataQuery &p_query, DataPage &r_page) const {
	if (!opened) {
		return DataResult::failure(ERR_UNCONFIGURED, "No CSV file is open.");
	}
	if (!matches_table(p_query.table_name)) {
		return DataResult::failure(ERR_DOES_NOT_EXIST, "CSV table does not exist: " + p_query.table_name);
	}
	if (p_query.offset < 0) {
		return DataResult::failure(ERR_INVALID_PARAMETER, "Fetch offset cannot be negative.");
	}
	if (p_query.limit < 0) {
		return DataResult::failure(ERR_INVALID_PARAMETER, "Fetch limit cannot be negative.");
	}

	r_page = DataPage();
	r_page.table_name = table.schema.table_name;
	r_page.offset = p_query.offset;
	r_page.total_count = table.rows.size();
	r_page.columns = table.schema.columns;

	if (p_query.offset >= table.rows.size() || p_query.limit == 0) {
		return DataResult::success();
	}

	const int64_t remaining = table.rows.size() - p_query.offset;
	const int64_t count = p_query.limit < remaining ? p_query.limit : remaining;
	for (int64_t index = 0; index < count; ++index) {
		r_page.rows.push_back(table.rows[p_query.offset + index]);
	}

	return DataResult::success();
}
