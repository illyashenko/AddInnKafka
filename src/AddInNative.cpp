#include "AddInNative.h"

#define TIME_LEN 65
#define BASE_ERRNO 7

static const std::locale LOCALE_RUS{ "Russian_Russia.1251" };

static const wchar_t* g_PropNames[] = {
	L"Host", 
	L"Topic", 
	L"Message", 
	L"Error", 
	L"GroupId", 
	L"AutoCommit"
};
static const wchar_t* g_MethodNames[] = {
	L"Send", 
	L"Read", 
	L"ConsumerInit", 
	L"ConsumerClose", 
	L"Commit", 
	L"ProducerInit",
	L"TopicCreate"
};

static const wchar_t* g_PropNamesRu[] = {
	L"Хост", 
	L"Топик", 
	L"Сообщение", 
	L"Ошибка", 
	L"ИдентификаторГруппы", 
	L"АвтоФиксация"
};
static const wchar_t* g_MethodNamesRu[] = {
	L"Отправить", 
	L"Читать", 
	L"ПотребительСоздать", 
	L"ПотребительЗакрыть", 
	L"Зафиксировать",
	L"ПоставщикСоздать",
	L"ТопикСоздать"
};

static const wchar_t g_kClassNames[] = L"AddInNativeExt";
static IAddInDefBase* pAsyncEvent = NULL;

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);
static AppCapabilities g_capabilities = eAppCapabilitiesInvalid;
static WcharWrapper s_names(g_kClassNames);
//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
	if (!*pInterface)
	{
		*pInterface = new CAddInNative;
		return (long)* pInterface;
	}
	return 0;
}
//---------------------------------------------------------------------------//
AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities)
{
	g_capabilities = capabilities;
	return eAppCapabilitiesLast;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
	if (!*pIntf)
		return -1;

	delete* pIntf;
	*pIntf = 0;
	return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
	return s_names;
}
//---------------------------------------------------------------------------//
VOID CALLBACK MyTimerProc(PVOID lpParam, BOOLEAN TimerOrWaitFired);
// CAddInNative
//---------------------------------------------------------------------------//
CAddInNative::CAddInNative()
{
	m_iMemory = 0;
	m_iConnect = 0;
	m_hTimerQueue = 0;
	m_uAutoCommit = true;
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{
	m_iConnect = (IAddInDefBase*)pConnection;
	return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo()
{
	// Component should put supported component technology version 
	// This component supports 2.0 version
	return 2000;
}
//---------------------------------------------------------------------------//
void CAddInNative::Done()
{
	if (m_hTimerQueue)
	{
		DeleteTimerQueue(m_hTimerQueue);
		m_hTimerQueue = 0;
	}
}
/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{
	const wchar_t* wsExtension = g_kClassNames;
	int iActualSize = ::wcslen(wsExtension) + 1;
	WCHAR_T* dest = 0;

	if (m_iMemory)
	{
		if (m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
			::convToShortWchar(wsExtensionName, wsExtension, iActualSize);
		return true;
	}

	return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps()
{
	// You may delete next lines and add your own implementation code here
	return ePropLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{
	long plPropNum = -1;
	wchar_t* propName = 0;

	::convFromShortWchar(&propName, wsPropName);
	plPropNum = findName(g_PropNames, propName, ePropLast);

	if (plPropNum == -1)
		plPropNum = findName(g_PropNamesRu, propName, ePropLast);

	delete[] propName;

	return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{
	if (lPropNum >= ePropLast)
		return NULL;

	wchar_t* wsCurrentName = NULL;
	WCHAR_T* wsPropName = NULL;
	int iActualSize = 0;

	switch (lPropAlias)
	{
	case 0: // First language
		wsCurrentName = (wchar_t*)g_PropNames[lPropNum];
		break;
	case 1: // Second language
		wsCurrentName = (wchar_t*)g_PropNamesRu[lPropNum];
		break;
	default:
		return 0;
	}

	iActualSize = (int)wcslen(wsCurrentName) + 1;

	if (m_iMemory && wsCurrentName)
	{
		if (m_iMemory->AllocMemory((void**)& wsPropName, iActualSize * sizeof(WCHAR_T)))
			::convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
	}

	return wsPropName;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
	switch (lPropNum) {
	case ePropHost:
		wstring_to_variant(m_uHost, pvarPropVal);
		break;
	case ePropTopic:
		wstring_to_variant(m_uTopic, pvarPropVal);
		break;
	case ePropMessage:
		wstring_to_variant(m_uMessage, pvarPropVal);
		break;
	case ePropError:
		wstring_to_variant(m_uError, pvarPropVal);
		break;
	case ePropGroupId:
		wstring_to_variant(m_uGroup_ig, pvarPropVal);
		break;
	case ePropAutoCommit:
		TV_VT(pvarPropVal) = VTYPE_BOOL;
		TV_BOOL(pvarPropVal) = m_uAutoCommit;
		break;
	default:
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant* varPropVal)
{
	switch (lPropNum) {
	case ePropHost:
		if (TV_VT(varPropVal) != VTYPE_PWSTR)
			return false;
		m_uHost = TV_WSTR(varPropVal);
		break;
	case ePropTopic:
		if (TV_VT(varPropVal) != VTYPE_PWSTR)
			return false;
		m_uTopic = TV_WSTR(varPropVal);
		break;
	case ePropMessage:
		if (TV_VT(varPropVal) != VTYPE_PWSTR)
			return false;
		m_uMessage = TV_WSTR(varPropVal);
		break;
	case ePropError:
		if (TV_VT(varPropVal) != VTYPE_PWSTR)
			return false;
		m_uError = TV_WSTR(varPropVal);
		break;
	case ePropGroupId:
		if (TV_VT(varPropVal) != VTYPE_PWSTR)
			return false;
		m_uGroup_ig = TV_WSTR(varPropVal);
		break;
	case ePropAutoCommit:
		if (TV_VT(varPropVal) != VTYPE_BOOL)
			return false;
		m_uAutoCommit = TV_BOOL(varPropVal);
		break;
	default:
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum)
{
	return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum)
{
	switch (lPropNum) {
	case ePropHost:
		return true;
	case ePropTopic:
		return true;
	case ePropMessage:
		return true;
	case ePropError:
		return true;
	case ePropGroupId:
		return true;
	case ePropAutoCommit:
		return true;
	default:
		return true;
	}
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods()
{
	return eMethLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T* wsMethodName)
{
	long plMethodNum = -1;
	wchar_t* name = 0;

	::convFromShortWchar(&name, wsMethodName);

	plMethodNum = findName(g_MethodNames, name, eMethLast);

	if (plMethodNum == -1)
		plMethodNum = findName(g_MethodNamesRu, name, eMethLast);

	delete[] name;

	return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum, const long lMethodAlias)
{
	if (lMethodNum >= eMethLast)
		return NULL;

	wchar_t* wsCurrentName = NULL;
	WCHAR_T* wsMethodName = NULL;
	int iActualSize = 0;

	switch (lMethodAlias)
	{
	case 0: // First language
		wsCurrentName = (wchar_t*)g_MethodNames[lMethodNum];
		break;
	case 1: // Second language
		wsCurrentName = (wchar_t*)g_MethodNamesRu[lMethodNum];
		break;
	default:
		return 0;
	}

	iActualSize = (int)wcslen(wsCurrentName) + 1;

	if (m_iMemory && wsCurrentName)
	{
		if (m_iMemory->AllocMemory((void**)& wsMethodName, iActualSize * sizeof(WCHAR_T)))
			::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
	}

	return wsMethodName;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eMethSend:
		return 0;
	case eMethRead:
		return 0;
	case eMethConsumerInit:
		return 0;
	case eMethConsumerClose:
		return 0;
	case eMethCommit:
		return 0;
	case eMethProducerInit:
		return 0;
	case eMethTopicCreate:
		return 0;
	default:
		return 0;
	}
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
	tVariant* pvarParamDefValue)
{
	TV_VT(pvarParamDefValue) = VTYPE_EMPTY;

	switch (lMethodNum)
	{
	case eMethSend:
		break;
	case eMethRead:
		break;
	case eMethConsumerInit:
		break;
	case eMethConsumerClose:
		break;
	case eMethCommit:
		break;
	case eMethProducerInit:
		break;
	case eMethTopicCreate:
		break;
	default:
		return false;
	}
	return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eMethSend:
		return true;
	case eMethRead:
		return true;
	default:
		return false;
	}
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum, tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{
	case eMethConsumerInit: {
		return consumerInit();
	}
	case eMethConsumerClose: {

		if (this->_consumer && !this->_consumer->closed())
		{
			this->_consumer->close();
		}

		return true;
	}
	case eMethCommit: {

		if (_consumer)
		{
			_consumer->commitSync(current_message.get());
		}

		return true;
	}
	case eMethProducerInit: {
		return producerInit();
	}
	case eMethTopicCreate: {
		return topicCreate();
	}

	default:
		return false;
	}
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{
	case eMethSend: {

		auto message_ = narrow_string(m_uMessage, LOCALE_RUS);
		auto topic_ = narrow_string(m_uTopic, LOCALE_RUS);

		RdKafka::ErrorCode response = _producer->produce(topic_, RdKafka::Topic::PARTITION_UA,
			RdKafka::Producer::RK_MSG_COPY, (void*)(message_.c_str()), message_.length() + 1, NULL, 0, 0, NULL);

		std::string ansewer{ RdKafka::err2str(response) };

		_producer->poll(0);
		auto err = _producer->flush(10000);

		if (err)
		{
			ansewer = RdKafka::err2str(err);
		}
		
		return string_to_retVariant(ansewer, pvarRetValue) ? true : false;
	}
	case eMethRead: {
		
		std::shared_ptr<RdKafka::Message> msg(this->_consumer->consume(1000));

		std::string ansewer{};

		if (msg->err() == RdKafka::ERR_NO_ERROR) {

			std::ostringstream s_ansewer;
			
			s_ansewer <<'{' << ' ';
			s_ansewer << '"' << "topic" << '"' << ':';
			s_ansewer << '"' <<  msg->topic_name() << '"' << ',' << ' ';
			s_ansewer << '"' << "data" << '"' << ':' << static_cast<const char*>(msg->payload());
			s_ansewer << ' ' << '}';
			
			ansewer = s_ansewer.str();
			current_message = msg;
		}

		return string_to_retVariant(ansewer, pvarRetValue) ? true : false;	
	}
	default:
		return false;
	}
}
//---------------------------------------------------------------------------//
// This code will work only on the client!
VOID CALLBACK MyTimerProc(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	if (!pAsyncEvent)
		return;
	DWORD dwTime = 0;
	wchar_t* who = L"ComponentNative", * what = L"Timer";

	wchar_t* wstime = new wchar_t[TIME_LEN];
	if (wstime)
	{
		wmemset(wstime, 0, TIME_LEN);
		time_t vtime;
		time(&vtime);
		::_ui64tow_s(vtime, wstime, TIME_LEN, 10);
		pAsyncEvent->ExternalEvent(who, what, wstime);
		delete[] wstime;
	}
}
//---------------------------------------------------------------------------//
void CAddInNative::SetLocale(const WCHAR_T* loc)
{
	_wsetlocale(LC_ALL, L"");
}
/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void* mem)
{
	m_iMemory = (IMemoryManager*)mem;
	return m_iMemory != 0;
}
//---------------------------------------------------------------------------//
void CAddInNative::addError(uint32_t wcode, const wchar_t* source, const wchar_t* descriptor, long code)
{
	if (m_iConnect)
	{
		WCHAR_T* err = 0;
		WCHAR_T* descr = 0;

		::convToShortWchar(&err, source);
		::convToShortWchar(&descr, descriptor);

		m_iConnect->AddError(wcode, err, descr, code);
		delete[] err;
		delete[] descr;
	}
}
//---------------------------------------------------------------------------//
long CAddInNative::findName(const wchar_t* names[], const wchar_t* name, const uint32_t size) const
{
	long ret = -1;
	for (uint32_t i = 0; i < size; i++)
	{
		if (!wcscmp(names[i], name))
		{
			ret = i;
			break;
		}
	}
	return ret;
}
//---------------------------------------------------------------------------//
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len)
{
	if (!len)
		len = (int)::wcslen(Source) + 1;

	if (!*Dest)
		* Dest = new WCHAR_T[len];

	WCHAR_T* tmpShort = *Dest;
	wchar_t* tmpWChar = (wchar_t*)Source;
	uint32_t res = 0;

	::memset(*Dest, 0, len * sizeof(WCHAR_T));

	for (; len; --len, ++res, ++tmpWChar, ++tmpShort)
	{
		*tmpShort = (WCHAR_T)* tmpWChar;
	}

	return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
	if (!len)
		len = getLenShortWcharStr(Source) + 1;

	if (!*Dest)
		* Dest = new wchar_t[len];

	wchar_t* tmpWChar = *Dest;
	WCHAR_T* tmpShort = (WCHAR_T*)Source;
	uint32_t res = 0;

	::memset(*Dest, 0, len * sizeof(wchar_t));

	for (; len; --len, ++res, ++tmpWChar, ++tmpShort)
	{
		*tmpWChar = (wchar_t)* tmpShort;
	}

	return res;
}
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
	uint32_t res = 0;
	WCHAR_T* tmpShort = (WCHAR_T*)Source;

	while (*tmpShort++)
		++res;

	return res;
}

//---------------------------------------------------------------------------//
WcharWrapper::WcharWrapper(const wchar_t* str) : m_str_wchar(NULL)
{
	if (str)
	{
		int len = (int)wcslen(str);
		m_str_wchar = new wchar_t[len + 1];
		memset(m_str_wchar, 0, sizeof(wchar_t) * (len + 1));
		memcpy(m_str_wchar, str, sizeof(wchar_t) * len);
	}

}
//---------------------------------------------------------------------------//
WcharWrapper::~WcharWrapper()
{
	if (m_str_wchar)
	{
		delete[] m_str_wchar;
		m_str_wchar = NULL;
	}
}
//---------------------------------------------------------------------------//

bool CAddInNative::wstring_to_variant(std::wstring& str, tVariant* val) {

	char* t1;

	TV_VT(val) = VTYPE_PWSTR;

	m_iMemory->AllocMemory((void**)&t1, (str.length() + 1) * sizeof(WCHAR_T));
	memcpy(t1, str.c_str(), (str.length() + 1) * sizeof(WCHAR_T));

	val->pstrVal = t1;
	val->strLen = str.length();
	
	return true;
}

bool CAddInNative::string_to_variant(std::string& str, tVariant* val)
{
	WCHAR_T* wsPropValue = 0;

	TV_VT(val) = VTYPE_PWSTR;

	std::wstring wTemp(str.begin(), str.end());

	int isActualSize = wTemp.length() + 1;

	if (m_iMemory)
	{
		if (m_iMemory->AllocMemory((void**)&wsPropValue, isActualSize * sizeof(WCHAR_T)))
			::convToShortWchar(&wsPropValue, wTemp.c_str(), isActualSize);
	}

	TV_WSTR(val) = wsPropValue;
	val->wstrLen = isActualSize;

	return false;
}

bool CAddInNative::variant_to_string(std::string& str, tVariant* val)
{
	std::locale lok("rus");
	std::wstring wstr = TV_WSTR(val);
	//std::string _str(wstr.begin(), wstr.end());
	str = narrow_string(wstr, lok);

	return true;
}

bool CAddInNative::string_to_retVariant(std::string& str, tVariant* retValue)
{
	bool result = false;

	if (m_iMemory->AllocMemory((void**)&retValue->pstrVal, (str.length() + 1) * sizeof(CHAR)))
	{
		memcpy(retValue->pstrVal, str.c_str(), str.length() * sizeof(CHAR));
		TV_VT(retValue) = VTYPE_PSTR;
		retValue->strLen = str.length();
		result = true;
	}

	return result;
}

std::string CAddInNative::narrow_string(std::wstring const& s, std::locale const& loc, char default_char)
{
	if (s.empty()) 
	{
		return std::string();
	}

	std::ctype<wchar_t> const& facet = std::use_facet<std::ctype<wchar_t> >(loc);

	wchar_t const* first = s.c_str();
	wchar_t const* last = first + s.size();

	std::vector<char> result(s.size());

	facet.narrow(first, last, default_char, &result[0]);

	return std::string(result.begin(), result.end());
}

bool CAddInNative::consumerInit()
{
	std::string errstr;

	auto host_ = narrow_string(m_uHost, LOCALE_RUS);
	auto topic_ = narrow_string(m_uTopic, LOCALE_RUS);
	auto group_id = narrow_string(m_uGroup_ig, LOCALE_RUS);

	if (host_.empty() || topic_.empty())
	{
		m_uError = L"Failed: HOST or TOPIC is empty";
		return false;
	}

	std::regex rdelim{ ";" };
	auto topics = split(topic_, rdelim);

	if (topics.empty())
	{
		m_uError = L"Error: split TOPIC's";
		return false;
	}

	std::unique_ptr<RdKafka::Conf> conf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

	conf->set("bootstrap.servers", host_, errstr);
	conf->set("auto.offset.reset", "earliest", errstr);
	conf->set("group.id", group_id.c_str(), errstr);

	if (!m_uAutoCommit)
	{
		conf->set("enable.auto.commit", "false", errstr);
	}

	std::shared_ptr<RdKafka::KafkaConsumer> consumer(RdKafka::KafkaConsumer::create(conf.get(), errstr));

	this->_consumer = consumer;

	if (!this->_consumer) {
		std::wstring werrstr(errstr.begin(), errstr.end());
		m_uError = L"Failed to create consumer: " + werrstr;
		return false;
	}

	RdKafka::ErrorCode err = this->_consumer->subscribe(topics);

	if (err)
	{
		auto errdkafka = RdKafka::err2str(err);
		std::wstring werr(errdkafka.begin(), errdkafka.end());
		m_uError = L"Failed to subscribe to topics: " + werr;
		return false;
	}

	return true;
}

bool CAddInNative::producerInit()
{
	std::string errstr;
	auto host_ = narrow_string(m_uHost, LOCALE_RUS);

	std::unique_ptr<RdKafka::Conf> conf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

	conf->set("metadata.broker.list", host_.c_str(), errstr);

	std::shared_ptr<RdKafka::Producer> producer(RdKafka::Producer::create(conf.get(), errstr));
	this->_producer = producer;

	if (!this->_producer)
	{
		std::wstring werrstr(errstr.begin(), errstr.end());
		m_uError = L"Failed to create Producer: " + werrstr;
		return false;
	}

	return true;
}

bool CAddInNative::topicCreate()
{
	auto topic_name = narrow_string(m_uTopic, LOCALE_RUS);
	std::string errstr{};

	if (!this->_producer)
	{
		m_uError = L"Failed to create Producer";
		return false;
	}

	std::unique_ptr<RdKafka::Conf> tconf(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));
	std::unique_ptr<RdKafka::Topic> topic(RdKafka::Topic::create(this->_producer.get(), topic_name, tconf.get(), errstr));

	if (!topic)
	{
		std::wstring werrstr(errstr.begin(), errstr.end());
		m_uError = L"Failed to create Topic:" + m_uTopic + werrstr;
		return false;
	}

	return true;
}

std::vector<std::string> CAddInNative::split(std::string& str_value, const std::regex& rdelim)
{
	std::vector<std::string> topics{
		   std::sregex_token_iterator(str_value.begin(), str_value.end(), rdelim, -1),
		   std::sregex_token_iterator()
	};

	return topics;
}
