#ifndef STORAGE_H
#define STORAGE_H

enum Type;
struct Value;
struct Object;
struct Array;

typedef enum Type {
    TYPE_OBJECT,
    TYPE_ARRAY,
    TYPE_STRING,
    TYPE_REAL,
    TYPE_INTEGER,
    TYPE_BOOLEAN,
    TYPE_NULL
} Type;

typedef struct Value {
    struct Value* parent;
    enum Type type;
    union {
        struct Object* object;
        struct Array* array;
        char* string;
        double real;
        long integer;
        char boolean;
    } data;
} Value;

typedef struct Object {
    char** names;
    struct Value** members;
    unsigned int count;
    unsigned int size;
} Object;

typedef struct Array {
    struct Value** elements;
    unsigned int count;
    unsigned int size;
} Array;

Value* CurioObject();
Value* CurioArray();
Value* CurioString(char* data);
Value* CurioReal(double data);
Value* CurioInteger(long data);
Value* CurioBoolean(char data);
Value* CurioNull();

Value* AppendMember(Value* parent, char* name, Value* data);
Value* AppendElement(Value* parent, Value* data);

void RemoveMember(Value* parent, char* name);
void RemoveElement(Value* parent, unsigned int index);

unsigned int ChildCount(Value* value);
char** ChildNames(Value* value);
Value** ChildValues(Value* value);
Value* GetArrayValue(Value* value, unsigned int index);
Value* GetObjectValue(Value* value, char* name);

void Serialize(char* filename, Value* value);
Value* Parse(char* filename);

void CurioFree(Value* value);

#endif