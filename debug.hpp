//
// Created by tate on 29-05-20.
//

#ifndef DLANG_DEBUG_HPP
#define DLANG_DEBUG_HPP

//#define DLANG_DEBUG_DEBUG_LOG 1

#ifdef DLANG_DEBUG_DEBUG_LOG
	#define DLANG_DEBUG_MSG(m) std::cout <<m;
	#define DLANG_DEBUG DLANG_DEBUG_MSG
#else
	#define DLANG_DEBUG_MSG(m)
#endif

#endif //DLANG_DEBUG_HPP
