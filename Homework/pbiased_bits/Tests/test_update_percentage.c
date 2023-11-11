#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

void test_updatePercentage(void) {
    float lowest = 50.0;
    float highest = 50.0;
    updatePercentage(&lowest, &highest, 40.0);
    CU_ASSERT_EQUAL(lowest, 40.0);
    CU_ASSERT_EQUAL(highest, 50.0);
    updatePercentage(&lowest, &highest, 60.0);
    CU_ASSERT_EQUAL(lowest, 40.0);
    CU_ASSERT_EQUAL(highest, 60.0);
}

int main() {
    CU_initialize_registry();
    CU_pSuite suite = CU_add_suite("updatePercentage_test", 0, 0);
    
    CU_add_test(suite, "test_updatePercentage", test_updatePercentage);
    
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return 0;
}