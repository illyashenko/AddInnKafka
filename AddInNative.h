#pragma once

#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "IMemoryManager.h"
#include "stdafx.h"
#include <stdio.h>
#include <wchar.h>
#include "AddInNative.h"
#include <string>
#include <vector>
#include <codecvt>
#include "OneCExtension.h"
#include "Utils.h"


///////////////////////////////////////////////////////////////////////////////
// class CAddInNative
class CAddInNative : public IComponentBase
{
public:
	enum Props
	{
		ePropHost = 0,
		ePropPort,
		ePropKey,
		ePropValue,
		ePropStr,
		ePropLast      // Always last
	};

	enum Methods
	{
		eMethSet = 0,
		eMethGet,
		eMethLast      // Always last
	};

	CAddInNative(void);
	virtual ~CAddInNative();
	// IInitDoneBase
	virtual bool ADDIN_API Init(void*);
	virtual bool ADDIN_API setMemManager(void* mem);
	virtual long ADDIN_API GetInfo();
	virtual void ADDIN_API Done();
	// ILanguageExtenderBase
	virtual bool ADDIN_API RegisterExtensionAs(WCHAR_T**);
	virtual long ADDIN_API GetNProps();
	virtual long ADDIN_API FindProp(const WCHAR_T* wsPropName);
	virtual const WCHAR_T* ADDIN_API GetPropName(long lPropNum, long lPropAlias);
	virtual bool ADDIN_API GetPropVal(const long lPropNum, tVariant* pvarPropVal);
	virtual bool ADDIN_API SetPropVal(const long lPropNum, tVariant* varPropVal);
	virtual bool ADDIN_API IsPropReadable(const long lPropNum);
	virtual bool ADDIN_API IsPropWritable(const long lPropNum);
	virtual long ADDIN_API GetNMethods();
	virtual long ADDIN_API FindMethod(const WCHAR_T* wsMethodName);
	virtual const WCHAR_T* ADDIN_API GetMethodName(const long lMethodNum, const long lMethodAlias);
	virtual long ADDIN_API GetNParams(const long lMethodNum);
	virtual bool ADDIN_API GetParamDefValue(const long lMethodNum, const long lParamNum, tVariant* pvarParamDefValue);
	virtual bool ADDIN_API HasRetVal(const long lMethodNum);
	virtual bool ADDIN_API CallAsProc(const long lMethodNum, tVariant* paParams, const long lSizeArray);
	virtual bool ADDIN_API CallAsFunc(const long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);
	// LocaleBase
	virtual void ADDIN_API SetLocale(const WCHAR_T* loc);

private:
	long findName(const wchar_t* names[], const wchar_t* name, const uint32_t size) const;
	bool wstring_to_variant(std::wstring& str, tVariant* val);
	bool string_to_variant(STRING& str, tVariant* val);
	bool variant_to_string(STRING& str, tVariant* val);
	bool string_to_retVariant(STRING& str, tVariant* retValue);
	void addError(uint32_t wcode, const wchar_t* source, const wchar_t* descriptor, long code);
	STRING narrow_string(std::wstring const& s, std::locale const& loc, char default_char = '?');
	// Attributes
	IAddInDefBase* m_iConnect;
	IMemoryManager* m_iMemory;
	// Redis
	STRING m_uiHost;
	int m_uiPort;

	std::wstring m_uiKey;
	std::wstring m_uiValue;

	std::basic_string<char16_t> m_uiStr;

	HANDLE  m_hTimer;
	HANDLE  m_hTimerQueue;
};

class WcharWrapper
{
public:
	WcharWrapper(const wchar_t* str);
	~WcharWrapper();

	operator const wchar_t* () { return m_str_wchar; }
	operator wchar_t* () { return m_str_wchar; }
private:
	WcharWrapper& operator = (const WcharWrapper& other) { return *this; }
	WcharWrapper(const WcharWrapper& other) { }
private:
	wchar_t* m_str_wchar;
};