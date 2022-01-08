//
// Created by 张育 on 2022/1/8.
//

#include <stdio.h>
#include "../cutest/CuTest.h"

CuSuite* db_engine_open_suite();

void RunAllTests(void) {
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuiteAddSuite(suite, db_engine_open_suite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

int main(void) {
    RunAllTests();
}
