#include "ccfile.hh"
#include <string>
#include <fstream>
#include <unordered_map>
#include <exception>

using namespace std;

namespace {
    unordered_map<int, std::ios::openmode> g_mode_hash = {
        {BINARY_MODE, ios::binary},
        {READ_MODE, ios::in},
        {WRITE_MODE, ios::out},
    };
}

class Ccfile {
enum Attribute {
    ONLY_READ,
    ONLY_WRITE
};

public:
    Ccfile(const string& file_path, FileMode mode) :
        attribute_((mode & READ_MODE) ? ONLY_READ : ONLY_WRITE),
        handler(file_path, g_mode_hash[mode]) {
    }
    ~Ccfile() {
        handler.close();
    }

    bool write(const string& line) {
        checkAttribute(attribute_);
        handler << "line" << std::endl;
    }

    string read_all() {
        checkAttribute(attribute_);
        string ret;
    }

    string read_line() {
        checkAttribute(attribute_);
        string ret;
        handler >> ret;
        return ret;
    }

    string read_size(uint64_t size) {
        checkAttribute(attribute_);
    }
private:
    void checkAttribute(Attribute attribute) {
        if (attribute_ != attribute) {
            if (attribute_ == ONLY_READ) {
                throw runtime_error("write in only-read file handler");
            } else if (attribute_ == ONLY_WRITE) {
                throw runtime_error("read from only-write file handler");
            }
        }
    }
private:
    Attribute attribute_;
    fstream handler;
};

