#include "pch.h"
#include "CppUnitTest.h"
#include "../loxrot/crontab.h"
#include "../loxrot/config.h"
#include "../loxrot/rotate.h"
//#include "../loxrot/config.h"

#include <iostream>

#include <windows.h>
#include <fstream>
#include <filesystem>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace loxrotTest
{
	TEST_CLASS(CrontabTest)
	{
	public:
		TEST_METHOD(General)
		{
			Crontab crontab;
			Assert::IsFalse(crontab.parse(L"* * * * * *"));
			Assert::IsFalse(crontab.parse(L"* - * *"));
			Assert::IsFalse(crontab.parse(L"* -2 * *"));
			Assert::IsFalse(crontab.parse(L"* -- * *"));
			Assert::IsFalse(crontab.parse(L"* 2, * *"));
			Assert::IsFalse(crontab.parse(L"* ,1 * *"));
			Assert::IsFalse(crontab.parse(L"* , * *"));
			Assert::IsTrue(crontab.parse(L"  * * * *  *"));
			Assert::IsTrue(crontab.parse(L"  * * * * *   "));
			Assert::IsFalse(crontab.parse(L"  * * * * *   abc"));
			Assert::IsFalse(crontab.parse(L"* ? * * *"));
			Assert::IsFalse(crontab.parse(L"* ? * * *"));
			Assert::IsFalse(crontab.parse(L"thisIsWrong"));
		}

		TEST_METHOD(Minutes)
		{
			Crontab crontab;
			Assert::IsTrue(crontab.parse(L"1/2 * * * *"));
			Assert::AreEqual(30, static_cast<int>(crontab.minutes.size()));
			for (int h = 0; h < crontab.minutes.size(); h++) {
				Assert::AreEqual(h * 2 + 1, crontab.minutes[h]);
			}

			Assert::IsTrue(crontab.parse(L"0/2 * * * *"));
			Assert::IsFalse(crontab.parse(L"x/ * * * *"));
			Assert::IsFalse(crontab.parse(L"/ * * * *"));
			Assert::IsFalse(crontab.parse(L"0/0 * * * *"));
			Assert::IsFalse(crontab.parse(L"/0 * * * *"));
			Assert::IsFalse(crontab.parse(L"/x * * * *"));
			Assert::IsFalse(crontab.parse(L"? * * * *"));
			Assert::IsFalse(crontab.parse(L"98 * * * *"));
			Assert::IsFalse(crontab.parse(L"-1 * * * *"));
			Assert::IsTrue(crontab.parse(L"2-4 * * * *"));
			Assert::IsFalse(crontab.parse(L"a * * * *"));
			Assert::IsFalse(crontab.parse(L"1,2,8, * * * *"));
			Assert::IsFalse(crontab.parse(L"1,2,8,9,120 * * * *"));
			Assert::IsFalse(crontab.parse(L"1,2,8,9,12,60 * * * *"));
			Assert::IsFalse(crontab.parse(L"1,2,8,9,,12 * * * *"));
			Assert::IsTrue(crontab.parse(L"1,0,8,9,12 * * * *"));
			Assert::IsFalse(crontab.parse(L"a * * * *"));
		}

		TEST_METHOD(Hours)
		{
			Crontab crontab;
			Assert::IsTrue(crontab.parse(L"* 1/2 * * *"));
			Assert::AreEqual(12, static_cast<int>(crontab.hours.size()));
			for (int h = 0; h < crontab.hours.size(); h++) {
				Assert::AreEqual(h * 2 + 1, crontab.hours[h]);
			}

			Assert::IsTrue(crontab.parse(L"* 0/2 * * *"));
			Assert::IsFalse(crontab.parse(L"* x/ * * *"));
			Assert::IsFalse(crontab.parse(L"* / * * *"));
			Assert::IsFalse(crontab.parse(L"* 0/0 * * *"));
			Assert::IsFalse(crontab.parse(L"* /0 * * *"));
			Assert::IsFalse(crontab.parse(L"* /x * * *"));
			Assert::IsFalse(crontab.parse(L"* ? * * *"));
			Assert::IsFalse(crontab.parse(L"* 98 * * *"));
			Assert::IsFalse(crontab.parse(L"* -1 * * *"));
			Assert::IsTrue(crontab.parse(L"* 2-4 * * *"));
			Assert::IsFalse(crontab.parse(L"* a * * *"));
			Assert::IsFalse(crontab.parse(L"* 1,2,8, * * *"));
			Assert::IsFalse(crontab.parse(L"* 1,2,8,9,120 * * *"));
			Assert::IsTrue(crontab.parse(L"* 1,2,8,9,12 * * *"));
			Assert::IsTrue(crontab.parse(L"* 1,0,8,9,12 * * *"));
			Assert::IsFalse(crontab.parse(L"* a * * *"));
		}

		TEST_METHOD(Days)
		{
			Crontab crontab;
			Assert::IsTrue(crontab.parse(L"* * 1/2 * *"));
			Assert::AreEqual(16, static_cast<int>(crontab.days.size()));
			for (int h = 0; h < crontab.days.size(); h++) {
				Assert::AreEqual(h * 2 + 1, crontab.days[h]);
			}

			Assert::IsFalse(crontab.parse(L"* * 0/2 * *"));
			Assert::IsFalse(crontab.parse(L"* * x/ * *"));
			Assert::IsFalse(crontab.parse(L"* * / * *"));
			Assert::IsFalse(crontab.parse(L"* * 0/0 * *"));
			Assert::IsFalse(crontab.parse(L"* * /0 * *"));
			Assert::IsFalse(crontab.parse(L"* * /x * *"));
			Assert::IsFalse(crontab.parse(L"* * ? * *"));
			Assert::IsFalse(crontab.parse(L"* * 98 * *"));
			Assert::IsFalse(crontab.parse(L"* * -1 * *"));
			Assert::IsTrue(crontab.parse(L"* * 2-4 * *"));
			Assert::IsFalse(crontab.parse(L"* * 2-4 * * *"));
			Assert::IsFalse(crontab.parse(L"* * a * *"));
			Assert::IsFalse(crontab.parse(L"* * 1,2,8, * *"));
			Assert::IsFalse(crontab.parse(L"* * 1,2,8,9,120 * *"));
			Assert::IsTrue(crontab.parse(L"* * 1,2,8,9,12 * *"));
			Assert::IsFalse(crontab.parse(L"* * 1,0,8,9,12 * *"));
			Assert::IsFalse(crontab.parse(L"* * a * *"));
		}

		TEST_METHOD(Months)
		{
			Crontab crontab;
			Assert::IsTrue(crontab.parse(L"* * * 1/2 *"));
			Assert::AreEqual(6, static_cast<int>(crontab.months.size()));
			for (int h = 0; h < crontab.months.size(); h++) {
				Assert::AreEqual(h * 2 + 1, crontab.months[h]);
			}

			Assert::IsTrue(crontab.parse(L"* * * 1/2 *"));
			Assert::IsFalse(crontab.parse(L"* * * x/ *"));
			Assert::IsFalse(crontab.parse(L"* * * / *"));
			Assert::IsFalse(crontab.parse(L"* * * 0/0 *"));
			Assert::IsFalse(crontab.parse(L"* * * /0 *"));
			Assert::IsFalse(crontab.parse(L"* * * /x *"));
			Assert::IsFalse(crontab.parse(L"* * * ? *"));
			Assert::IsFalse(crontab.parse(L"* * * 98 *"));
			Assert::IsFalse(crontab.parse(L"* * * -1 *"));
			Assert::IsTrue(crontab.parse(L"* * * 2-4 *"));
			Assert::IsFalse(crontab.parse(L"* * * 2-4 * *"));
			Assert::IsFalse(crontab.parse(L"* * * a *"));
			Assert::IsFalse(crontab.parse(L"* * * 1,2,8, *"));
			Assert::IsFalse(crontab.parse(L"* * * 1,2,8,9,120 *"));
			Assert::IsTrue(crontab.parse(L"* * * 1,2,8,9,11 *"));
			Assert::IsFalse(crontab.parse(L"* * * 1,0,8,9,12 *"));
			Assert::IsFalse(crontab.parse(L"* * * a *"));
		}

		TEST_METHOD(Weekdays)
		{
			Crontab crontab;
			Assert::IsTrue(crontab.parse(L"* * * * 1/2"));
			Assert::AreEqual(3, static_cast<int>(crontab.weekdays.size()));
			for (int h = 0; h < crontab.weekdays.size(); h++) {
				Assert::AreEqual(h * 2 + 1, crontab.weekdays[h]);
			}

			Assert::IsTrue(crontab.parse(L"* * * * 1/2"));
			Assert::IsFalse(crontab.parse(L"* * * * x/"));
			Assert::IsFalse(crontab.parse(L"* * * * /"));
			Assert::IsFalse(crontab.parse(L"* * * * 0/0"));
			Assert::IsFalse(crontab.parse(L"* * * * /0"));
			Assert::IsFalse(crontab.parse(L"* * * * /x"));
			Assert::IsFalse(crontab.parse(L"* * * * ?"));
			Assert::IsFalse(crontab.parse(L"* * * * 98"));
			Assert::IsFalse(crontab.parse(L"* * * * -1"));
			Assert::IsTrue(crontab.parse(L"* * * * 2-4"));
			Assert::IsFalse(crontab.parse(L"* * * * 2-4 *"));
			Assert::IsFalse(crontab.parse(L"* * * * a"));
			Assert::IsFalse(crontab.parse(L"* * * * 1,2,8,"));
			Assert::IsFalse(crontab.parse(L"* * * * 1,2,8,9,120"));
			Assert::IsTrue(crontab.parse(L"* * * * 1,2,6"));
			Assert::IsFalse(crontab.parse(L"* * * * 1,0,8,9,12"));
			Assert::IsFalse(crontab.parse(L"* * * * a *"));
		}
	};

	TEST_CLASS(MinAgeTest)
	{
	public:
		//TEST_METHOD(Seconds) {
		//	Config c;
		//	std::wstring m(L"4");
		//	Assert::AreEqual(c.convertToSeconds(m), 4);
		//}

		//TEST_METHOD(Minutes) {
		//	Config c;
		//	std::wstring m(L"4m");
		//	Assert::AreEqual(c.convertToSeconds(m), 240);
		//}

		//TEST_METHOD(Hours) {
		//	Config c;
		//	std::wstring m(L"4h");
		//	Assert::AreEqual(c.convertToSeconds(m), 1440);
		//}

		//TEST_METHOD(Days) {
		//	Config c;
		//	std::wstring m(L"4d");
		//	Assert::AreEqual(c.convertToSeconds(m), 345600);
		//}

		//TEST_METHOD(Weeks) {
		//	Config c;
		//	std::wstring m(L"4w");
		//	Assert::AreEqual(c.convertToSeconds(m), 2419200);
		//}

		//TEST_METHOD(Months) {
		//	Config c;
		//	std::wstring m(L"4M");
		//	Assert::AreEqual(c.convertToSeconds(m), 10368000);
		//}

		//TEST_METHOD(Years) {
		//	Config con;
		//	con.load(L"config.ini");
		//	con.getSection(L"section1");
		//	con.convertToSeconds(L"4y");
			//std::wstring m(L"4y");
			//Assert::AreEqual(c.convertToSeconds(m), 126144000);
		//}
	};

	TEST_CLASS(Program) {
	public:
		TEST_METHOD(General) {
			const std::wstring path(L"D:\\Code\\loxrot\\x64\\Debug\\test\\");
			std::filesystem::create_directory(path);

			std::wstring configContent(L"[Programname]\n"
				L"Directory = " + path + L"\n"
				L"FilePattern = ^.*\\.log$\n"
				L"KeepFiles = 4\n"
				L"Timer = * * * * *\n"
				L"MinAge = 1d\n"
				L"FirstCompress = 2\n"
				L"Simulation = false\n");
			std::wofstream out(std::wstring(path + L"config.conf"));
			out << configContent;
			out.close();

			HANDLE h = CreateFile(std::wstring(path + L"test.log").c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			WriteFile(h, "test", 4, NULL, NULL);
			SYSTEMTIME st;
			GetLocalTime(&st);
			st.wDay -= 2;
			FILETIME ft;
			SystemTimeToFileTime(&st, &ft);
			SetFileTime(h, &ft, 0, 0);
			CloseHandle(h);

			Config config;
			config.load(std::wstring(path + L"config.conf"));

			Rotate rotate;
			for (std::map<std::wstring, Config::Section>::iterator it = config.getConfigs().begin(); it != config.getConfigs().end(); it++) {
				// Perform log rotation
				rotate.doRotates((std::pair<std::wstring, Config::Section>*)(&(*it)));
			}

		}
	};
}
