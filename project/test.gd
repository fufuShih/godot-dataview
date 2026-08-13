extends Node

func _ready() -> void:
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

	print("DataView CSV test passed: ", page)
