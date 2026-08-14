extends Control

const DEMO_ROW_COUNT := 10_000
const DEMO_PATH := "user://virtual_items.csv"

@onready var table: Control = %VirtualDataTable


func _ready() -> void:
	_test_csv_data_source()
	_create_large_demo_csv()

	var demo_data := DataView.new()
	assert(demo_data.open(DEMO_PATH), demo_data.get_last_error())
	assert(table.set_data_view(demo_data, "virtual_items"))

	await get_tree().process_frame
	assert(table.get_total_row_count() == DEMO_ROW_COUNT)
	assert(table.get_created_row_control_count() < 100)

	table.scroll_to_row(5_000)
	await get_tree().process_frame
	assert(table.get_visible_row_range().x == 5_000)
	table.scroll_to_row(0)

	print(
		"VirtualDataTable test passed: ",
		DEMO_ROW_COUNT,
		" data rows, ",
		table.get_created_row_control_count(),
		" row controls",
	)


func _test_csv_data_source() -> void:
	assert(ClassDB.class_exists("DataView"))

	var data := DataView.new()
	assert(data.open("res://data/items.csv"), data.get_last_error())
	assert(data.is_open())
	assert(data.has_capability(DataView.READ))
	assert(not data.has_capability(DataView.QUERY))

	var tables := data.get_tables()
	assert(tables == PackedStringArray(["items"]))

	var schema := data.get_schema("items")
	assert(schema.row_count == 4)
	assert(schema.columns.size() == 4)

	var page := data.fetch({
		"table": "items",
		"offset": 1,
		"limit": 2,
	})
	assert(page.total_count == 4)
	assert(page.count == 2)
	assert(page.rows[0][1] == "Healing Potion, Small")
	assert(page.rows[1][1] == "魔法書")


func _create_large_demo_csv() -> void:
	var file := FileAccess.open(DEMO_PATH, FileAccess.WRITE)
	assert(file != null, "Unable to create the virtual table demo CSV.")
	file.store_csv_line(PackedStringArray(["id", "name", "category", "price"]))

	var categories := PackedStringArray(["Weapon", "Armor", "Consumable", "Book"])
	for row_id: int in range(1, DEMO_ROW_COUNT + 1):
		file.store_csv_line(PackedStringArray([
			str(row_id),
			"Item %05d" % row_id,
			categories[row_id % categories.size()],
			str(row_id * 3),
		]))

	file.close()
