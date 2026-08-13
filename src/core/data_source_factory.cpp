#include "data_source_factory.h"

#include "../sources/csv_source.h"

using namespace godot;
using namespace godot::dataview;

std::unique_ptr<IDataSource> DataSourceFactory::create(
		const String &p_path,
		const DataSourceOptions &p_options,
		DataResult &r_result) {
	const String extension = p_path.get_extension().to_lower();

	if (extension == "csv") {
		r_result = DataResult::success();
		return std::make_unique<CsvDataSource>(
				p_options.csv_delimiter,
				p_options.csv_has_header);
	}

	r_result = DataResult::failure(
			ERR_FILE_UNRECOGNIZED,
			"Unsupported data source extension: " + extension);
	return nullptr;
}
