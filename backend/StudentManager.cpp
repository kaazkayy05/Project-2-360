#include "StudentManager.h"

StudentManager::StudentManager(){
    availableCourses = {
        {"CS101", "Introduction to Computer Science", 3},
        {"CS201", "Data Structures", 3},
        {"CS301", "Database Systems", 3},

        {"CS501", "Advanced Algorithms", 3},
        {"CS510", "Artificial Intelligence", 3},
        {"CS520", "Machine Learning", 3},

        {"CS701", "Advanced Research Methods", 3},
        {"CS710", "Doctoral Seminar", 3},
        {"CS720", "Advanced Computing Research", 3}
    };
}

bool StudentManager::addStudent(
    string type, string first, string last, string id, string phone,
    string address) {
        if (id.empty() || findStudent(id) != nullptr) {
            return false;
        }

        if (type == "Undergraduate") {
            undergraduateStudents.emplace_back(
                first, last, id, phone, address
            );
        }
        else if (type == "MS"){
            msStudents.emplace_back(
                first, last, id, phone, address
            );
        }
        else if (type == "PhD"){
            phdStudents.emplace_back(
                first, last, id, phone, address
            );
        }
        else{
            return false;
        }

        return true;
    }

    Student* StudentManager::findStudent(string id){
        for (UndergraduateStudent& student : undergraduateStudents){
            if (student.getStudentID() == id){
                return &student;
            }
        }

        for (MSStudent& student : msStudents){
            if(student.getStudentID() == id){
                return &student;
            }
        }
        for (PhDStudent& student : phdStudents){
            if(student.getStudentID() == id){
                return &student;
            }
        }

        return nullptr;
    }

    string StudentManager::getStudentType(string id){
        for (UndergraduateStudent& student : undergraduateStudents){
            if(student.getStudentID() == id){
                return student.getStudentType();
            }
        }

        for (MSStudent& student : msStudents){
            if (student.getStudentID() == id){
                return student.getStudentType();
            }
        }

        for (PhDStudent& student : phdStudents) {
            if (student.getStudentID() == id) {
                return student.getStudentType();
            }
        }

        return "";
    }

    const vector<Course>& StudentManager::getAvailableCourses() const {
        return availableCourses;
    }

    bool StudentManager::registerStudent(string id, string courseCode){
        Student* student = findStudent(id);
        if(student == nullptr){
            return false;
        }

        for (const Course& course : availableCourses) {
            if (course.code == courseCode){
                return student->registerCourse(course);
            }
        }

        return false;
    }

    bool StudentManager::assignGrade(string id, string courseCode, string grade){
        Student* student = findStudent(id);
        if(student == nullptr){
            return false;
        }

        return student->setCourseGrade(courseCode, grade);
    }

    bool StudentManager::registerMSProject(string id, string title){
        for (MSStudent& student : msStudents){
            if(student.getStudentID() == id){
                return student.registerMSProject(title);
            }
        }

        for (PhDStudent& student : phdStudents){
            if (student.getStudentID() == id){
                return student.registerMSProject(title);
            }
        }

        return false;
    }

    bool StudentManager::registerDissertation(string id, string title){
        for (PhDStudent& student : phdStudents){
            if(student.getStudentID() == id){
                return student.registerDissertaion(title);
            }
        }

        return false;

    }

    string StudentManager::getMSProject(string id){
        for (MSStudent& student : msStudents){
            if(student.getStudentID() == id){
                return student.getMSProject();
            }
        }

        for (PhDStudent& student : phdStudents){
            if (student.getStudentID() == id){
                return student.getMSProject();
            }
        }

        return "";
    }

    string StudentManager::getDissertation(string id){
        for (PhDStudent& student : phdStudents){
            if(student.getStudentID() == id){
                return student.getDissertation();
            }
        }

        return "";

    }