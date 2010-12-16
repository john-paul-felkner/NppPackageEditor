#pragma once

class DataField
{
public:
	
	DataField();
	~DataField(void);
	//template <class T>
	//__declspec(property(get = getValue, put = putValue))T Value;


private:
	int* i;
	wchar_t* str;
	union DATA {
		int* i;
		wchar_t* str;
	} data;
	template <class T>
	T getValue()
	{
		//type_info t = typeid(T);
		//if (t == 
		return default(T);
	}
	template <class T>
	void putValue()
	{

	}


};
