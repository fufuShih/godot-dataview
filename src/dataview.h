#ifndef DATAVIEW_H
#define DATAVIEW_H


#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class DataView: public RefCounted {

	GDCLASS(DataView, RefCounted);

	protected:
		static void _bind_methods();

	public:
		DataView();
		~DataView();

		String hello() const;

};

}

#endif
