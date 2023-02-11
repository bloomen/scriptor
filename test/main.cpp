#include "helper.h"

int
main(int argc, char** argv)
{
    set_utc();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
