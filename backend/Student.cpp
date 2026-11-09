#include "Student.h"

Student::Student(string first, string last, string id,
                 string phone, string addr) {
    firstName = first;
    lastName = last;
    studentID = id;
    phoneNumber = phone;
    address = addr;
}

string Student::getFirstName() const {
    return firstName;
}

string Student::getLastName() const {
    return lastName;
}

string Student::getStudentID() const {
    return studentID;
}

string Student::getPhoneNumber() const {
    return phoneNumber;
}

string Student::getAddress() const {
    return address;
}

bool Student::registerCourse(const Course& course) {
    for (const RegisteredCourse& current : registeredCourses) {
        if (current.course.code == course.code) {
            return false;
        }
    }

    registeredCourses.push_back({course, ""});
    return true;
}

bool Student::setCourseGrade(string courseCode, string grade) {
    if (grade != "A" && grade != "B" &&
        grade != "C" && grade != "D" && grade != "F") {
        return false;
    }

    for (RegisteredCourse& current : registeredCourses) {
        if (current.course.code == courseCode) {
            current.grade = grade;
            return true;
        }
    }

    return false;
}

const vector<RegisteredCourse>& Student::getRegisteredCourses() const {
    return registeredCourses;
}

double Student::calcGPA() const {
    double totalPoints = 0.0;
    int totalCredits = 0;

    for (const RegisteredCourse& current : registeredCourses) {
        double points = 0.0;
        string grade = current.grade;

        if (grade == "") {
            continue;
        }

        if (grade == "A") points = 4.0;
        else if (grade == "B") points = 3.0;
        else if (grade == "C") points = 2.0;
        else if (grade == "D") points = 1.0;
        else if (grade == "F") points = 0.0;

        totalPoints += points * current.course.credits;
        totalCredits += current.course.credits;
    }

    if (totalCredits == 0) {
        return 0.0;
    }

    return totalPoints / totalCredits;
}
