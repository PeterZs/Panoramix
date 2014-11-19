
#include <iostream>
#include <random>

#include "../src/core/matlab.hpp"
#include "gtest/gtest.h"
#include "test_config.hpp"

using namespace panoramix;
using namespace test;

TEST(Matlab, Basic){

#ifdef USE_MATLAB
    EXPECT_TRUE(core::Matlab::IsBuilt());
    ASSERT_TRUE(core::Matlab::IsUsable());
    ASSERT_TRUE(core::Matlab::RunScript("x = 1;"));

    auto image = cv::imread(ProjectDataDirStrings::Normal + "/75.jpg");
    ASSERT_TRUE(core::Matlab::PutVariable("im", image));
    ASSERT_TRUE(core::Matlab::RunScript("imshow(im);"));
    ASSERT_TRUE(core::Matlab::RunScript("im = horzcat(im * 0.5, im);"));
    ASSERT_TRUE(core::Matlab::RunScript("imshow(im);"));
    core::Image newImage;
    ASSERT_TRUE(core::Matlab::GetVariable("im", newImage));

    cv::imshow("new image", newImage);
    cv::waitKey();

    core::ImageWithType<core::Vec<int, 3>> all123s(500, 500, core::Vec<int, 3>(1, 2, 3));
    ASSERT_TRUE(core::Matlab::PutVariable("all123s", all123s));
    ASSERT_TRUE(core::Matlab::RunScript("all246s = all123s * 2;"));
    core::ImageWithType<core::Vec<int, 3>> all246s;
    ASSERT_TRUE(core::Matlab::GetVariable("all246s", all246s));

    for (auto & i : all246s){
        ASSERT_TRUE(i == (core::Vec<int, 3>(2, 4, 6)));
    }
#else
    EXPECT_FALSE(core::Matlab::IsBuilt());
#endif   

}

TEST(Matlab, GeneralConversion){

#ifdef USE_MATLAB
    EXPECT_TRUE(core::Matlab::IsBuilt());
    ASSERT_TRUE(core::Matlab::IsUsable());
    auto points = std::vector<core::Point3>{core::Point3(1, 2, 3), core::Point3(4, 5, 6)};
    ASSERT_TRUE(core::Matlab::PutVariable("x", points));
    ASSERT_TRUE(core::Matlab::RunScript("x = x * 2;"));
    std::vector<core::Point3> npoints;
    ASSERT_TRUE(core::Matlab::GetVariable("x", npoints, true));

    ASSERT_EQ(points.size(), npoints.size());
    for (int i = 0; i < points.size(); i++){
        ASSERT_TRUE(points[i] * 2 == npoints[i]);
    }

#else
    EXPECT_FALSE(core::Matlab::IsBuilt());
#endif  

}



int main(int argc, char * argv[], char * envp[])
{
	testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
