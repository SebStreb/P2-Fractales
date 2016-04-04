#include <stdlib.h>
#include <CUnit/basic.h>
#include "../libfractal/fractal.h"

void testFractales(void) {
	char * name = "Test";
	int hauteur =100;
	int largeur = 50;
	double a = 0.75;
	double b = -0.7;
	double moyenne = 2.221400;
	struct fractal * test = fractal_new(name, hauteur, largeur, a, b);
	test=fractal_fill(test);
	CU_ASSERT_STRING_EQUAL(name, fractal_get_name(test));
	CU_ASSERT_EQUAL(hauteur, fractal_get_height(test));
	CU_ASSERT_EQUAL(largeur, fractal_get_width(test));
	CU_ASSERT_EQUAL(a, fractal_get_a(test));
	CU_ASSERT_EQUAL(b, fractal_get_b(test));
	CU_ASSERT_EQUAL(moyenne, fractal_get_av(test));
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

	if ((NULL == CU_add_test(pSuite, "test de la librairie fractale", testFractales))
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
