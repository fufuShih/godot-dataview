#include "dataview.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

DataView::DataView() {

}
DataView::~DataView() {

}

String DataView::hello() const {
	return "Hello from DataView C++!";
}

void DataView::_bind_methods() {
	ClassDB::bind_method(
		D_METHOD("hello"),
		&DataView::hello
	);
}
