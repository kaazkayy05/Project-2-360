#pragma once
#include <string>
#include <vector>
#include "Course.h"

using namespace std;

class Student {
private:
    string firstName;
    string lastName;
    string studentID;
    string phoneNumber;
    string address;
    vector<RegisteredCourse> registeredCourses;

public:
    Student(string first, string last, string id, string phone, string address);

    string getFirstName() const;
    string getLastName() const;
    string getStudentID() const;
    string getPhoneNumber() const;
    string getAddress() const;

    bool registerCourse(const Course& course);
    bool setCourseGrade(string courseCode, string grade);
    const vector<RegisteredCourse>& getRegisteredCourses() const;
    double calcGPA() const;
};