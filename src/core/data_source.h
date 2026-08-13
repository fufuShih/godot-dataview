#ifndef DATAVIEW_DATA_SOURCE_H
#define DATAVIEW_DATA_SOURCE_H

#include "data_types.h"

namespace godot::dataview {

class IDataSource {
public:
	virtual ~IDataSource() = default;

	virtual DataResult open(const String &p_path) = 0;
	virtual void close() = 0;
	virtual bool is_open() const = 0;

	virtual CapabilityMask get_capabilities() const = 0;
	virtual Vector<String> get_tables() const = 0;
	virtual DataResult get_schema(const String &p_table, DataSchema &r_schema) const = 0;
	virtual DataResult fetch(const DataQuery &p_query, DataPage &r_page) const = 0;

	virtual DataResult save() {
		return DataResult::failure(ERR_UNAVAILABLE, "This data source is read-only.");
	}
};

} // namespace godot::dataview

#endif
