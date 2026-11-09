#pragma once
#include "Student.h"

using namespace std;

class UndergraduateStudent : public Student {
public:
    using Student::Student;
    string getStudentType() const;
};