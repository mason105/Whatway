// 从类型库向导中用“添加类”创建的计算机生成的 IDispatch 包装类
#ifndef _COMMX_
#define _COMMX_

#import "./lib/hundsun_com/HsCommX.dll" no_namespace
// CComm 包装类

class CComm : public COleDispatchDriver
{
public:
	CComm(){} // 调用 COleDispatchDriver 默认构造函数
	CComm(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CComm(const CComm& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

	// 特性
public:

	// 操作
public:


	// IComm 方法
public:
	long Connect()
	{
		long result;
		InvokeHelper(0x1, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		return result;
	}
	long Send()
	{
		long result;
		InvokeHelper(0x2, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		return result;
	}
	long Receive()
	{
		long result;
		InvokeHelper(0x3, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		return result;
	}
	void SetHead(long BranchNo, long FuncNo)
	{
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0x4, DISPATCH_METHOD, VT_EMPTY, NULL, parms, BranchNo, FuncNo);
	}
	long get_ErrorNo()
	{
		long result;
		InvokeHelper(0x5, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	CString get_ErrorMsg()
	{
		CString result;
		InvokeHelper(0x6, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		return result;
	}
	long DisConnect()
	{
		long result;
		InvokeHelper(0x7, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
		return result;
	}
	void AddField(LPCTSTR sField)
	{
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x9, DISPATCH_METHOD, VT_EMPTY, NULL, parms, sField);
	}
	void AddValue(LPCTSTR sValue)
	{
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0xa, DISPATCH_METHOD, VT_EMPTY, NULL, parms, sValue);
	}
	void SetRange(long FieldCount, long RowCount)
	{
		static BYTE parms[] = VTS_I4 VTS_I4 ;
		InvokeHelper(0xb, DISPATCH_METHOD, VT_EMPTY, NULL, parms, FieldCount, RowCount);
	}
	void MoveBy(long Step)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0xc, DISPATCH_METHOD, VT_EMPTY, NULL, parms, Step);
	}
	CString FieldByName(LPCTSTR FieldName)
	{
		CString result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0xd, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, FieldName);
		return result;
	}
	long get_RecordCount()
	{
		long result;
		InvokeHelper(0xe, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_Eof()
	{
		long result;
		InvokeHelper(0xf, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_Bof()
	{
		long result;
		InvokeHelper(0x10, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	void SetConnect()
	{
		InvokeHelper(0x11, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void Free()
	{
		InvokeHelper(0x8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	long get_SendTimeOut()
	{
		long result;
		InvokeHelper(0x12, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	void put_SendTimeOut(long newValue)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x12, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
	}
	long get_ReceiveTimeOut()
	{
		long result;
		InvokeHelper(0x13, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	void put_ReceiveTimeOut(long newValue)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x13, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
	}
	void Create()
	{
		InvokeHelper(0x14, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	long get_SectionIndex()
	{
		long result;
		InvokeHelper(0x15, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	void put_SectionIndex(long newValue)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x15, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
	}
	long get_SenderId()
	{
		long result;
		InvokeHelper(0x16, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	void put_SenderId(long newValue)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x16, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
	}
	void CreatePack()
	{
		InvokeHelper(0x17, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void FreePack()
	{
		InvokeHelper(0x18, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	CString get_Buffer()
	{
		CString result;
		InvokeHelper(0x1c, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		return result;
	}
	void put_Buffer(LPCTSTR newValue)
	{
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x1c, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
	}
	long SendBuffer(LPCTSTR value)
	{
		long result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x19, DISPATCH_METHOD, VT_I4, (void*)&result, parms, value);
		return result;
	}
	void CreateX(long Order)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x1a, DISPATCH_METHOD, VT_EMPTY, NULL, parms, Order);
	}
	long get_FieldCount()
	{
		long result;
		InvokeHelper(0x1b, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long ConnectX(long Protocol, LPCTSTR Address, long Port, long KeyCipher, LPCTSTR Key, long KeyLen)
	{
		long result;
		static BYTE parms[] = VTS_I4 VTS_BSTR VTS_I4 VTS_I4 VTS_BSTR VTS_I4 ;
		InvokeHelper(0x1d, DISPATCH_METHOD, VT_I4, (void*)&result, parms, Protocol, Address, Port, KeyCipher, Key, KeyLen);
		return result;
	}
	CString GetFieldName(long Index)
	{
		CString result;
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x1e, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, Index);
		return result;
	}
	long get_Connection()
	{
		long result;
		InvokeHelper(0x1f, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}

	// IComm 属性
public:

};
#endif
