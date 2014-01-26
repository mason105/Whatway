#ifndef _AGC_H
#define _AGC_H

#include <string>
#include <map>
#include <vector>

typedef struct field
{
	std::string name;
	std::string type;
	int size;
	bool bEncrypt;
	int pos; // ×Ö¶ÎÆ«ÒÆÁ¿

}FIELD;

typedef struct function
{
	std::string FuncId;

	std::string response; 

	int sizeRequest;
	int sizeResult;
	int sizeReturn;

	std::vector<FIELD> vRequest;
	std::vector<FIELD> vResult;
	std::vector<FIELD> vReturn;
}FUNC;

class AGC
{
public:
	AGC(void);
	~AGC(void);

	std::map<std::string, FUNC> m_Funcs;

public:
	bool LoadXML();
	int GetSizeByType(std::string type);

	FUNC GetFunctionById(std::string FuncId);

	bool FindFunctionById(std::string FuncId);
};

extern AGC gAGC;

#endif
