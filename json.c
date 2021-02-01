// TODO(Felix): Maybe replace u8*  with void* after all, as we don't really
// seem to be doing (Memory++) that often

typedef enum 
{ 
	SIMPLIFIED_JSON_OBJECT_DATA_TYPE_NULL,
	SIMPLIFIED_JSON_OBJECT_DATA_TYPE_STRING,
	SIMPLIFIED_JSON_OBJECT_DATA_TYPE_OBJECT_OR_ARRAY,
	SIMPLIFIED_JSON_OBJECT_DATA_TYPE_BOOLEAN32,
} simplified_json_object_data_type;

typedef struct simplified_json_object simplified_json_object;
struct simplified_json_object
{
	char *Name;
	void *Data;
	simplified_json_object_data_type DataType;
	simplified_json_object *Next;
};

internal simplified_json_object * JsonParseStringIntoTraversableObject(char *JsonToParse, void *MemoryToUse);
internal void * JsonFindDataByName(simplified_json_object *Object, char *Name);

internal void *
JsonFindDataByNameDepthFirst(simplified_json_object *Object, char *Name)
{
	assert(Object);
	simplified_json_object *CurrentObject = Object;
	void *Result = 0;
	while (CurrentObject && 0 == Result)
	{
		if (CurrentObject->Name && 0 == strcmp(CurrentObject->Name, Name))
		{
			Result = CurrentObject->Data;
		}
		else if (CurrentObject->DataType == SIMPLIFIED_JSON_OBJECT_DATA_TYPE_OBJECT_OR_ARRAY)
		{
			Result = JsonFindDataByNameDepthFirst(CurrentObject->Data, Name);
		}
		
		CurrentObject = CurrentObject->Next;
	}
	return (Result);
}

internal void *
JsonFindDataByNameBreadthFirst(simplified_json_object *Object, char *Name)
{
	assert(Object);
	simplified_json_object *(QueueBuffer[1024]) = { 0 };
	umm FilledIndex = 0;
	umm FreeIndex = 0;
	for (simplified_json_object *CurrentObject = Object;
		 0 != CurrentObject;
		 CurrentObject = CurrentObject->Next)
	{
		QueueBuffer[FreeIndex++] = CurrentObject;
	}

	void *Result = 0;
	while (FreeIndex != FilledIndex && // Queue not empty
		   0 == Result)                // Result not yet found
	{
		simplified_json_object *CurrentObject = QueueBuffer[FilledIndex++];
		if (CurrentObject->Name &&
			0 == strcmp(CurrentObject->Name, Name))
		{
			Result = CurrentObject->Data;
		}
		else if (CurrentObject->DataType == SIMPLIFIED_JSON_OBJECT_DATA_TYPE_OBJECT_OR_ARRAY)
		{
			for (simplified_json_object *ObjectToAdd = CurrentObject->Data;
				 0 != ObjectToAdd;
				 ObjectToAdd = ObjectToAdd->Next)
			{
				QueueBuffer[FreeIndex++] = ObjectToAdd;
				FreeIndex %= ARRAYCOUNT(QueueBuffer);
			}
		}
	}
	return (Result);
}

internal void
JsonSimplifiedObjectPrint(simplified_json_object *Object)
{
	assert(Object);
	if (Object->Name)
	{
		printf("%s: ", Object->Name);
	}

	switch (Object->DataType)
	{
		case SIMPLIFIED_JSON_OBJECT_DATA_TYPE_NULL: {
			printf("null");
		} break;

		case SIMPLIFIED_JSON_OBJECT_DATA_TYPE_STRING: {
			printf("%s", (char *)Object->Data);
		} break;

		case SIMPLIFIED_JSON_OBJECT_DATA_TYPE_OBJECT_OR_ARRAY: {
			printf("{ ");
			JsonSimplifiedObjectPrint(Object->Data);
			printf(" }");
		} break;

		case SIMPLIFIED_JSON_OBJECT_DATA_TYPE_BOOLEAN32: {
			b32 Value = *(b32 *)Object->Data;
			if (Value)
			{
				printf("true");
			}
			else
			{
				printf("false");
			}
		} break;
		
		default: { assert(!"Unknown json object type"); } break;
	}
	
	if (Object->Next)
	{
		printf(", ");
		JsonSimplifiedObjectPrint(Object->Next);
	}
}

