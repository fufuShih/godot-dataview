@tool
extends EditorPlugin

const DATA_VIEW_DOCK := preload("res://addons/dataview/ui/data_view_dock.tscn")

var _dock: Control
var _bottom_panel_button: Button


func _enter_tree() -> void:
	_dock = DATA_VIEW_DOCK.instantiate()
	_bottom_panel_button = add_control_to_bottom_panel(_dock, "DataView")
	_bottom_panel_button.tooltip_text = "Inspect CSV data"

	if "--dataview-smoke-test" in OS.get_cmdline_user_args():
		_run_smoke_test.call_deferred()


func _exit_tree() -> void:
	if is_instance_valid(_dock):
		remove_control_from_bottom_panel(_dock)
		_dock.queue_free()
	_dock = null
	_bottom_panel_button = null


func _run_smoke_test() -> void:
	if not _dock.open_file("res://data/items.csv"):
		push_error("DataView editor panel could not open the CSV smoke-test file.")
		get_tree().quit(1)
		return

	var table := _dock.find_child("VirtualDataTable", true, false)
	if table == null or table.get_total_row_count() != 4:
		push_error("DataView editor table did not receive the expected rows.")
		get_tree().quit(1)
		return

	print("DataView editor plugin test passed: bottom panel loaded 4 CSV rows")
	get_tree().quit()
