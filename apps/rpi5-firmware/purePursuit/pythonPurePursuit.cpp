#include <boost/python.hpp>
#include "PurePursuit.hpp"

BOOST_PYTHON_MODULE(purePursuit) {
	boost::python::class_<PurePursuit>("PurePursuit", boost::python::init<>())
		.def("control", &PurePursuit::control)
		.def("getAngle", &PurePursuit::getAngle);
}
