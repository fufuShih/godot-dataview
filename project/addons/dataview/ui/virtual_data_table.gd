@tool
extends Control

signal row_activated(row_index: int, values: Array)

@export_range(20.0, 80.0, 1.0) var row_height := 32.0
@export_range(20.0, 80.0, 1.0) var header_height := 36.0
@export_range(0, 10, 1) var overscan_rows := 2

var _data_view: Object
var _table_name := ""
var _column_names := PackedStringArray()
var _total_rows := 0
var _visible_rows := 1
var _current_offset := -1
var _row_pool: Array[PanelContainer] = []

@onready var _header_panel: PanelContainer = %HeaderPanel
@onready var _header: HBoxContainer = %Header
@onready var _body: Control = %Body
@onready var _rows_clip: Control = %RowsClip
@onready var _rows: Control = %Rows
@onready var _scroll_bar: VScrollBar = %VerticalScroll


func _ready() -> void:
	_scroll_bar.value_changed.connect(_on_scroll_value_changed)
	_rows_clip.resized.connect(_on_viewport_resized)
	call_deferred("_refresh_layout")


func set_data_view(data_view: Object, table_name := "") -> bool:
	clear()

	if data_view == null or not data_view.has_method("is_open") or not data_view.is_open():
		push_error("VirtualDataTable requires an open DataView.")
		return false

	var tables: PackedStringArray = data_view.get_tables()
	if table_name.is_empty():
		if tables.is_empty():
			push_error("DataView does not contain a table.")
			return false
		table_name = tables[0]

	var schema: Dictionary = data_view.get_schema(table_name)
	if schema.is_empty():
		push_error(data_view.get_last_error())
		return false

	_data_view = data_view
	_table_name = table_name
	_total_rows = int(schema.get("row_count", 0))

	for column: Dictionary in schema.get("columns", []):
		_column_names.push_back(str(column.get("name", "")))

	_build_header()
	_refresh_layout()
	return true


func clear() -> void:
	_data_view = null
	_table_name = ""
	_column_names.clear()
	_total_rows = 0
	_current_offset = -1

	if not is_node_ready():
		return

	_scroll_bar.value = 0
	for child: Node in _header.get_children():
		child.queue_free()
	for row: PanelContainer in _row_pool:
		row.queue_free()
	_row_pool.clear()


func scroll_to_row(row_index: int) -> void:
	if _total_rows == 0:
		return
	_scroll_bar.value = clampi(row_index, 0, maxi(0, _total_rows - _visible_rows))
	_render_visible_rows()


func get_total_row_count() -> int:
	return _total_rows


func get_created_row_control_count() -> int:
	return _row_pool.size()


func get_visible_row_range() -> Vector2i:
	var first := maxi(_current_offset, 0)
	return Vector2i(first, mini(first + _visible_rows, _total_rows))


func _refresh_layout() -> void:
	if not is_node_ready():
		return

	_header_panel.offset_bottom = header_height
	_body.offset_top = header_height
	_visible_rows = maxi(1, ceili(_rows_clip.size.y / row_height))

	_scroll_bar.min_value = 0
	_scroll_bar.max_value = maxi(_total_rows, _visible_rows)
	_scroll_bar.page = _visible_rows
	_scroll_bar.step = 1
	_scroll_bar.visible = _total_rows > _visible_rows
	_header_panel.offset_right = -16.0 if _scroll_bar.visible else 0.0
	_rows_clip.offset_right = -16.0 if _scroll_bar.visible else 0.0

	var pool_size := mini(_total_rows, _visible_rows + overscan_rows)
	_ensure_row_pool(pool_size)
	_layout_row_pool()
	_render_visible_rows(true)


func _build_header() -> void:
	for child: Node in _header.get_children():
		child.queue_free()

	for column_name: String in _column_names:
		var label := _create_label(column_name)
		label.add_theme_color_override("font_color", Color("e9edf5"))
		_header.add_child(label)


