#include <boost/python.hpp>
#include "LkaControl.hpp"

BOOST_PYTHON_MODULE(lkaControl) {
	boost::python::class_<LkaControl>("LkaControl", boost::python::init<>())
		.def("control", &LkaControl::control)
		.def("getAngle", &LkaControl::getAngle);

	boost::python::class_<Debug>("Debug", boost::python::init<>())
		.def_readwrite("angle", &Debug::angle)
		.def_readwrite("cte", &Debug::cte)
		.def_readwrite("diff", &Debug::heading);
}
