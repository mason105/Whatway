#include "Sywg_CRC_DES.h"
#include "./config/ConfigManager.h"

CSywg_CRC_DES gSywg;

CSywg_CRC_DES::CSywg_CRC_DES(void)
{
}


CSywg_CRC_DES::~CSywg_CRC_DES(void)
{
}

BOOL CSywg_CRC_DES::Load()
{
	hinstLib = NULL;
	//hinstLib = LoadLibrary("C:\\workspace_cpp\\SywgTest\\Debug\\descrc.dll");
	std::string file =  gConfigManager::instance().m_sPath + "\\descrc.dll";
	hinstLib = LoadLibrary(file.c_str());
	if (hinstLib == NULL)
		return FALSE;

	CalCrc = NULL;
	dedes = NULL;
	desdone = NULL;
	desinit = NULL;
	dessetkey = NULL;
	endes = NULL;

	CalCrc = (PCalCRC) GetProcAddress(hinstLib, "CalCRC");
	if (CalCrc == NULL)
		goto error;

	dedes = (PDedes) GetProcAddress(hinstLib, "dedes");
	if (dedes == NULL)
		goto error;

	desdone = (PDesdone) GetProcAddress(hinstLib, "desdone");
	if (desdone == NULL)
		goto error;

	desinit = (PDesinit) GetProcAddress(hinstLib, "desinit");
	if (desinit == NULL)
		goto error;

	dessetkey = (PDessetkey) GetProcAddress(hinstLib, "dessetkey");
	if (dessetkey == NULL)
		goto error;

	endes = (PEndes) GetProcAddress(hinstLib, "endes");
	if (endes == NULL)
		goto error;

	return TRUE;

error:
	return FALSE;
}

void CSywg_CRC_DES::UnLoad()
{
	if (hinstLib != NULL)
	{
		FreeLibrary(hinstLib);
		hinstLib = NULL;
	}
}
