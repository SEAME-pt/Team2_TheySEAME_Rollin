#include <boost/python.hpp>
#include "PurePursuit.hpp"

BOOST_PYTHON_MODULE(purePursuit) {
	boost::python::class_<PurePursuit>("PurePursuit", boost::python::init<>())
		.def("control", &PurePursuit::control)
		.def("getAngle", &PurePursuit::getAngle);

	boost::python::class_<Debug>("Debug", boost::python::init<>())
		.def_readwrite("angle", &Debug::angle)
		.def_readwrite("cte", &Debug::cte);
}
