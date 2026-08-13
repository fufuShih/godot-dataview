#ifndef DATAVIEW_CSV_SOURCE_H
#define DATAVIEW_CSV_SOURCE_H

#include "../core/data_source.h"

namespace godot::dataview {

class CsvDataSource final : public IDataSource {
private:
	String delimiter;
	bool has_header = true;
	bool opened = false;
	DataTable table;

	bool matches_table(const String &p_table) const;

public:
	CsvDataSource(const String &p_delimiter = ",", bool p_has_header = true);
	~CsvDataSource() override = default;

	DataResult open(const String &p_path) override;
	void close() override;
	bool is_open() const override;

	CapabilityMask get_capabilities() const override;
	Vector<String> get_tables() const override;
	DataResult get_schema(const String &p_table, DataSchema &r_schema) const override;
	DataResult fetch(const DataQuery &p_query, DataPage &r_page) const override;
};

} // namespace godot::dataview

#endif
