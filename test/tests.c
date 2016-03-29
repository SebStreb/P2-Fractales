#include <stdlib.h>
#include <CUnit/basic.h>
#include "../libfractal/fractal.h"

void test1(void) {
	struct fractal * test = fractal_new("Test", 100, 30, 0.5, 0.4);
	CU_PASS("Test réussi !");
}

int setup(void) {
	return 0;
}

int teardown(void) {
	return 0;
}

int main(int argc, char const *argv[]) {
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();
	CU_pSuite pSuite = NULL;

	pSuite = CU_add_suite("Suite de tests : fractales", setup, teardown);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	if ((NULL == CU_add_test(pSuite, "test1", test1))
	) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_basic_show_failures(CU_get_failure_list());
	CU_cleanup_registry();
	return 0;
}
