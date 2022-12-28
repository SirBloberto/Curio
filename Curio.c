#include "Curio.h"
#include "stdio.h"
#include "stdlib.h"
#include "regex.h"

typedef struct ValueIterator {
    Value value;
    unsigned int iterated;
} ValueIterator;

char* Read(char** string, char stop, int continue_flag);
int Match(char** string, char* expected);
int Validate(char* string, char* regex_string);

Value* CurioObject() {
    Value* value = (Value*)malloc(sizeof(ValueIterator));
    value->parent = 0;
    value->type = TYPE_OBJECT;
    value->data.object = (Object*)malloc(sizeof(Object));
    value->data.object->count = 0;
    value->data.object->size = 8;
    value->data.object->names = (char**)calloc(value->data.object->size, sizeof(char*));
    value->data.object->members = (Value**)calloc(value->data.object->size, sizeof(Value*));
    return value;
}

Value* CurioArray() {
    Value* value = (Value*)malloc(sizeof(ValueIterator));
    value->parent = 0;
    value->type = TYPE_ARRAY;
    value->data.array = (Array*)malloc(sizeof(Array));
    value->data.array->count = 0;
    value->data.array->size = 8;
    value->data.array->elements = (Value**)calloc(value->data.array->size, sizeof(Value*));
    return value;
}

Value* CurioString(char* data) {
    Value* value = (Value*)malloc(sizeof(Value));
    value->parent = 0;
    value->type = TYPE_STRING;
    value->data.string = data;
    return value;
}

Value* CurioReal(double data) {
    Value* value = (Value*)malloc(sizeof(Value));
    value->parent = 0;
    value->type = TYPE_REAL;
    value->data.real = data;
    return value;
}

Value* CurioInteger(long data) {
    Value* value = (Value*)malloc(sizeof(Value));
    value->parent = 0;
    value->type = TYPE_INTEGER;
    value->data.integer = data;
    return value;
}

Value* CurioBoolean(char data) {
    Value* value = (Value*)malloc(sizeof(Value));
    value->parent = 0;
    value->type = TYPE_BOOLEAN;
    value->data.boolean = data;
    return value;
}

Value* CurioNull() {
    Value* value = (Value*)malloc(sizeof(Value));
    value->parent = 0;
    value->type = TYPE_NULL;
    return value;
}

Value* AppendMember(Value* parent, char* name, Value* data) {
    data->parent = parent;
    if(parent->data.object->count == parent->data.object->size) {
        parent->data.object->size *= 2;
        char** names = (char**)calloc(parent->data.object->size, sizeof(char*));
        Value** members = (Value**)calloc(parent->data.object->size, sizeof(Value*));
        for(int i = 0; i < parent->data.object->count; i++) {
            names[i] = parent->data.object->names[i];
            members[i] = parent->data.object->members[i];
        }
        free(parent->data.object->names);
        for(int i = 0; i < parent->data.object->count; i++)
            CurioFree(parent->data.object->members[i]);
        free(parent->data.object->members);
        parent->data.object->names = names;
        parent->data.object->members = members;
    }
    parent->data.object->names[parent->data.object->count] = name;
    parent->data.object->members[parent->data.object->count++] = data;
    return parent;
}

Value* AppendElement(Value* parent, Value* data) {
    data->parent = parent;
    if(parent->data.array->count >= parent->data.array->size) {
        parent->data.array->size *= 2;
        Value** elements = (Value**)calloc(parent->data.array->size, sizeof(Value*));
        for(int i = 0; i < parent->data.array->count; i++)
            elements[i] = parent->data.array->elements[i];
        for(int i = 0; i < parent->data.array->count; i++)
            CurioFree(parent->data.array->elements[i]);
        free(parent->data.array->elements);
        parent->data.array->elements = elements;
    }
    parent->data.array->elements[parent->data.array->count++] = data;
    return parent;
}

void RemoveMember(Value* parent, char* name) {
    if(parent->type != TYPE_OBJECT)
        return (void)printf("[Curio-RemoveMember]: Parent must be TYPE_OBJECT");
    int index = 0;
    for(; index < parent->data.object->count; index++) {
        if(parent->data.object->names[index] == name)
            break;
    }
    if(index + 1 >= parent->data.object->count)
        return (void)printf("[Curio-RemoveMember]: Could not find value with name: %s", name);
    for(; index < parent->data.object->count - 1; index++) {
        parent->data.object->names[index] = parent->data.object->names[index + 1];
        parent->data.object->members[index] = parent->data.object->members[index + 1];
    }
}

void RemoveElement(Value* parent, unsigned int index) {
    if(parent->type != TYPE_ARRAY)
        return (void)printf("[Curio-RemoveElement]: Parent must be TYPE_ARRAY");
    if(index > parent->data.array->count)
        return (void)printf("[Curio-RemoveElement]: Cannot remove value, index is greater than element count");
    for(int i = index; i < parent->data.array->count - 1; i++)
        parent->data.array->elements[i] = parent->data.array->elements[i + 1];
}

