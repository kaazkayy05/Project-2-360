// COMP 360 - University Management System
// Frontend adapter only: uses StudentManager without changing the team's backend.
// One tab-separated command per line; fields are percent encoded.
// One JSON response per line.
#include "backend/StudentManager.h"
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::string jsonString(const std::string& text) {
    std::ostringstream out;
    out << '"';
    for (unsigned char c : text) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (c < 0x20) {
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(c) << std::dec;
                } else {
                    out << static_cast<char>(c);
                }
        }
    }
    out << '"';
    return out.str();
}

int fromHex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

std::string decodeField(const std::string& input) {
    std::string output;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '%' && i + 2 < input.size()) {
            int a = fromHex(input[i + 1]);
            int b = fromHex(input[i + 2]);
            if (a >= 0 && b >= 0) {
                output += static_cast<char>(a * 16 + b);
                i += 2;
                continue;
            }
        }
        output += input[i];
    }
    return output;
}

std::vector<std::string> parseFields(const std::string& line) {
    std::vector<std::string> fields;
    std::istringstream input(line);
    std::string field;
    while (std::getline(input, field, '\t')) fields.push_back(decodeField(field));
    // A trailing empty field is significant.
    if (!line.empty() && line.back() == '\t') fields.emplace_back();
    return fields;
}

std::string error(const std::string& message) {
    return "{\"ok\":false,\"error\":" + jsonString(message) + "}";
}

std::string success(const std::string& message, const std::string& student = "") {
    std::string answer = "{\"ok\":true,\"message\":" + jsonString(message);
    if (!student.empty()) answer += ",\"student\":" + student;
    return answer + "}";
}

std::string studentJson(StudentManager& manager, const std::string& id) {
    Student* student = manager.findStudent(id);
    if (!student) return "null";
    std::ostringstream out;
    out << "{\"id\":" << jsonString(student->getStudentID())
        << ",\"firstName\":" << jsonString(student->getFirstName())
        << ",\"lastName\":" << jsonString(student->getLastName())
        << ",\"phone\":" << jsonString(student->getPhoneNumber())
        << ",\"address\":" << jsonString(student->getAddress())
        << ",\"type\":" << jsonString(manager.getStudentType(id))
        << ",\"gpa\":" << std::fixed << std::setprecision(2) << student->calcGPA()
        << ",\"msProject\":" << jsonString(manager.getMSProject(id))
        << ",\"dissertation\":" << jsonString(manager.getDissertation(id))
        << ",\"courses\":[";
    bool first = true;
    for (const RegisteredCourse& registered : student->getRegisteredCourses()) {
        if (!first) out << ',';
        first = false;
        out << "{\"code\":" << jsonString(registered.course.code)
            << ",\"name\":" << jsonString(registered.course.name)
            << ",\"credits\":" << registered.course.credits
            << ",\"grade\":" << jsonString(registered.grade) << '}';
    }
    out << "]}";
    return out.str();
}

bool isBlank(const std::string& s) {
    for (unsigned char c : s) if (!std::isspace(c)) return false;
    return true;
}

std::string process(StudentManager& manager,
                    std::vector<std::string>& addedIDs,
                    const std::vector<std::string>& v) {
    if (v.empty()) return error("No command supplied.");
    const std::string& op = v[0];
    if (op == "PING") return success("Backend connected.");
    if (op == "LIST_COURSES") {
        std::ostringstream out;
        out << "{\"ok\":true,\"courses\":[";
        bool first = true;
        for (const Course& course : manager.getAvailableCourses()) {
            if (!first) out << ',';
            first = false;
            out << "{\"code\":" << jsonString(course.code)
                << ",\"name\":" << jsonString(course.name)
                << ",\"credits\":" << course.credits << '}';
        }
        return out.str() + "]}";
    }
    if (op == "LIST_STUDENTS") {
        std::ostringstream out;
        out << "{\"ok\":true,\"students\":[";
        bool first = true;
        for (const std::string& id : addedIDs) {
            if (manager.findStudent(id) == nullptr) continue;
            if (!first) out << ',';
            first = false;
            out << studentJson(manager, id);
        }
        return out.str() + "]}";
    }
    if (op == "GET_STUDENT") {
        if (v.size() != 2) return error("Student ID is required.");
        if (!manager.findStudent(v[1])) return error("Student not found. Check the ID.");
        return success("Student found.", studentJson(manager, v[1]));
    }
    if (op == "ADD_STUDENT") {
        if (v.size() != 7) return error("All six student fields are required.");
        for (std::size_t i = 1; i < v.size(); ++i)
            if (isBlank(v[i])) return error("All student fields are required.");
        if (manager.findStudent(v[4])) return error("That student ID already exists.");
        if (v[1] != "Undergraduate" && v[1] != "MS" && v[1] != "PhD")
            return error("Select Undergraduate, MS or PhD.");
        if (!manager.addStudent(v[1], v[2], v[3], v[4], v[5], v[6]))
            return error("Could not add student.");
        addedIDs.push_back(v[4]);
        return success("Student added successfully.", studentJson(manager, v[4]));
    }
    if (op == "REGISTER_COURSE") {
        if (v.size() != 3) return error("Student ID and course are required.");
        Student* student = manager.findStudent(v[1]);
        if (!student) return error("Student not found. Add the student first.");
        for (const RegisteredCourse& course : student->getRegisteredCourses())
            if (course.course.code == v[2]) return error("Student is already registered for this course.");
        bool found = false;
        for (const Course& course : manager.getAvailableCourses())
            if (course.code == v[2]) found = true;
        if (!found) return error("That course is not in the catalog.");
        if (!manager.registerStudent(v[1], v[2])) return error("Course registration failed.");
        return success("Course registered successfully.", studentJson(manager, v[1]));
    }
    if (op == "ASSIGN_GRADE") {
        if (v.size() != 4) return error("Student ID, course and grade are required.");
        Student* student = manager.findStudent(v[1]);
        if (!student) return error("Student not found.");
        if (v[3] != "A" && v[3] != "B" && v[3] != "C" && v[3] != "D" && v[3] != "F")
            return error("Grade must be A, B, C, D or F.");
        if (!manager.assignGrade(v[1], v[2], v[3]))
            return error("Student is not registered for this course.");
        return success("Grade updated successfully.", studentJson(manager, v[1]));
    }
    if (op == "REGISTER_PROJECT" || op == "REGISTER_DISSERTATION") {
        if (v.size() != 3 || isBlank(v[2])) return error("A student ID and title are required.");
        if (!manager.findStudent(v[1])) return error("Student not found.");
        const std::string type = manager.getStudentType(v[1]);
        if (op == "REGISTER_PROJECT") {
            if (type == "Undergraduate") return error("Only MS and PhD students can register MS projects.");
            if (!manager.registerMSProject(v[1], v[2])) return error("Project registration failed.");
            return success("MS project saved successfully.", studentJson(manager, v[1]));
        }
        if (type != "PhD") return error("Only PhD students can register dissertations.");
        if (!manager.registerDissertation(v[1], v[2])) return error("Dissertation registration failed.");
        return success("PhD dissertation saved successfully.", studentJson(manager, v[1]));
    }
    return error("Unknown command: " + op);
}
} // namespace

int main() {
    StudentManager manager;
    std::vector<std::string> addedIDs;
    std::string line;
    while (std::getline(std::cin, line)) {
        try {
            std::cout << process(manager, addedIDs, parseFields(line)) << '\n' << std::flush;
        } catch (const std::exception& ex) {
            std::cout << error(std::string("Backend error: ") + ex.what()) << '\n' << std::flush;
        }
    }
    return 0;
}
