# Curio
## Install
Copy the Curio.h and Curio.c files into your project
## Example
```
Window: {}
  Title: Main
  Height: 500
  Width: 500
Properties: []
- {}
    value: New
    onclick: CreateDoc()
- {}
    value: Open
    onclick: OpenDoc()
- {}
    value: Close
    onclick: CloseDoc()
Debug: True
Parent: Null
```
## API
### Types
```c
typedef enum Type {
    TYPE_OBJECT,
    TYPE_ARRAY,
    TYPE_STRING,
    TYPE_REAL,
    TYPE_INTEGER,
    TYPE_BOOLEAN,
    TYPE_NULL
} Type;
```
### Serialization
| Method | Parameters | Return | Description |
| :--- | :--- | :--- | :--- |
| Serialzie | `char*` filename `Value*` value | `void` | Serialized `Value*` to filename with Curio format |
### Parsing
| Method | Parameters | Return | Description |
| :--- | :--- | :--- | :--- |
| Parse | `char*` filename | `Value*` | Parse filename with Curio format |
### Creating
| Method | Parameters | Return | Description |
| :--- | :--- | :--- | :--- |
| CurioObject | | `Value*` | Create a Curio Value with `TYPE_OBJECT` |
| CurioArray | | `Value*` | Create a Curio Value with `TYPE_ARRAY` |
| CurioString | `char*` string | `Value*` | Create a Curio Value with `TYPE_STRING` |
| CurioReal | `double` real | `Value*` | Create a Curio Value with `TYPE_REAL` |
| CurioInteger | `long` integer | `Value*` | Create a Curio Value with `TYPE_INTEGER` |
| CurioBoolean | `char` boolean | `Value*` | Create a Curio Value with `TYPE_BOOLEAN` |
| CurioNull | | `Value*` | Create a Curio Value with `TYPE_NULL` |
### Adding
| Method | Parameters | Return | Description |
| :--- | :--- | :--- | :--- |
| AppendMember | `Value*` parent `char*` name `Value*` data | `Value*` | Append a member to an `Object` |
| AppendElement | `Value*` parent `Value*` data | `Value*` | Append an element to an `Array` |
### Removing
| Method | Parameters | Return | Description |
| :--- | :--- | :--- | :--- |
| RemoveMember | `Value*` parent `char*` name | `void` | Remove a member by name from an `Object` |
| RemoveElement | `Value*` parent `int` index | `void` | Remove a element by index from a `Array` |
### Properties
| Method | Parameters | Return | Description |
| :--- | :--- | :--- | :--- |
| ChildCount | `Value*` value | `int` | Get the number of children from `TYPE_OBJECT` or `TYPE_ARRAY` |
| ChildNames | `Value*` value | `char**` | Get the member names from a `TYPE_OBJECT` |
| ChildValues | `Value*` value | `Value**` | Get the child values from `TYPE_OBJECT` or `TYPE_ARRAY` |
### Access
| Method | Parameters | Return | Description |
| :--- | :--- | :--- | :--- |
| GetArrayValue | `Value*` value `int` index | `Value*` | Get value at index from  `TYPE_ARRAY` |
| GetObjectValue | `Value*` value `char*` name | `Value*` | Get value with name from `TYPE_OBJECT` |
### Destroying
| Method | Parameters | Return | Description |
| :--- | :--- | :--- | :--- |
| CurioFree | `Value*` value | `void` | Free memory for `Value*` and all children |
