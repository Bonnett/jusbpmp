#pragma once

#include <iostream>
#include <string>

class filemetadata
{
public:
	filemetadata(void);
	~filemetadata(void);

	std::wstring title;
	std::wstring artist;
	std::wstring album;
	std::wstring genre;
	UINT rating;

	inline void dump(){std::wcout << title << L","<< artist << L","<< album << L","<< genre << L"," << rating << std::endl;};
};
