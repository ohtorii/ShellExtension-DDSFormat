#include "pch.h"
#include "CppUnitTest.h"
#include <array>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using dds_loader::Loader;

#define L_JOIN(a,b) a##b
#define JOIN(a,b)	L_JOIN(L,a##b)
#define MAKE_ABS_PATH(a) JOIN("../../UnitTest/Assets/",a)

namespace UnitTest
{
    TEST_CLASS(UnitTest)
    {
    public:

        TEST_METHOD(FileNotFound)
        {
            Loader loader;
            Assert::AreEqual(loader.Load(L"FileNotFound"), false);

            std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::FourCC)>      szField;
            const auto writeSize = loader.GetFourCCAsWChar(szField.data(), szField.size());
            //1 == '\0'を書き込んだぶん
            Assert::AreEqual(writeSize, static_cast<size_t>(1));
            Assert::AreEqual(wcscmp(szField.data(), L"") == 0, true);
        }

        TEST_METHOD(GetFourCCAsWChar_InValidFile) {
            Loader loader;
            //"invalid.dds" == 拡張子は.ddsだがファイルの中身は.png
            Assert::AreEqual(loader.Load(MAKE_ABS_PATH("invalid.dds")), false);

            {
                std::array<wchar_t, 32>      szField;
                const auto writeSize = loader.GetFourCCAsWChar(szField.data(), szField.size());
                //1 == '\0'を書き込んだぶん
                Assert::AreEqual(writeSize, static_cast<size_t>(1));
                Assert::AreEqual(wcscmp(szField.data(), L"") == 0, true);
            }
            {
                std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1)>      szField;
                const auto writeSize = loader.GetReserved1AsWChar(szField.data(), szField.size());
                //1 == '\0'を書き込んだぶん
                Assert::AreEqual(writeSize, static_cast<size_t>(1));
                Assert::AreEqual(wcscmp(szField.data(), L"") == 0, true);
            }
            {
                std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1HexDump)>      szField;
                const auto writeSize = loader.GetReserved1AsHexDump(szField.data(), szField.size());
                //1 == '\0'を書き込んだぶん
                Assert::AreEqual(writeSize, static_cast<size_t>(1));
                Assert::AreEqual(wcscmp(szField.data(), L"") == 0, true);
            }
        }

        TEST_METHOD(GetFourCC) {
            Loader loader;
            Assert::AreEqual(loader.Load(MAKE_ABS_PATH("DXT1.dds")), true);

            {
                const auto format = loader.GetFourCC();
                Assert::AreEqual(format[0], static_cast<BYTE>('D'));
                Assert::AreEqual(format[1], static_cast<BYTE>('X'));
                Assert::AreEqual(format[2], static_cast<BYTE>('T'));
                Assert::AreEqual(format[3], static_cast<BYTE>('1'));
            }
            {
                std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::FourCC)>      szField;
                const auto writeSize = loader.GetFourCCAsWChar(szField.data(), szField.size());
                Assert::AreEqual(writeSize, static_cast<size_t>(Loader::MinimumBufferCount::FourCC));
                Assert::AreEqual(wcscmp(szField.data(), L"DXT1") == 0, true);
            }
        }

        TEST_METHOD(GetReserved1) {
            Loader loader;
            Assert::AreEqual(loader.Load(MAKE_ABS_PATH("DXT1.dds")), true);

            {
                const auto reserved1 = loader.GetReserved1();
                Assert::AreEqual(reserved1[0], static_cast<BYTE>('G'));
                Assert::AreEqual(reserved1[1], static_cast<BYTE>('I'));
                Assert::AreEqual(reserved1[2], static_cast<BYTE>('M'));
                Assert::AreEqual(reserved1[3], static_cast<BYTE>('P'));
            }
            {
                std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1)>      szField;
                const auto writeCount = loader.GetReserved1AsWChar(szField.data(), szField.size());
                Assert::AreEqual(writeCount, static_cast<size_t>(12));
                Assert::AreEqual(wcscmp(szField.data(), L"GIMP-DDS\\\t\x03") == 0, true);
            }

            {
                std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1HexDump)>      szFieldEnough;
                const auto writeSize = loader.GetReserved1AsHexDump(szFieldEnough.data(), szFieldEnough.size());
                Assert::AreEqual(writeSize, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1HexDump));
                Assert::AreEqual(szFieldEnough[0], L'4');
                Assert::AreEqual(szFieldEnough[1], L'7');
                Assert::AreEqual(szFieldEnough[2], L' ');
                Assert::AreEqual(szFieldEnough[szFieldEnough.size() - 1], L'\0');
            }
            {
                std::array<wchar_t, 3>      szField3;
                const auto writeSize = loader.GetReserved1AsHexDump(szField3.data(), szField3.size());
                Assert::AreEqual(writeSize, static_cast<size_t>(3));
            }
            {
                std::array<wchar_t, 2>      szField2;
                const auto writeSize = loader.GetReserved1AsHexDump(szField2.data(), szField2.size());
                //1 == '\0'のぶん
                Assert::AreEqual(writeSize, static_cast<size_t>(1));
                Assert::AreEqual(szField2[0], L'\0');
            }
            {
                std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1AsciiDump)>      szField;
                const auto writeSize = loader.GetReserved1AsAsciiDump(szField.data(), szField.size());
                Assert::AreEqual(writeSize, static_cast<size_t>(Loader::MinimumBufferCount::Reserved1AsciiDump));
                Assert::AreEqual(szField[0], L'G');
                Assert::AreEqual(szField[1], L'I');
                Assert::AreEqual(szField[2], L'M');
                Assert::AreEqual(szField[3], L'P');
            }
        }

        TEST_METHOD(GetMipMapCount) {
            Loader loader;
            Assert::AreEqual(loader.Load(MAKE_ABS_PATH("DXT1.dds")), true);
            Assert::AreEqual(loader.GetMipMapCount(),static_cast<DWORD>(7));
        }

        TEST_METHOD(GetDDPFFlags) {
            Loader loader;
            Assert::AreEqual(loader.Load(MAKE_ABS_PATH("8.dds")), true);
            std::array<wchar_t, static_cast<size_t>(Loader::MinimumBufferCount::PixelFormat)>      szField;
            const auto writeCount = loader.GetDDPFFlags(szField.data(), szField.size());
            Assert::AreEqual(writeCount,static_cast<size_t>(4));
            Assert::AreEqual(wcscmp(szField.data(),L"RGB")==0, true);
        }
    };
}