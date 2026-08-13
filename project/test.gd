extends Node

func _ready() -> void:
	print(ClassDB.class_exists("DataView"))

	var dataview = DataView.new()

	print(dataview.hello())
