/*
 * parser_test.h
 *
 *  Created on: 15 thg 12, 2025
 *      Author: nguye
 */

#ifndef UNIT_TESTS_PARSER_TEST_H_
#define UNIT_TESTS_PARSER_TEST_H_

#include <stdbool.h>

#define TLV_PARSER_TEST  1

#ifdef __cplusplus
extern "C" {
#endif

// Run all TLV parser tests once
void parser_run_all_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* UNIT_TESTS_PARSER_TEST_H_ */
