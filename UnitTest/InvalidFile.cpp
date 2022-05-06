#include "pch.h"
#include "CppUnitTest.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using dds_loader::Loader;

namespace UnitTest
{
    TEST_CLASS(InvalidFile)
    {
        Loader m_LoaderDXT1;
    public:
        InvalidFile(){
            //"invalid.dds" == 拡張子は.ddsだがファイルの中身は.png
            m_LoaderDXT1.Load(MAKE_ABS_PATH("invalid.dds"));
        }

        TEST_METHOD(GetFourCCAsAsciiDump) {
            std::array<wchar_t, 32>      szField;
            const auto writeSize = m_LoaderDXT1.GetFourCCAsAsciiDump(szField.data(), szField.size());
            //1 == '\0'を書き込んだぶん
            Assert::AreEqual(writeSize, static_cast<size_t>(1));
            Assert::AreEqual(wcscmp(szField.data(), L"") == 0, true);
        }
        TEST_METHOD(GetReserved1AsWChar) {
            std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1)>      szField;
            const auto writeSize = m_LoaderDXT1.GetReserved1AsWChar(szField.data(), szField.size());
            //1 == '\0'を書き込んだぶん
            Assert::AreEqual(writeSize, static_cast<size_t>(1));
            Assert::AreEqual(wcscmp(szField.data(), L"") == 0, true);
        }

        TEST_METHOD(GetReserved1AsHexDump)
        {
            std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1HexDump)>      szField;
            const auto writeSize = m_LoaderDXT1.GetReserved1AsHexDump(szField.data(), szField.size());
            //1 == '\0'を書き込んだぶん
            Assert::AreEqual(writeSize, static_cast<size_t>(1));
            Assert::AreEqual(wcscmp(szField.data(), L"") == 0, true);
        }

    };
};
