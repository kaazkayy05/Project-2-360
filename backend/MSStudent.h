#pragma once
#include "Student.h"

using namespace std;

class MSStudent : public Student {
private:
    string msProject;


public:
    using Student::Student;
    string getStudentType() const;

    bool registerMSProject(const string& title);
    string getMSProject() const;
};