#include "PhDStudent.h"

string PhDStudent::getStudentType() const{
    return "PhD";
}

bool PhDStudent::registerMSProject(const string& title){
    if (title.empty()){
        return false;
    }

    msProject = title;
    return true;
}

bool PhDStudent::registerDissertaion(const string& title){
    if (title.empty()){
        return false;
    }

    dissertation = title;
    return true;
}

string PhDStudent::getMSProject() const{
    return msProject;
}

string PhDStudent::getDissertation() const{
    return dissertation;
}