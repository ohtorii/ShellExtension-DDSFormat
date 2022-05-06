#include "pch.h"
#include "CppUnitTest.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using dds_loader::Loader;

namespace UnitTest
{
    TEST_CLASS(UnitTest)
    {
        Loader m_LoaderDXT1;
    public:
        UnitTest() {
            m_LoaderDXT1.Load(MAKE_ABS_PATH("DXT1.dds"));
        }

        TEST_METHOD(GetFourCC) {
            {
                const auto format = m_LoaderDXT1.GetFourCC();
                Assert::AreEqual(format[0], static_cast<BYTE>('D'));
                Assert::AreEqual(format[1], static_cast<BYTE>('X'));
                Assert::AreEqual(format[2], static_cast<BYTE>('T'));
                Assert::AreEqual(format[3], static_cast<BYTE>('1'));
            }
            {
                std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::FourCCAsciiDump)>      szField;
                const auto writeSize = m_LoaderDXT1.GetFourCCAsAsciiDump(szField.data(), szField.size());
                Assert::AreEqual(writeSize, static_cast<size_t>(Loader::MinimumBufferCount::FourCCAsciiDump));
                Assert::AreEqual(wcscmp(szField.data(), L"DXT1") == 0, true);
            }
            {
                std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::FourCCHexDump)>      szField;
                const auto writeSize = m_LoaderDXT1.GetFourCCAsHexDump(szField.data(), szField.size());
                Assert::AreEqual(writeSize, static_cast<size_t>(Loader::MinimumBufferCount::FourCCHexDump));
                Assert::AreEqual(wcscmp(szField.data(), L"44 58 54 31 ") == 0, true); //"DXT1"
            }
        }

        TEST_METHOD(GetReserved1) {

            {
                const auto reserved1 = m_LoaderDXT1.GetReserved1();
                Assert::AreEqual(reserved1[0], static_cast<BYTE>('G'));
                Assert::AreEqual(reserved1[1], static_cast<BYTE>('I'));
                Assert::AreEqual(reserved1[2], static_cast<BYTE>('M'));
                Assert::AreEqual(reserved1[3], static_cast<BYTE>('P'));
            }
            {
                std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1)>      szField;
                const auto writeCount = m_LoaderDXT1.GetReserved1AsWChar(szField.data(), szField.size());
                Assert::AreEqual(writeCount, static_cast<size_t>(12));
                Assert::AreEqual(wcscmp(szField.data(), L"GIMP-DDS\\\t\x03") == 0, true);
            }

            {
                std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1HexDump)>      szFieldEnough;
                const auto writeSize = m_LoaderDXT1.GetReserved1AsHexDump(szFieldEnough.data(), szFieldEnough.size());
                Assert::AreEqual(writeSize, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1HexDump));
                Assert::AreEqual(szFieldEnough[0], L'4');
                Assert::AreEqual(szFieldEnough[1], L'7');
                Assert::AreEqual(szFieldEnough[2], L' ');
                Assert::AreEqual(szFieldEnough[szFieldEnough.size() - 1], L'\0');
            }
            {
                std::array<wchar_t, 3>      szField3;
                const auto writeSize = m_LoaderDXT1.GetReserved1AsHexDump(szField3.data(), szField3.size());
                Assert::AreEqual(writeSize, static_cast<size_t>(3));
            }
            {
                std::array<wchar_t, 2>      szField2;
                const auto writeSize = m_LoaderDXT1.GetReserved1AsHexDump(szField2.data(), szField2.size());
                //1 == '\0'のぶん
                Assert::AreEqual(writeSize, static_cast<size_t>(1));
                Assert::AreEqual(szField2[0], L'\0');
            }
            {
                std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1AsciiDump)>      szField;
                const auto writeSize = m_LoaderDXT1.GetReserved1AsAsciiDump(szField.data(), szField.size());
                Assert::AreEqual(writeSize, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1AsciiDump));
                Assert::AreEqual(szField[0], L'G');
                Assert::AreEqual(szField[1], L'I');
                Assert::AreEqual(szField[2], L'M');
                Assert::AreEqual(szField[3], L'P');
            }
        }

        TEST_METHOD(GetMipMapCount) {
            Assert::AreEqual(m_LoaderDXT1.GetMipMapCount(),static_cast<DWORD>(7));
        }
        TEST_METHOD(GetDepth) {
            Assert::AreEqual(m_LoaderDXT1.GetDepth(),static_cast<DWORD>(0));
        }
        TEST_METHOD(GetRGBBitCount) {
            Assert::AreEqual(m_LoaderDXT1.GetRGBBitCount(),static_cast<DWORD>(0));
        }
        TEST_METHOD(GetDDPFFlags) {
            Loader m_LoaderDXT1;
            Assert::AreEqual(m_LoaderDXT1.Load(MAKE_ABS_PATH("DDPixelFormat_0xFFFFFFFF.dds")), true);
            std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::PixelFormat)>      szField;
            const auto writeCount = m_LoaderDXT1.GetDDPFFlags(szField.data(), szField.size());
            Assert::AreEqual(writeCount,static_cast<size_t>(0x50));
            //Assert::AreEqual(wcscmp(szField.data(),L"RGB")==0, true);
        }
    };
}