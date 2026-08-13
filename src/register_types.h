#ifndef DATAVIEW_REGISTER_TYPES_H
#define DATAVIEW_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_dataview_module(
    ModuleInitializationLevel p_level
);

void uninitialize_dataview_module(
    ModuleInitializationLevel p_level
);

#endif