func _ensure_row_pool(required_count: int) -> void:
	while _row_pool.size() < required_count:
		var row := PanelContainer.new()
		row.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
		row.gui_input.connect(_on_row_gui_input.bind(row))

		var row_style := StyleBoxFlat.new()
		row_style.bg_color = Color("151922") if _row_pool.size() % 2 == 0 else Color("1b202b")
		row_style.border_width_bottom = 1
		row_style.border_color = Color("2c3442")
		row.add_theme_stylebox_override("panel", row_style)

		var cells := HBoxContainer.new()
		cells.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
		cells.mouse_filter = Control.MOUSE_FILTER_IGNORE
		cells.add_theme_constant_override("separation", 1)
		row.add_child(cells)

		for _column_name: String in _column_names:
			cells.add_child(_create_label(""))

		_rows.add_child(row)
		_row_pool.push_back(row)

	for index: int in range(_row_pool.size()):
		_row_pool[index].visible = index < required_count


func _layout_row_pool() -> void:
	for index: int in range(_row_pool.size()):
		var row := _row_pool[index]
		row.position = Vector2(0, index * row_height)
		row.size = Vector2(_rows_clip.size.x, row_height)


func _create_label(text_value: String) -> Label:
	var label := Label.new()
	label.text = text_value
	label.custom_minimum_size = Vector2(80, row_height)
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	label.size_flags_stretch_ratio = 1.0
	label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	label.text_overrun_behavior = TextServer.OVERRUN_TRIM_ELLIPSIS
	label.tooltip_text = text_value
	label.mouse_filter = Control.MOUSE_FILTER_IGNORE
	label.add_theme_constant_override("outline_size", 1)
	label.add_theme_color_override("font_outline_color", Color("10131a"))
	return label


func _render_visible_rows(force := false) -> void:
	if _data_view == null or _row_pool.is_empty():
		return

	var max_offset := maxi(0, _total_rows - _visible_rows)
	var offset := clampi(floori(_scroll_bar.value), 0, max_offset)
	if not force and offset == _current_offset:
		return
	_current_offset = offset

	var page: Dictionary = _data_view.fetch({
		"table": _table_name,
		"offset": offset,
		"limit": _row_pool.size(),
	})
	if page.is_empty():
		push_error(_data_view.get_last_error())
		return

	var page_rows: Array = page.get("rows", [])
	for pool_index: int in range(_row_pool.size()):
		var row_control := _row_pool[pool_index]
		if pool_index >= page_rows.size():
			row_control.visible = false
			continue

		row_control.visible = true
		row_control.set_meta("row_index", offset + pool_index)
		row_control.set_meta("values", page_rows[pool_index])

		var cells := row_control.get_child(0) as HBoxContainer
		var values: Array = page_rows[pool_index]
		for column_index: int in range(cells.get_child_count()):
			var label := cells.get_child(column_index) as Label
			label.text = str(values[column_index]) if column_index < values.size() else ""
			label.tooltip_text = label.text


func _on_scroll_value_changed(_value: float) -> void:
	_render_visible_rows()


func _on_viewport_resized() -> void:
	_refresh_layout()


func _on_row_gui_input(event: InputEvent, row: PanelContainer) -> void:
	if not event is InputEventMouseButton:
		return

	var mouse_event := event as InputEventMouseButton
	if mouse_event.pressed and mouse_event.button_index == MOUSE_BUTTON_WHEEL_UP:
		_scroll_bar.value -= maxi(1, _visible_rows / 3)
		row.accept_event()
	elif mouse_event.pressed and mouse_event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
		_scroll_bar.value += maxi(1, _visible_rows / 3)
		row.accept_event()
	elif mouse_event.double_click and mouse_event.button_index == MOUSE_BUTTON_LEFT:
		row_activated.emit(int(row.get_meta("row_index")), row.get_meta("values"))