unsigned int ChildCount(Value* value) {
    switch(value->type) {
        case TYPE_OBJECT: return value->data.object->count;
        case TYPE_ARRAY: return value->data.array->count;
        default: return 0 & printf("[Curio-ChildCount]: Value cannot have children");
    }
}

char** ChildNames(Value* value) {
    if(value->type == TYPE_OBJECT)
        return value->data.object->names;
    return (char**)((long)0 & printf("[Curio-ChildNames]: Value must be TYPE_OBJECT"));
}

Value** ChildValues(Value* value) {
    switch(value->type) {
        case TYPE_OBJECT: return value->data.object->members;
        case TYPE_ARRAY: return value->data.array->elements;
        default: return (Value**)((long)0 & printf("[Curio-ChildValues]: Value cannot have children"));
    }
}

Value* GetArrayValue(Value* value, unsigned int index) {
    if(value->type != TYPE_ARRAY)
        return (Value*)((long)0 & printf("[Curio-GetArrayValue]: Value must be TYPE_ARRAY"));
    if(index >= value->data.array->count)
        return (Value*)((long)0 & printf("[Curio-GetArrayValue]: Could not find element at index: %d", index));
    return value->data.array->elements[index];
}

Value* GetObjectValue(Value* value, char* name) {
    if(value->type != TYPE_OBJECT)
        return (Value*)((long)0 & printf("[Curio-GetObjectValue]: Value must be TYPE_OBJECT"));
    for(int i = 0; i < value->data.object->count; i++) {
        if(value->data.object->names[i] == name)
            return value->data.object->members[i];
    }
    return (Value*)((long)0 & printf("[Curio-GetObjectValue]: Could not find member with name: %s", name));
}

void Serialize(char* filename, Value* value) {
    FILE* file = fopen(filename, "w");
    if(!file)
        return (void)printf("[Curio-Serialize]: Could not open file: %s", filename);
    int depth = -1;
    while(value) {
        switch(value->type) {
            case TYPE_OBJECT:
                if(((ValueIterator*)value)->iterated == 0) {
                    if(depth != -1)
                        fprintf(file, "{}");
                    depth++;
                }
                if(((ValueIterator*)value)->iterated == value->data.object->count) {
                    depth--;
                    ((ValueIterator*)value)->iterated = 0;
                    break;
                }
                if(depth == 0) {
                    if(((ValueIterator*)value)->iterated > 0)
                        fprintf(file, "\n");
                } else
                    fprintf(file, "\n");
                for(int i = 0; i < depth; i++)
                    fprintf(file, "  ");
                fprintf(file, "%s", value->data.object->names[((ValueIterator*)value)->iterated]);
                fprintf(file, ": ");
                value = value->data.object->members[((ValueIterator*)value)->iterated++];
                continue;
            case TYPE_ARRAY:
                if(((ValueIterator*)value)->iterated == 0) {
                    if(depth != -1)
                        fprintf(file, "[]");
                    depth++;
                }
                if(((ValueIterator*)value)->iterated == value->data.array->count) {
                    depth--;
                    ((ValueIterator*)value)->iterated = 0;
                    break;
                }
                fprintf(file, "\n");
                for(int i = 0; i < depth - 1; i++)
                    fprintf(file, "  ");
                if(depth > 0)
                    fprintf(file, "- ");
                value = value->data.array->elements[((ValueIterator*)value)->iterated++];
                continue;
            case TYPE_STRING:
                fprintf(file, "%s", value->data.string);
                break;
            case TYPE_REAL:
                int length = snprintf(0, 0, "%f", value->data.real);
                char* buffer = malloc(sizeof(char) * length);
                snprintf(buffer, length, "%f", value->data.real);
                while((buffer[length] == '\0' || buffer[length] == '0') && buffer[length - 1] != '.')
                    buffer[length--] = '\0';
                fprintf(file, "%s", buffer);
                free(buffer);
                break;
            case TYPE_INTEGER:
                fprintf(file, "%ld", value->data.integer);
                break;
            case TYPE_BOOLEAN:
                if(value->data.boolean)
                    fprintf(file, "True");
                else
                    fprintf(file, "False");
                break;
            case TYPE_NULL:
                fprintf(file, "Null");
                break;
            default:
                return (void)printf("[Curio-Serialize]: Data type does not match any Curio pattern");
                break;
        }
        value = value->parent;
    }
    fclose(file);
}

