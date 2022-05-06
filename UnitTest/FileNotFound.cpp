#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using dds_loader::Loader;

#define L_JOIN(a,b) a##b
#define JOIN(a,b)	L_JOIN(L,a##b)
#define MAKE_ABS_PATH(a) JOIN("../../UnitTest/Assets/",a)

namespace UnitTest
{
    TEST_CLASS(FileNotFound)
    {
    public:

        TEST_METHOD(Test_1)
        {
            Loader m_LoaderDXT1;
            Assert::AreEqual(m_LoaderDXT1.Load(L"FileNotFound"), false);

            std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::FourCCAsciiDump)>      szField;
            const auto writeSize = m_LoaderDXT1.GetFourCCAsAsciiDump(szField.data(), szField.size());
            //1 == '\0'ÇèëÇ´çûÇÒÇæÇ‘ÇÒ
            Assert::AreEqual(writeSize, static_cast<size_t>(1));
            Assert::AreEqual(wcscmp(szField.data(), L"") == 0, true);
        }
    };
};