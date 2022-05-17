#include "pch.h"
#include "CppUnitTest.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using dds_loader::Loader;

namespace UnitTest
{
    TEST_CLASS(StructTest)
    {
        TEST_METHOD(SizeOf)
        {
            Assert::AreEqual(Loader::GetDDSHeaderDx7Size(), (size_t)128);
            Assert::AreEqual(Loader::GetHHSHeaderSize(), (size_t)128 + 20);
        }
        TEST_METHOD(AlignAs16_Stack)
        {
            Loader l1;
            Loader l2;

            Assert::AreEqual(((intptr_t)&l1) & 0xf, (intptr_t)0);
            Assert::AreEqual(((intptr_t)&l2) & 0xf, (intptr_t)0);
        }

        TEST_METHOD(AlignAs16_New)
        {
            auto *p = new Loader();
            Assert::AreEqual(((intptr_t)p) & 0xf, (intptr_t)0);
            delete p; p = nullptr;
        }
    };
};
