#include <stdlib.h>
#include <CUnit/Basic.h>
#include "../libfractal/fractal.h"
#include "../stack/stack.h"

void testGetSetValue(void) {
	struct fractal * test = fractal_new("NULL", 1, 1, 0, 0);
	int x = 10;
	fractal_set_value(test, 1, 1, x);
	CU_ASSERT_EQUAL(x, fractal_get_value(test, 1, 1));
	fractal_free(test);
}

void testGetSetAv(void) {
	struct fractal * test = fractal_new("NULL", 1, 1, 0, 0);
	int x = 10;
	fractal_set_av(test, x);
	CU_ASSERT_EQUAL(x, fractal_get_av(test));
	fractal_free(test);
}

void testFractalesNames(void) {
	char * name = "Test";
	int largeur =100;
	int hauteur = 50;
	double a = 0.75;
	double b = -0.7;
	struct fractal * test = fractal_new(name, largeur, hauteur, a, b);
	test=fractal_fill(test);
	CU_ASSERT_STRING_EQUAL(name, fractal_get_name(test));
	fractal_free(test);
}

void testFractalesSize(void) {
	char * name = "Test";
	int largeur = 100;
	int hauteur = 50;
	double a = 0.75;
	double b = -0.7;
	struct fractal * test = fractal_new(name, largeur, hauteur, a, b);
	test=fractal_fill(test);
	CU_ASSERT_EQUAL(hauteur, fractal_get_height(test));
	CU_ASSERT_EQUAL(largeur, fractal_get_width(test));
	fractal_free(test);
}

void testFractalesAB(void) {
	char * name = "Test";
	int largeur = 100;
	int hauteur = 50;
	double a = 0.75;
	double b = -0.7;
	struct fractal * test = fractal_new(name, largeur, hauteur, a, b);
	test=fractal_fill(test);
	CU_ASSERT_EQUAL(a, fractal_get_a(test));
	CU_ASSERT_EQUAL(b, fractal_get_b(test));
	fractal_free(test);
}

void testFractalesAverage(void) {
	char * name = "Test";
	int largeur = 100;
	int hauteur = 50;
	double a = 0.75;
	double b = -0.7;
	double moyenne = 2.221400;
	struct fractal * test = fractal_new(name, largeur, hauteur, a, b);
	test=fractal_fill(test);
	CU_ASSERT_EQUAL(moyenne, fractal_get_av(test));
	fractal_free(test);
}

void testStackSize(void){
	node * stackTest = malloc(sizeof(node));
	int base = 0;
	CU_ASSERT_EQUAL(base, stack_length(stackTest));
	struct fractal * fill = fractal_new("Nom", 10, 10, 1, 0.6);
	int plus = 1;
	stack_push(&stackTest, fill);
	CU_ASSERT_EQUAL(plus, stack_length(stackTest));
	fractal_free(fill);
	free(stackTest);
}

void testStackPushPop(void){
	node * stackTest = malloc(sizeof(node));
	struct fractal * fill = fractal_new("Nom", 10, 10, 1, 0.6);
	stack_push(&stackTest, fill);
	struct fractal * same = stack_pop(&stackTest);
	CU_ASSERT_EQUAL(fill, same);
	fractal_free(fill);
	free(stackTest);
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

	if (
		NULL == CU_add_test(pSuite, "test get/set value", testGetSetValue) ||
		NULL == CU_add_test(pSuite, "test get/set average", testGetSetAv) ||
		NULL == CU_add_test(pSuite, "test de la librairie fractale, nom", testFractalesNames) ||
		NULL == CU_add_test(pSuite, "test de la librairie fractale, taille", testFractalesSize) ||
		NULL == CU_add_test(pSuite, "test de la librairie fractale, a et b", testFractalesAB) ||
		NULL == CU_add_test(pSuite, "test de la librairie fractale, moyenne", testFractalesAverage) ||
		NULL == CU_add_test(pSuite, "test de la librairie stack, taille", testStackSize) ||
		NULL == CU_add_test(pSuite, "test de la librairie stack, push et pop", testStackSize)
	) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_basic_show_failures(CU_get_failure_list());
	CU_cleanup_registry();

	printf("\n\n");
	return 0;
}
