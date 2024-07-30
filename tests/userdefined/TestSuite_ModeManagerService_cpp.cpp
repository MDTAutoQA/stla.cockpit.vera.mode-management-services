/*
 * Copyright (C) 2021 Stellantis/MobileDrive and/or their affiliated companies. All rights reserved.
 *
 * This software, including documentation, is protected by copyright and controlled by
 * Stellantis/MobileDrive. All rights are reserved. Copying, including reproducing, storing,
 * adapting or translating any or all of this material requires the prior written
 * consent of Stellantis/MobileDrive jointly. This material also contains confidential information,
 * which may not be disclosed to others without the prior and joint written consent of Stellantis/MobileDrive.
 *
 * ****** About File **********
 * Unit Test for the Service Basic Functionality.
 */
#include "cpptest.h"
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string>
#include <orpheus/framework/utils/logging/Log.h>

// Include ModeManager Service headers
#include "ModeManagerService.h"
#include <ModeManagerServiceSkeleton.h>

using namespace std;
using namespace orpheus::service::ModeManagerService;

/* CPPTEST_TEST_SUITE_CODE_BEGIN AdditionalIncludes */
/* CPPTEST_TEST_SUITE_CODE_END AdditionalIncludes */

CPPTEST_CONTEXT("/stla.cockpit.vera.mode-management-services_Analysis/src/ModeManagerService.cpp");
CPPTEST_TEST_SUITE_INCLUDED_TO("/stla.cockpit.vera.mode-management-services_Analysis/src/ModeManagerService.cpp");

class TestSuite_ModeManagerService : public CppTest_TestSuite
{
public:
    CPPTEST_TEST_SUITE(TestSuite_ModeManagerService);
    CPPTEST_TEST_SUITE_SETUP(testSuiteSetUp);
    CPPTEST_TEST(test_requestModeUpdate);
    CPPTEST_TEST(test_popBackStack);
    CPPTEST_TEST(test_requestTransientModeUpdate);
    CPPTEST_TEST(test_setPendingRequest);
    CPPTEST_TEST(test_iRequestLaunchParams);
    CPPTEST_TEST_SUITE_TEARDOWN(testSuiteTearDown);
    CPPTEST_TEST_SUITE_END();

    static void testSuiteSetUp();
    static void testSuiteTearDown();

    void setUp();
    void tearDown();

    void test_requestModeUpdate();
    void test_popBackStack();
    void test_requestTransientModeUpdate();
    void test_setPendingRequest();
    void test_iRequestLaunchParams();

    ModeManagerService modeManagerService;
};

CPPTEST_TEST_SUITE_REGISTRATION(TestSuite_ModeManagerService);

void TestSuite_ModeManagerService::testSuiteSetUp()
{
    /* CPPTEST_TEST_SUITE_CODE_BEGIN TestSuiteSetUp */
    /* CPPTEST_TEST_SUITE_CODE_END TestSuiteSetUp */
}

void TestSuite_ModeManagerService::testSuiteTearDown()
{
    /* CPPTEST_TEST_SUITE_CODE_BEGIN TestSuiteTearDown */
    /* CPPTEST_TEST_SUITE_CODE_END TestSuiteTearDown */
}

void TestSuite_ModeManagerService::setUp()
{
    /* CPPTEST_TEST_SUITE_CODE_BEGIN TestCaseSetUp */
    /* CPPTEST_TEST_SUITE_CODE_END TestCaseSetUp */
}

void TestSuite_ModeManagerService::tearDown()
{
    /* CPPTEST_TEST_SUITE_CODE_BEGIN TestCaseTearDown */
    /* CPPTEST_TEST_SUITE_CODE_END TestCaseTearDown */
}

/* CPPTEST_TEST_CASE_BEGIN test_requestModeUpdate */
void TestSuite_ModeManagerService::test_requestModeUpdate()
{
    std::string expected_componentId{"com.abc.main"};
    std::string path{"testpath"};
    int result = modeManagerService.requestModeUpdate(expected_componentId, path);
    CPPTEST_ASSERT_EQUAL(0, result); // Check the result
}

void TestSuite_ModeManagerService::test_popBackStack()
{
    std::string expected_componentId{"com.abc.main"};
    std::string componentId_return{"NA"};
    std::string path{"testpath"};
    modeManagerService.requestModeUpdate(expected_componentId, path);
    modeManagerService.backStackSubscribe([&](std::string &componentId, std::string &path, std::string &error, void *ctx) -> int
                                          {
        std::cout << "Backstack callback componentId: " << componentId << " path: " << path << std::endl;
        componentId_return = componentId;
        return 0;
        }, nullptr);
    int result = modeManagerService.popBackStack();
    CPPTEST_ASSERT_EQUAL(expected_componentId, componentId_return);
    modeManagerService.backStackUnsubscribe();
    CPPTEST_ASSERT_EQUAL(0, result); // Check the result
}

void TestSuite_ModeManagerService::test_requestTransientModeUpdate()
{
    std::string expected_componentId{"com.abc.main"};
    // std::string path{"testpath"};
    int result = modeManagerService.requestTransientModeUpdate(expected_componentId); // Call the method on the instance
    CPPTEST_ASSERT_EQUAL(0, result);                                                  // Check the result
}

void TestSuite_ModeManagerService::test_setPendingRequest()
{
    std::string expected_componentId{"com.abc.main"};
    int result = modeManagerService.setPendingRequest(expected_componentId); // Call the method on the instance
    CPPTEST_ASSERT_EQUAL(0, result);                                         // Check the result
}

void TestSuite_ModeManagerService::test_iRequestLaunchParams()
{
    std::string errorReturn = "NA";
    std::string componentId = "com.stellantis.unittest";
    std::string requestUuid = "123qaz";
    std::string launchParams = "NA";
    int iRequestLaunchParamsReturn = modeManagerService.iRequestLaunchParams(
        componentId,
        requestUuid,
        [&](std::string &resComponentId, std::string &resLaunchParams, std::string &resRequestUuid, std::string &error, void *result_ctx) -> int
        {
            componentId = resComponentId;
            launchParams = resLaunchParams;
            requestUuid = resRequestUuid;
            errorReturn = error;
            return 0;
        },
        nullptr);

    CPPTEST_ASSERT_EQUAL(componentId, "com.stellantis.unittest");
    CPPTEST_ASSERT_EQUAL(requestUuid, "123qaz");
    CPPTEST_ASSERT_EQUAL(launchParams, "");
    CPPTEST_ASSERT_EQUAL(errorReturn, "");
    CPPTEST_ASSERT_EQUAL(iRequestLaunchParamsReturn, 0);
}