Value* Parse(char* filename) {
    FILE* file = fopen(filename, "r");
    if(!file)
        return (Value*)((long)0 & printf("[Curio-Parse]: Could not open file: %s", filename));
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = (char*)malloc(sizeof(char) * length);
    fread(buffer, 1, length, file);
    fclose(file);
    unsigned int line = 1;
    int depth = 0;
    Value* parent = CurioObject();
    while(*buffer != '\0') {
        char* name;
        Value* value;
        for(int i = 0; i < depth - 1; i++) {
            if(!Match(&buffer, "  ")) {
                parent = parent->parent;
                depth--;
            }
        }
        if(depth > 0 && Match(&buffer, "- ")) {
            if(parent->type != TYPE_ARRAY)
                return (Value*)((long)0 & printf("[Curio-Parse]: %s(%d). Indentation error. Parent is not an Array", filename, line));
        } else {
            if(depth > 0 && !Match(&buffer, "  ")) {
                parent = parent->parent;
                depth--;
            }
            if(parent->type != TYPE_OBJECT)
                return (Value*)((long)0 & printf("[Curio-Parse]: %s(%d). Indentation error. Parent is not an Object", filename, line));
            if(Validate(Read(&buffer, '\n', 0), "(: ){1,}"))
                return (Value*)((long)0 & printf("[Curio-Parse]: %s(%d). Object member is missing seperator ': '", filename, line));
            name = Read(&buffer, ':', 1);
            if(Validate(name, "^[_a-zA-Z][_a-zA-Z0-9]*$"))
                return (Value*)((long)0 & printf("[Curio-Parse]: %s(%d). Invalid member name", filename, line));
            buffer += 2;
        }
        char character = *buffer++;
        if(character == '{') {
            value = CurioObject();
            depth++;
            if(!Match(&buffer, "}"))
                return (Value*)((long)0 & printf("[Curio-Parse]: %s(%d). Object needs a closing brace '}'", filename, line));
        } else if(character == '[') {
            value = CurioArray();
            depth++;
            if(!Match(&buffer, "]"))
                return (Value*)((long)0 & printf("[Curio-Parse]: %s(%d). Array needs a closing brace ']'", filename, line));
        } else if(character >= 'A' && character <= 'z') {
            buffer--;
            char* string = Read(&buffer, '\n', 1);
            if(Match(&string, "True"))
                value = CurioBoolean(1);
            else if(Match(&string, "False"))
                value = CurioBoolean(0);
            else if(Match(&string, "Null"))
                value = CurioNull();
            else
                value = CurioString(string);
        } else if((character >= '0' && character <= '9') || character == '-') {
            char firstCharacter = character;
            buffer--;
            if(Validate(Read(&buffer, '\n', 0), "^0$|^0.0$|^-?0.[0-9]*[1-9]+$|^-?[1-9][0-9]*(.[0-9]*[1-9]+)?$"))
                return (Value*)((long)0 & printf("[Curio-Parse]: %s(%d). Value does not match any number pattern", filename, line));
            if(firstCharacter == '-')
                buffer++;
            long integer = 0;
            for(character = *buffer++; character >= '0' && character <= '9'; character = *buffer++)
                integer = integer * 10 + character - '0';
            if(character == '.') {
                double real = (double)integer;
                character = *buffer++;
                for(int i = 10; character >= '0' && character <= '9'; i *= 10, character = *buffer++)
                    real += (double)(character - '0') / i;
                if(firstCharacter == '-')
                    real = -real;
                value = CurioReal(real);
            } else {
                if(firstCharacter == '-')
                    integer = -integer;
                value = CurioInteger(integer);
            }
            buffer--;
        } else
            return (Value*)((long)0 & printf("[Curio-Parse]: %s(%d). Value does not match any Curio pattern", filename, line));
        if(!Match(&buffer, "\n"))
            return (Value*)((long)0 & printf("[Curio-Parse]: %s(%d). Missing New Line character", filename, line));
        line++;
        if(parent->type == TYPE_OBJECT)
            AppendMember(parent, name, value);
        if(parent->type == TYPE_ARRAY)
            AppendElement(parent, value);
        if(value->type == TYPE_OBJECT || value->type == TYPE_ARRAY)
            parent = value;
    }
    while(parent->parent != 0)
        parent = parent->parent;
    return parent;
}

void CurioFree(Value* value) {
    switch(value->type) {
        case TYPE_OBJECT:
            free(value->data.object->names);
            for(int i = 0; i < value->data.object->count; i++)
                CurioFree(value->data.object->members[i]);
            free(value->data.object->members);
            free(value->data.object);
            break;
        case TYPE_ARRAY:
            for(int i = 0; i < value->data.array->size; i++)
                CurioFree(value->data.array->elements[i]);
            free(value->data.array);
            break;
        default:
            break;
    }
    free(value);
}

char* Read(char** string, char stop, int continue_flag) {
    int length = 0;
    for(; **string != stop && **string != '\0'; length++, (*string)++);
    *string -= length;
    char* name = (char*)calloc(length, sizeof(char));
    for(int i = 0; i < length; i++)
        name[i] = *(*string)++;
    if(!continue_flag)
        *string -= length;
    return name;
}

int Match(char** string, char* expected) {
    for(int i = 0; expected[i] != '\0' && (*string)[i] != '\0'; i++, (*string)++) {
        if(expected[i] != **string) {
            *string -= i;
            return 0;
        }
    }
    return 1;
}

int Validate(char* string, char* regex_string) {
    regex_t regex;
    regcomp(&regex, regex_string, REG_EXTENDED);
    return regexec(&regex, string, 0, NULL, 0);
}