#ifndef DATAVIEW_DATA_SOURCE_FACTORY_H
#define DATAVIEW_DATA_SOURCE_FACTORY_H

#include "data_source.h"

#include <memory>

namespace godot::dataview {

class DataSourceFactory {
public:
	static std::unique_ptr<IDataSource> create(
			const String &p_path,
			const DataSourceOptions &p_options,
			DataResult &r_result);
};

} // namespace godot::dataview

#endif
