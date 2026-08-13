#ifndef DATAVIEW_H
#define DATAVIEW_H

#include "data_source.h"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <memory>

namespace godot {

class DataView : public RefCounted {
	GDCLASS(DataView, RefCounted);

public:
	enum Capability {
		READ = dataview::CAPABILITY_READ,
		WRITE = dataview::CAPABILITY_WRITE,
		MULTI_TABLE = dataview::CAPABILITY_MULTI_TABLE,
		QUERY = dataview::CAPABILITY_QUERY,
		TRANSACTION = dataview::CAPABILITY_TRANSACTION,
		SCHEMA_EDIT = dataview::CAPABILITY_SCHEMA_EDIT,
	};

private:
	std::unique_ptr<dataview::IDataSource> source;
	String last_error;

	static String data_type_name(dataview::DataType p_type);
	void set_result(const dataview::DataResult &p_result);

protected:
	static void _bind_methods();

public:
	DataView() = default;
	~DataView() override = default;

	bool open(const String &p_path, const Dictionary &p_options = Dictionary());
	void close();
	bool is_open() const;

	int64_t get_capabilities() const;
	bool has_capability(int64_t p_capability) const;

	PackedStringArray get_tables() const;
	Dictionary get_schema(const String &p_table = "");
	Dictionary fetch(const Dictionary &p_query = Dictionary());

	bool save();
	String get_last_error() const;
};

} // namespace godot

#endif
