
#include <iostream>
#include "backend/StudentManager.h"

using namespace std;

int main() {
    StudentManager manager;

    // Add one student of each type
    cout << "Adding students:" << endl;

    cout << manager.addStudent(
        "Undergraduate", "John", "Smith",
        "1001", "1234567890", "Greensboro"
    ) << endl;

    cout << manager.addStudent(
        "MS", "Jane", "Doe",
        "1002", "1234567891", "Charlotte"
    ) << endl;

    cout << manager.addStudent(
        "PhD", "Alex", "Brown",
        "1003", "1234567892", "Raleigh"
    ) << endl;

    // Display available courses
    cout << "\nAVAILABLE COURSES:" << endl;

    for (const Course& course : manager.getAvailableCourses()) {
        cout << course.code << " - " << course.name << endl;
    }

    // Register courses for all three students
    cout << "\nCOURSE REGISTRATION:" << endl;

    // Undergraduate
    cout << manager.registerStudent("1001", "CS101") << endl;
    cout << manager.registerStudent("1001", "CS201") << endl;
    cout << manager.registerStudent("1001", "CS301") << endl;

    // MS Student
    cout << manager.registerStudent("1002", "CS501") << endl;
    cout << manager.registerStudent("1002", "CS510") << endl;

    // PhD Student
    cout << manager.registerStudent("1003", "CS701") << endl;
    cout << manager.registerStudent("1003", "CS710") << endl;
    cout << manager.registerStudent("1003", "CS720") << endl;

    // Test assigning grades
    cout << "\nGRADE ASSIGNMENT:" << endl;

    // Undergraduate
    cout << manager.assignGrade("1001", "CS101", "A") << endl;
    cout << manager.assignGrade("1001", "CS201", "B") << endl;
    cout << manager.assignGrade("1001", "CS301", "C") << endl;

    // MS Student
    cout << manager.assignGrade("1002", "CS501", "A") << endl;
    cout << manager.assignGrade("1002", "CS510", "B") << endl;

    // PhD Student
    cout << manager.assignGrade("1003", "CS701", "A") << endl;
    cout << manager.assignGrade("1003", "CS710", "A") << endl;
    cout << manager.assignGrade("1003", "CS720", "B") << endl;

    // Test invalid grade
    cout << "\nINVALID GRADE TEST:" << endl;
    cout << manager.assignGrade("1001", "CS101", "Z") << endl;

    // Test project registration
    cout << "\nMS PROJECT REGISTRATION:" << endl;

    cout << manager.registerMSProject("1001", "Test Project") << endl;
    cout << manager.registerMSProject("1002", "AI Research") << endl;
    cout << manager.registerMSProject("1003", "Advanced Research") << endl;

    // Test dissertation registration
    cout << "\nDISSERTATION REGISTRATION:" << endl;

    cout << manager.registerDissertation("1001", "Test") << endl;
    cout << manager.registerDissertation("1002", "Test") << endl;
    cout << manager.registerDissertation("1003", "Machine Learning") << endl;

    // Display student information
    cout << "\nSTUDENT INFORMATION:" << endl;

    for (string id : {"1001", "1002", "1003"}) {
        Student* student = manager.findStudent(id);

        if (student != nullptr) {
            cout << "\nName: " << student->getFirstName()
                 << " " << student->getLastName() << endl;

            cout << "ID: " << student->getStudentID() << endl;
            cout << "Type: " << manager.getStudentType(id) << endl;

            cout << "Registered Courses:" << endl;

            for (const RegisteredCourse& course :
                student->getRegisteredCourses()) {
                cout << course.course.code
                    << " - " << course.course.name
                    << " | Grade: " << course.grade << endl;
            }

            cout << "GPA: " << student->calcGPA() << endl;
        }
    }

    cout << "\nDUPLICATE COURSE TEST:" << endl;
    cout << manager.registerStudent("1001", "CS101") << endl;

    cout << "\nDUPLICATE STUDENT ID TEST:" << endl;
    cout << manager.addStudent(
        "Undergraduate", "Test", "Student",
        "1001", "1234567890", "Greensboro"
    ) << endl;

    cout << "\nINVALID STUDENT ID TEST:" << endl;
    cout << manager.registerStudent("9999", "CS101") << endl;

    return 0;
}
