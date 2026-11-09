#pragma once
#include <string>

using namespace std;

struct Course 
{
    string code;
    string name;
    int credits;
};

struct RegisteredCourse
{
    Course course;
    string grade;
};

