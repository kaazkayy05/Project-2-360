#pragma once
#include <string>
#include <vector>
#include <deque>

#include "UndergraduateStudent.h"
#include "MSStudent.h"
#include "PhDStudent.h"

using namespace std;

class StudentManager {
private:
    vector<UndergraduateStudent> undergraduateStudents;
    vector<MSStudent> msStudents;
    vector<PhDStudent> phdStudents;
    vector<Course> availableCourses;
    
public:
    StudentManager();

    bool addStudent(string type, string first, string last, string id, string phone, 
        string address);
    
    Student* findStudent(string id);
    string getStudentType(string id);
    const vector<Course>& getAvailableCourses() const;

    bool registerStudent(string id, string title);
    bool assignGrade(string id, string courseCode, string grade);
    string getMSProject(string id);
    string getDissertation(string id);
};