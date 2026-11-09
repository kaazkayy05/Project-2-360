#include "MSStudent.h"

string MSStudent::getStudentType() const{
    return "MS";
}

bool MSStudent::registerMSProject(const string& title){
    if (title.empty()){
        return false;
    }

    msProject = title;
    return true;
}

string MSStudent::getMSProject() const{
    return msProject;
}