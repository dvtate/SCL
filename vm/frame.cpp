//
// Created by tate on 02-05-20.
//

#include "frame.hpp"

uint64_t Frame::_uid = 0;



Frame::Frame() : id(_uid++) {

}