internal char *
JsonParseString(char **JsonToParse, u8 **MemoryToUse)
{
	//fprintf(stderr, "PARSING STRING: %.*s\n", 64, *JsonToParse);
	assert(**JsonToParse == '"');
	*JsonToParse += 1;

	char *Result = (char *)*MemoryToUse;
	u32 StringLength = 0;
	
	while (**JsonToParse != '"')
	{
		if (**JsonToParse == '\\')
		{
			*JsonToParse += 1;
			switch (**JsonToParse)
			{
				case '"': {
					// TODO(Felix): 
				} break;
				
				case '\\': {
					// TODO(Felix): 
				} break;
				
				case 'n': {
					// TODO(Felix): 
				} break;
				
				case 't': {
					// TODO(Felix): 
				} break;
				
				case 'u': {
					// TODO(Felix): 
				} break;
				
				default: { 
					fprintf(stderr, "%c: ", **JsonToParse);
					assert(!"Unexpected escaped char");
				} break;
			}
		}
		else
		{
			Result[StringLength++] = **JsonToParse;
			*JsonToParse += 1;
		}
	}
	
	*JsonToParse += 1;
	Result[StringLength++] = 0; // 0 Terminate string
	*MemoryToUse = (void *)(Result+StringLength);
	return (Result);
}

internal simplified_json_object * JsonParseObject(char **JsonToParse, u8 **MemoryToUse);
internal simplified_json_object * JsonParseArray(char **JsonToParse, u8 **MemoryToUse);

internal void *
JsonParseValue(char **JsonToParse, u8 **MemoryToUse, simplified_json_object_data_type *ResultType)
{
	//fprintf(stderr, "PARSING VALUE: %.*s\n", 64, *JsonToParse);
	void *ReturnValue = 0;
	*ResultType = SIMPLIFIED_JSON_OBJECT_DATA_TYPE_NULL;

	if (**JsonToParse == '"')
	{
		// Just a string
		*ResultType = SIMPLIFIED_JSON_OBJECT_DATA_TYPE_STRING;
		ReturnValue = JsonParseString(JsonToParse, MemoryToUse);
	}
	else if (isdigit(**JsonToParse) || **JsonToParse == '-') // Negative
	{
		// We'll copy that number over (as a string)
		*ResultType = SIMPLIFIED_JSON_OBJECT_DATA_TYPE_STRING;
		char *ResultString = (char *)*MemoryToUse;
		u32 ResultStringLength = 0;
		while (isdigit(**JsonToParse) || 
			   **JsonToParse == '.'   || // floating point
			   **JsonToParse == '-')  // Negative number
		{ 
			ResultString[ResultStringLength++] = **JsonToParse;
			*JsonToParse += 1; }
		ResultString[ResultStringLength++] = 0; // 0 Terminate
		ReturnValue = ResultString;
		*MemoryToUse = (void *)&ResultString[ResultStringLength];
	}
	else if (**JsonToParse == '{')
	{
		// It's an object
		*ResultType = SIMPLIFIED_JSON_OBJECT_DATA_TYPE_OBJECT_OR_ARRAY;
		ReturnValue = JsonParseObject(JsonToParse, MemoryToUse);
	}
	else if (**JsonToParse == '[')
	{
		// It's an array
		*ResultType = SIMPLIFIED_JSON_OBJECT_DATA_TYPE_OBJECT_OR_ARRAY;
		ReturnValue = JsonParseArray(JsonToParse, MemoryToUse);
	}
	else if (0 == strncmp(*JsonToParse, "true", strlen("true")))
	{
		*ResultType = SIMPLIFIED_JSON_OBJECT_DATA_TYPE_BOOLEAN32;
		b32 *Bool = (b32 *)*MemoryToUse;
		*MemoryToUse = (u8 *)(Bool+1);
		*Bool = 1;
		*JsonToParse += strlen("true");
		ReturnValue = Bool;
	}
	else if (0 == strncmp(*JsonToParse, "false", strlen("false")))
	{
		*ResultType = SIMPLIFIED_JSON_OBJECT_DATA_TYPE_BOOLEAN32;
		b32 *Bool = (b32 *)*MemoryToUse;
		*MemoryToUse = (u8 *)(Bool+1);
		*Bool = 0;
		*JsonToParse += strlen("false");
		ReturnValue = Bool;
	}
	else if (0 == strncmp(*JsonToParse, "null", strlen("null")))
	{
		*ResultType = SIMPLIFIED_JSON_OBJECT_DATA_TYPE_NULL;
		*JsonToParse += strlen("null");
	}
	else
	{
		fprintf(stderr, "Cannot parse json value %s\n", *JsonToParse);
		assert(!"Unexpected json value");
	}
	
	return (ReturnValue);
}

