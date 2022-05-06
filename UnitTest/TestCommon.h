#pragma once

#define L_JOIN(a,b) a##b
#define JOIN(a,b)	L_JOIN(L,a##b)
#define MAKE_ABS_PATH(a) JOIN("../../UnitTest/Assets/",a)
