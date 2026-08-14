@tool
extends VBoxContainer

var _data_view: Object

@onready var _path: LineEdit = %Path
@onready var _browse: Button = %Browse
@onready var _reload: Button = %Reload
@onready var _delimiter: LineEdit = %Delimiter
@onready var _has_header: CheckBox = %HasHeader
@onready var _status: Label = %Status
@onready var _table: Control = %VirtualDataTable
@onready var _file_dialog: FileDialog = %FileDialog


func _ready() -> void:
	_browse.pressed.connect(_on_browse_pressed)
	_reload.pressed.connect(_open_current_path)
	_path.text_submitted.connect(func(_value: String) -> void: _open_current_path())
	_file_dialog.file_selected.connect(_on_file_selected)
	_table.row_activated.connect(_on_row_activated)
	_set_status("Choose a CSV file to inspect.")


func open_file(path: String) -> bool:
	path = path.strip_edges()
	if path.is_empty():
		_set_status("Choose a CSV file first.", true)
		return false

	if not ClassDB.class_exists("DataView"):
		_set_status("The DataView native extension is not loaded.", true)
		return false

	var delimiter := _delimiter.text
	if delimiter.is_empty():
		delimiter = ","

	var candidate: Object = ClassDB.instantiate("DataView")
	if candidate == null:
		_set_status("Unable to instantiate the DataView native class.", true)
		return false

	if not candidate.open(path, {
		"delimiter": delimiter,
		"has_header": _has_header.button_pressed,
	}):
		_table.clear()
		_data_view = null
		_set_status(candidate.get_last_error(), true)
		return false

	var tables: PackedStringArray = candidate.get_tables()
	if tables.is_empty():
		_table.clear()
		_data_view = null
		_set_status("The data source does not contain a table.", true)
		return false

	var table_name := tables[0]
	var schema: Dictionary = candidate.get_schema(table_name)
	if schema.is_empty() or not _table.set_data_view(candidate, table_name):
		_table.clear()
		_data_view = null
		_set_status(candidate.get_last_error(), true)
		return false

	_data_view = candidate
	_path.text = path
	_set_status(
		"%s  •  %d rows  •  %d columns" % [
			path,
			int(schema.get("row_count", 0)),
			Array(schema.get("columns", [])).size(),
		]
	)
	return true


func _open_current_path() -> void:
	open_file(_path.text)


func _on_browse_pressed() -> void:
	if _path.text.is_empty():
		_file_dialog.current_dir = ProjectSettings.globalize_path("res://")
	_file_dialog.popup_centered_ratio(0.75)


func _on_file_selected(path: String) -> void:
	var project_root := ProjectSettings.globalize_path("res://")
	if path.replace("\\", "/").begins_with(project_root.replace("\\", "/")):
		path = ProjectSettings.localize_path(path)
	_path.text = path
	open_file(path)


func _on_row_activated(row_index: int, values: Array) -> void:
	_set_status("Row %d  •  %s" % [row_index + 1, str(values)])


func _set_status(message: String, is_error := false) -> void:
	_status.text = message
	_status.tooltip_text = message
	_status.add_theme_color_override(
			"font_color",
			Color("ff7b72") if is_error else Color("aeb8c8"))
