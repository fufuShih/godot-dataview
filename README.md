# Godot DataView

DataView is a Godot editor tool for inspecting structured data. The current
version reads CSV files through a native GDExtension and displays them in a
virtualized table in the editor's bottom panel.

## Current features

- CSV files from `res://`, `user://`, or an absolute filesystem path.
- Configurable single-character delimiter and optional header row.
- RFC-style quoted fields, UTF-8 text, and multiline-safe parsing.
- Fixed-row-height virtual table that reuses visible row controls.
- A format-independent native API designed for future XLSX and SQLite sources.

## Build and open the editor tool

Clone dependencies and build the native extension:

```bash
git submodule update --init --recursive
scons
```

Open `project/` in Godot 4.7. The development project enables DataView by
default. In another project, enable it under **Project > Project Settings >
Plugins**.

Open the **DataView** bottom panel, choose a CSV file, then use **Reload** after
the file changes. Both `res://` paths and absolute paths are accepted.

The installable addon layout is:

```text
addons/dataview/
├─ plugin.cfg
├─ plugin.gd
├─ dataview.gdextension
├─ bin/
└─ ui/
```

## Architecture

```text
Godot editor bottom panel
          │
          ▼
      DataView facade
          │
          ▼
  DataSourceFactory
          │
          ▼
    CsvDataSource
          │
          ▼
csv-parser 5.3.0
```

Third-party parser types stay inside `CsvDataSource`. The public data model is
defined by `DataSchema`, `DataQuery`, `DataPage`, and `DataResult`, so future
XLSX and SQLite implementations can use the same editor UI.

## Native API

The editor panel uses this small API internally:

```gdscript
var data := DataView.new()

if not data.open("res://data/items.csv", {
    "delimiter": ",",
    "has_header": true,
}):
    push_error(data.get_last_error())
    return

var schema := data.get_schema("items")
var visible_page := data.fetch({
    "table": "items",
    "offset": 100,
    "limit": 30,
})
```

`fetch()` returns `table`, `offset`, `count`, `total_count`, `columns`, and
`rows`. CSV currently exposes the `DataView.READ` capability. Writing,
filtering, and sorting are not implemented yet.

## Verification

Run the native/API and virtualization smoke test:

```powershell
Godot_v4.7-stable_win64.exe --headless --path project --quit-after 5 res://test.tscn
```

Run the editor plugin smoke test:

```powershell
Godot_v4.7-stable_win64.exe --headless --editor --path project `
  -- --dataview-smoke-test
```