internal simplified_json_object *
JsonParseObject(char **JsonToParse, u8 **MemoryToUse)
{
	//fprintf(stderr, "PARSING OBJECT: %.*s\n", 64, *JsonToParse);
	assert(**JsonToParse == '{');
	simplified_json_object *Result = (void *)*MemoryToUse;
	*Result = (simplified_json_object ){ 0 };
	*MemoryToUse = (void *)(Result+1);
	
	*JsonToParse += 1;
	if (**JsonToParse == '}')
	{
		// Object is empty
	}
	else
	{
		assert(**JsonToParse == '"'); // Objects consists of "string:value"

		Result->Name = JsonParseString(JsonToParse, MemoryToUse);
		assert(**JsonToParse == ':');
		*JsonToParse += 1;
		Result->Data = JsonParseValue(JsonToParse, MemoryToUse, &Result->DataType);

		simplified_json_object **ToFill = &Result->Next;
		while (**JsonToParse != '}')
		{
			assert(**JsonToParse == ',');
			*JsonToParse += 1;

			*ToFill = (simplified_json_object *)*MemoryToUse;
			**ToFill = (simplified_json_object){ 0 };
			*MemoryToUse = (u8 *)(*ToFill + 1);

			(*ToFill)->Name = JsonParseString(JsonToParse, MemoryToUse);
			assert(**JsonToParse == ':');
			*JsonToParse += 1;
			(*ToFill)->Data = JsonParseValue(JsonToParse, MemoryToUse, &(*ToFill)->DataType);

			ToFill = &(*ToFill)->Next;
		}
	}
	
	*JsonToParse += 1;
	return (Result);
}

internal simplified_json_object *
JsonParseArray(char **JsonToParse, u8 **MemoryToUse)
{
	//fprintf(stderr, "PARSING ARRAY: %.*s\n", 64, *JsonToParse);
	assert(**JsonToParse == '[');
	*JsonToParse += 1;

	simplified_json_object *Result = (void *)*MemoryToUse;
	*Result = (simplified_json_object ){ 0 };
	*MemoryToUse = (void *)(Result+1);

	if (**JsonToParse == ']')
	{
		// Array is empty, don't do anything
	}
	else
	{
		Result->Data = JsonParseValue(JsonToParse, MemoryToUse, &Result->DataType);
		simplified_json_object **ToFill = &Result->Next;
		while (**JsonToParse != ']')
		{
			assert(**JsonToParse == ',');
			*JsonToParse += 1;

			*ToFill = (simplified_json_object *)*MemoryToUse;
			**ToFill = (simplified_json_object){ 0 };
			*MemoryToUse = (u8 *)(*ToFill + 1);
			(*ToFill)->Data = JsonParseValue(JsonToParse, MemoryToUse, &(*ToFill)->DataType);

			ToFill = &(*ToFill)->Next;
		}
	}
	*JsonToParse += 1;
	return (Result);
}

internal simplified_json_object *
JsonParseStringIntoTraversableObject(char *JsonToParse, void *MemoryToUse)
{
	u8 *Memory = MemoryToUse;
	if (*JsonToParse == '{')
	{
		return (JsonParseObject(&JsonToParse, &Memory));
	}
	else if (*JsonToParse == '[')
	{
		return (JsonParseArray(&JsonToParse, &Memory));
	}
	else
	{
		fprintf(stderr, "Json which cannot be parsed: %c\n", *JsonToParse);
		assert(!"Cannot parse json input");
		return (0);
	}
}
