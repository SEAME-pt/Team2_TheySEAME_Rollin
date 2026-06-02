#include <boost/python.hpp>
#include "PurePursuit.hpp"

BOOST_PYTHON_MODULE(purePursuit) {
	boost::python::class_<PurePursuit>("PurePursuit", boost::python::init<>())
		.def("control", &PurePursuit::control)
		.def("getAngle", &PurePursuit::getAngle);

	boost::python::class_<quadFunc>("quadFunc", boost::python::init<>())
		.def_readwrite("a", &quadFunc::a)
		.def_readwrite("b", &quadFunc::b)
		.def_readwrite("c", &quadFunc::c);
}
