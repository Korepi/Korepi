//////////////////////////////////////////////////////////////////////////
//
// WinRegTest.cpp -- by Giovanni Dicanio
//
// Test some of the WinReg code
//
// NOTE --- Test Preparation ---
// In the folder containing this source file, there should be also a file
// "GioTest.reg". This REG file contains some initial data to load into
// the registry for this test.
//
//////////////////////////////////////////////////////////////////////////

#include "WinReg.hpp"   // Module to test

#include <exception>
#include <iostream>
#include <string>
#include <vector>


using std::pair;
using std::vector;
using std::wcout;
using std::wstring;

using winreg::RegKey;
using winreg::RegException;
using winreg::RegExpected;


//
// Test common RegKey methods
//
void Test()
{
    wcout << "\n *** Testing Common RegKey Methods *** \n\n";

    //
    // Test subkey and value enumeration
    //

    const wstring testSubKey = L"SOFTWARE\\GioTest";
    RegKey key{ HKEY_CURRENT_USER, testSubKey };

    vector<wstring> subKeyNames = key.EnumSubKeys();
    wcout << L"Subkeys:\n";
    for (const auto& s : subKeyNames)
    {
        wcout << L"  [" << s << L"]\n";
    }
    wcout << L'\n';

    vector<pair<wstring, DWORD>> values = key.EnumValues();
    wcout << L"Values:\n";
    for (const auto& v : values)
    {
        wcout << L"  [" << v.first << L"](" << RegKey::RegTypeToString(v.second) << L")\n";
    }
    wcout << L'\n';

    key.Close();


    //
    // Test SetXxxValue, GetXxxValue and TryGetXxxValue methods
    //

    key.Open(HKEY_CURRENT_USER, testSubKey);

    const DWORD testDw = 0x1234ABCD;
    const ULONGLONG testQw = 0xAABBCCDD11223344;
    const wstring testSz = L"CiaoTestSz";
    const wstring testExpandSz = L"%PATH%";
    const vector<BYTE> testBinary = { 0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33 };
    const vector<BYTE> testEmptyBinary; // used to test zero-length binary data array
    const vector<wstring> testMultiSz = { L"Hi", L"Hello", L"Ciao" };

    key.SetDwordValue(L"TestValueDword", testDw);
    key.SetQwordValue(L"TestValueQword", testQw);
    key.SetStringValue(L"TestValueString", testSz);
    key.SetExpandStringValue(L"TestValueExpandString", testExpandSz);
    key.SetMultiStringValue(L"TestValueMultiString", testMultiSz);
    key.SetBinaryValue(L"TestValueBinary", testBinary);
    key.SetBinaryValue(L"TestEmptyBinary", testEmptyBinary);
    // TODO: May add tests for other empty values, like empty string, etc.

    if (key.TrySetDwordValue(L"TestTryValueDword", testDw).Failed())
    {
        wcout << L"RegKey::TrySetDwordValue failed.\n";
    }

    if (key.TrySetQwordValue(L"TestTryValueQword", testQw).Failed())
    {
        wcout << L"RegKey::TrySetQwordValue failed.\n";
    }

    if (key.TrySetStringValue(L"TestTryValueString", testSz).Failed())
    {
        wcout << L"RegKey::TrySetStringValue failed.\n";
    }

    if (key.TrySetExpandStringValue(L"TestTryValueExpandString", testExpandSz).Failed())
    {
        wcout << L"RegKey::TrySetExpandStringValue failed.\n";
    }

    if (key.TrySetMultiStringValue(L"TestTryValueMultiString", testMultiSz).Failed())
    {
        wcout << L"RegKey::TrySetMultiStringValue failed.\n";
    }

    if (key.TrySetBinaryValue(L"TestTryValueBinary", testBinary).Failed())
    {
        wcout << L"RegKey::TrySetBinaryValue failed.\n";
    }

    if (key.TrySetBinaryValue(L"TestTryEmptyBinary", testEmptyBinary).Failed())
    {
        wcout << L"RegKey::TrySetBinaryValue failed with zero-length binary array.\n";
    }


    DWORD testDw1 = key.GetDwordValue(L"TestValueDword");
    if (testDw1 != testDw)
    {
        wcout << L"RegKey::GetDwordValue failed.\n";
    }

    if (auto testDw2 = key.TryGetDwordValue(L"TestTryValueDword"))
    {
        if (testDw2.GetValue() != testDw)
        {
            wcout << L"RegKey::TryGetDwordValue failed.\n";
        }
    }
    else
    {
        wcout << L"RegKey::TryGetDwordValue failed.\n";
    }

    DWORD typeId = key.QueryValueType(L"TestValueDword");
    if (typeId != REG_DWORD)
    {
        wcout << L"RegKey::QueryValueType failed for REG_DWORD.\n";
    }

    ULONGLONG testQw1 = key.GetQwordValue(L"TestValueQword");
    if (testQw1 != testQw)
    {
        wcout << L"RegKey::GetQwordValue failed.\n";
    }

    if (auto testQw2 = key.TryGetQwordValue(L"TestTryValueQword"))
    {
        if (testQw2.GetValue() != testQw)
        {
            wcout << L"RegKey::TryGetQwordValue failed.\n";
        }
    }
    else
    {
        wcout << L"RegKey::TryGetQwordValue failed.\n";
    }

    typeId = key.QueryValueType(L"TestValueQword");
    if (typeId != REG_QWORD)
    {
        wcout << L"RegKey::QueryValueType failed for REG_QWORD.\n";
    }

    wstring testSz1 = key.GetStringValue(L"TestValueString");
    if (testSz1 != testSz)
    {
        wcout << L"RegKey::GetStringValue failed.\n";
    }

    if (auto testSz2 = key.TryGetStringValue(L"TestTryValueString"))
    {
        if (testSz2.GetValue() != testSz)
        {
            wcout << L"RegKey::TryGetStringValue failed.\n";
        }
    }
    else
    {
        wcout << L"RegKey::TryGetStringValue failed.\n";
    }

    typeId = key.QueryValueType(L"TestValueString");
    if (typeId != REG_SZ)
    {
        wcout << L"RegKey::QueryValueType failed for REG_SZ.\n";
    }

    wstring testExpandSz1 = key.GetExpandStringValue(L"TestValueExpandString");
    if (testExpandSz1 != testExpandSz)
    {
        wcout << L"RegKey::GetExpandStringValue failed.\n";
    }

    if (auto testExpandSz2 = key.TryGetExpandStringValue(L"TestTryValueExpandString"))
    {
        if (testExpandSz2.GetValue() != testExpandSz)
        {
            wcout << L"RegKey::TryGetExpandStringValue failed.\n";
        }
    }
    else
    {
        wcout << L"RegKey::TryGetExpandStringValue failed.\n";
    }

    typeId = key.QueryValueType(L"TestValueExpandString");
    if (typeId != REG_EXPAND_SZ)
    {
        wcout << L"RegKey::QueryValueType failed for REG_EXPAND_SZ.\n";
    }

    vector<wstring> testMultiSz1 = key.GetMultiStringValue(L"TestValueMultiString");
    if (testMultiSz1 != testMultiSz)
    {
        wcout << L"RegKey::GetMultiStringValue failed.\n";
    }

    if (auto testMultiSz2 = key.TryGetMultiStringValue(L"TestTryValueMultiString"))
    {
        if (testMultiSz2.GetValue() != testMultiSz)
        {
            wcout << L"RegKey::TryGetMultiStringValue failed.\n";
        }
    }
    else
    {
        wcout << L"RegKey::TryGetMultiStringValue failed.\n";
    }

    typeId = key.QueryValueType(L"TestValueMultiString");
    if (typeId != REG_MULTI_SZ)
    {
        wcout << L"RegKey::QueryValueType failed for REG_MULTI_SZ.\n";
    }

    vector<BYTE> testBinary1 = key.GetBinaryValue(L"TestValueBinary");
    if (testBinary1 != testBinary)
    {
        wcout << L"RegKey::GetBinaryValue failed.\n";
    }

    if (auto testBinary2 = key.TryGetBinaryValue(L"TestTryValueBinary"))
    {
        if (testBinary2.GetValue() != testBinary)
        {
            wcout << L"RegKey::TryGetBinaryValue failed.\n";
        }
    }
    else
    {
        wcout << L"RegKey::TryGetBinaryValue failed.\n";
    }

    typeId = key.QueryValueType(L"TestValueBinary");
    if (typeId != REG_BINARY)
    {
        wcout << L"RegKey::QueryValueType failed for REG_BINARY.\n";
    }

    // Test the special case of zero-length binary array
    vector<BYTE> testEmptyBinary1 = key.GetBinaryValue(L"TestEmptyBinary");
    if (testEmptyBinary1 != testEmptyBinary)
    {
        wcout << L"RegKey::GetBinaryValue failed with zero-length binary data.\n";
    }

    if (auto testEmptyBinary2 = key.TryGetBinaryValue(L"TestTryEmptyBinary"))
    {
        if (testEmptyBinary2.GetValue() != testEmptyBinary)
        {
            wcout << L"RegKey::TryGetBinaryValue failed with zero-length binary data.\n";
        }
    }
    else
    {
        wcout << L"RegKey::TryGetBinaryValue failed.\n";
    }


    //
    // Remove some test values
    //

    key.DeleteValue(L"TestValueDword");
    key.DeleteValue(L"TestValueQword");
    key.DeleteValue(L"TestValueString");
    key.DeleteValue(L"TestValueExpandString");
    key.DeleteValue(L"TestValueMultiString");
    key.DeleteValue(L"TestValueBinary");

    key.DeleteValue(L"TestTryValueDword");
    key.DeleteValue(L"TestTryValueQword");
    key.DeleteValue(L"TestTryValueString");
    key.DeleteValue(L"TestTryValueExpandString");
    key.DeleteValue(L"TestTryValueMultiString");
    key.DeleteValue(L"TestTryValueBinary");
}


int main()
{
    const int kExitOk = 0;
    const int kExitError = 1;

    try
    {
        wcout << L"=========================================\n";
        wcout << L"*** Testing Giovanni Dicanio's WinReg ***\n";
        wcout << L"=========================================\n\n";

        Test();

        wcout << L"All right!! :)\n\n";
    }
    catch (const RegException& e)
    {
        wcout << L"\n*** Registry Exception: " << e.what();
        wcout << L"\n*** [Windows API error code = " << e.code() << L"]\n\n";
        return kExitError;
    }
    catch (const std::exception& e)
    {
        wcout << L"\n*** ERROR: " << e.what() << L'\n';
        return kExitError;
    }

    return kExitOk;
}
