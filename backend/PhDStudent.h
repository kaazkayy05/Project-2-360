#pragma once
#include "Student.h"

using namespace std;

class PhDStudent : public Student {
private:
    string msProject;
    string dissertation;


public:
    using Student::Student;
    string getStudentType() const;

    bool registerMSProject(const string& title);
    bool registerDissertaion(const string& title);
    string getMSProject() const;
    string getDissertation() const;
};