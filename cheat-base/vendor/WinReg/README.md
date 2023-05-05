# WinReg v6.1.0
## High-level C++ Wrapper Around the Low-level Windows Registry C-interface API

by Giovanni Dicanio

The Windows Registry C-interface API is  _very low-level_ and _hard_ to use.

I developed some **C++ wrappers** around this low-level Win32 API, to raise the semantic level, 
using C++ classes like `std::wstring`, `std::vector`, etc. instead of raw C-style buffers and 
low-level mechanisms. 

For example, the `REG_MULTI_SZ` registry type associated to double-NUL-terminated C-style strings 
is handled using a much easier higher-level `vector<wstring>`. My C++ code does the _translation_ 
between high-level C++ STL-based stuff and the low-level Win32 C-interface API.

Moreover, Win32 error codes are translated to C++ exceptions. 
However, note that if you [prefer checking return codes](https://blogs.msmvps.com/gdicanio/2022/07/13/exceptions-or-error-return-codes/),
there are also methods that follow this pattern.

The Win32 registry value types are mapped to C++ higher-level types according the following table:

| Win32 Registry Type  | C++ Type                     |
| -------------------- |:----------------------------:| 
| `REG_DWORD`          | `DWORD`                      |
| `REG_QWORD`          | `ULONGLONG`                  |
| `REG_SZ`             | `std::wstring`               |
| `REG_EXPAND_SZ`      | `std::wstring`               |
| `REG_MULTI_SZ`       | `std::vector<std::wstring>`  |
| `REG_BINARY`         | `std::vector<BYTE>`          |


This code is currently developed using **Visual Studio 2019** with **C++17** features enabled 
(`/std:c++17`). I have no longer tested the code with previous compilers. 
The code compiles cleanly at warning level 4 (`/W4`) in both 32-bit and 64-bit builds.

This is a **header-only** library, implemented in the **[`WinReg.hpp`](WinReg/WinReg.hpp)** 
header file.

`WinRegTest.cpp` contains some demo/test code for the library: check it out for some sample usage.

The library exposes four main classes:

* `RegKey`: a tiny efficient wrapper around raw Win32 `HKEY` handles
* `RegException`: an exception class to signal error conditions
* `RegResult`: a tiny wrapper around Windows Registry API `LSTATUS` error codes, 
returned by some `Try` methods (like `RegKey::TryOpen`)
* `RegExpected<T>`: an object that contains a value of type `T` 
(e.g. a `DWORD` read from the registry) on success, 
or an instance of a `RegResult`-wrapped return code on error

There are many member functions inside the `RegKey` class, that wrap several parts of the native 
C-interface Windows Registry API, in a convenient higher-level C++ way.

For example, you can simply open a registry key and get registry values with C++ code like this:

```c++
RegKey  key{ HKEY_CURRENT_USER, L"SOFTWARE\\SomeKey" };

DWORD   dw = key.GetDwordValue (L"SomeDwordValue");
wstring s  = key.GetStringValue(L"SomeStringValue");
```

You can also open a registry key using a two-step construction process:

```c++
RegKey key;
key.Open(HKEY_CURRENT_USER, L"SOFTWARE\\SomeKey");
```

The above code will throw an exception on error. If you prefer to check return codes, you can do 
that as well, using a `TryXxxx` method, e.g.:

```c++
RegKey key;
RegResult result = key.TryOpen(HKEY_CURRENT_USER, L"SOFTWARE\\SomeKey");
if (! result)
{
    //
    // Open failed.
    //
    // You can invoke the RegResult::Code and RegResult::ErrorMessage methods
    // for further details.
    //
    ...
}
```

You can also enumerate all the values under a given key with simple C++ code like this:

```c++
auto values = key.EnumValues();

for (const auto & v : values)
{
    //
    // Process current value:
    //
    //   - v.first  (wstring) is the value name
    //   - v.second (DWORD)   is the value type
    //
    ...
}
```

You can also use the `RegKey::TryGet...Value` methods, that return `RegExpected<T>` 
instead of throwing an exception on error:

```c++
//
// RegKey::TryGetDwordValue() returns a RegExpected<DWORD>;
// the returned RegExpected contains a DWORD on success, 
// or a RegResult instance on error.
//
// 'res' is a RegExpected<DWORD> in this case:
//
const auto res = key.TryGetDwordValue(L"SomeDwordValue");
if (res.IsValid())  // or simply:  if (res)
{
    //
    // All right: Process the returned value ...
    //
    // Use res.GetValue() to access the stored DWORD.
    //
}
else
{
    //
    // The method has failed: 
    //
    // The returned RegExpected contains a RegResult with an error code.
    // Use res.GetError() to access the RegResult object.
    //
}
```

**Version Note** WinReg v5.1.1 is the latest version in which the `TryGetXxxValue` methods return 
`std::optional<T>` (discarding the information about the error code).
Starting from v6.0.0, the `TryGetXxxxValue` methods return `RegExpected<T>` (which keeps 
the error information on failure).


Note that many methods are available in _two forms_: one that _throws an exception_ of type 
`RegException` on error (e.g. `RegKey::Open`), and another that _returns an error status object_ 
of type `RegResult` (e.g. `RegKey::TryOpen`) instead of throwing an exception.
In addition, as indicated above, some methods like the `RegKey::TryGet...Value` ones return 
`RegExpected` instead of throwing exceptions; in case of errors, the returned `RegExpected` 
contains a `RegResult` storing the error code.

You can take a look at the test code in `WinRegTest.cpp` for some sample usage.

The library stuff lives under the `winreg` namespace.

See the [**`WinReg.hpp`**](WinReg/WinReg.hpp) header for more details and **documentation**.

Thanks to everyone who contributed to this project with some additional features and constructive 
comments and suggestions.
